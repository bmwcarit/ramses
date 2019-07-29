//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmRendererImpl.h"
#include "ramses-renderer-api/IDcsmRendererEventHandler.h"
#include "ramses-renderer-api/DcsmRendererConfig.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "IDcsmConsumerImpl.h"
#include "DcsmRendererConfigImpl.h"
#include "Utils/LogMacros.h"
#include <array>

namespace ramses
{
    DcsmRendererImpl::DcsmRendererImpl(const DcsmRendererConfig& config, IDcsmConsumerImpl& dcsmConsumer, std::unique_ptr<ramses_display_manager::IDisplayManager> displayManager)
        : m_displayManager(std::move(displayManager))
        , m_dcsmConsumer(dcsmConsumer)
    {
        static_assert(ramses_display_manager::SceneState::Rendered > ramses_display_manager::SceneState::Ready, "adjust logic");
        static_assert(ramses_display_manager::SceneState::Ready > ramses_display_manager::SceneState::Available, "adjust logic");
        static_assert(ramses_display_manager::SceneState::Available > ramses_display_manager::SceneState::Unavailable, "adjust logic");

        for (const auto& c : config.m_impl.getCategories())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: registered category " << c.first.getValue() << " (" << c.second.size.width << "x" << c.second.size.height << ") on display " << c.second.display);
            m_categories[c.first] = { c.second.size, c.second.display, {} };
        }
    }

    status_t DcsmRendererImpl::requestContentReady(ContentID contentID, uint64_t timeOut)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:requestContentReady: content " << contentID.getValue() << " timeOut " << timeOut);
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set state of unknown content");

        auto& contentInfo = contentIt->second;
        const ContentState currState = getCurrentState(contentID);
        switch (currState)
        {
        case ContentState::Available:
            contentInfo.readyRequested = true;
            contentInfo.obWidth = contentInfo.obHeight = 0; // reset OB if previously used OB link
            if (!contentInfo.dcsmReady)
                CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, { 0, 0 }));
            break;
        case ContentState::Ready:
            if (contentInfo.obWidth != 0 && contentInfo.obHeight != 0)
                return addErrorEntry("DcsmRenderer: content is already ready using different parameters, release content first in order to change mapping/linking.");
            // TODO update timeOut
            break;
        case ContentState::Shown:
            return addErrorEntry("DcsmRenderer:requestContentReady: content is shown already, cannot request ready");
        case ContentState::Invalid:
            assert(false);
        }

        return StatusOK;
    }

    ramses::status_t DcsmRendererImpl::requestContentReadyAndLinkedViaOffscreenBuffer(ContentID contentID, uint32_t width, uint32_t height, ContentID consumerContentID, dataConsumerId_t consumerDataId, uint64_t timeOut)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:requestContentReadyAndLinkedViaOffscreenBuffer: content " << contentID.getValue()
            << " OB[" << width << "x" << height << "] consumerContent " << consumerContentID.getValue() << " consumerData " << consumerDataId << " timeOut " << timeOut);
        if (consumerContentID == contentID)
            return addErrorEntry("DcsmRenderer: cannot link content to itself via offscreen buffer");
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot make unknown content ready");
        const auto consumerContentIt = m_contents.find(consumerContentID);
        if (consumerContentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: consumer content ID is unknown.");
        if (width == 0 || height == 0)
            return addErrorEntry("DcsmRenderer: invalid offscreen buffer width and/or height.");

        const auto consumerContentState = getCurrentState(consumerContentID);
        if (consumerContentState != ContentState::Ready && consumerContentState != ContentState::Shown)
            return addErrorEntry("DcsmRenderer: consumer content has to be in ready state in order to link other content to it.");

        auto& contentInfo = contentIt->second;
        const auto categoryId = contentInfo.category;
        assert(m_categories.count(categoryId) > 0);
        const auto displayId = m_categories.find(categoryId)->second.display;
        const auto consumerCategoryId = consumerContentIt->second.category;
        assert(m_categories.count(consumerCategoryId) > 0);
        const auto consumerDisplayId = m_categories.find(consumerCategoryId)->second.display;
        if (displayId != consumerDisplayId)
            return addErrorEntry("DcsmRenderer: cannot link content via OB to consumer content that is mapped on other display.");

        const auto consumerSceneIt = std::find_if(m_scenes.cbegin(), m_scenes.cend(), [consumerContentID](const auto& s)
        {
            return s.second.associatedContents.count(consumerContentID) != 0;
        });
        assert(consumerSceneIt != m_scenes.cend());
        const auto consumerSceneId = consumerSceneIt->first;

        const ContentState currState = getCurrentState(contentID);
        switch (currState)
        {
        case ContentState::Available:
            contentInfo.readyRequested = true;
            contentInfo.obWidth = width;
            contentInfo.obHeight = height;
            contentInfo.consumerSceneID = consumerSceneId;
            contentInfo.consumerDataID = consumerDataId;
            if (!contentInfo.dcsmReady)
                CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, { 0, 0 }));
            break;
        case ContentState::Ready:
            if (contentInfo.obWidth != width || contentInfo.obHeight != height || contentInfo.consumerSceneID != consumerSceneId || contentInfo.consumerDataID != consumerDataId)
                return addErrorEntry("DcsmRenderer: content is already ready using different linking parameters, release content first in order to change mapping/linking.");
            // TODO update timeOut
            break;
        case ContentState::Shown:
            return addErrorEntry("DcsmRenderer:requestContentReady: content is shown already, cannot request ready.");
        case ContentState::Invalid:
            assert(false);
        }

        return StatusOK;
    }

    status_t DcsmRendererImpl::showContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:showContent: content " << contentID.getValue() << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set state of unknown content");

        if (!contentIt->second.dcsmReady)
            return addErrorEntry("DcsmRenderer:showContent: cannot show content if it is not ready from Dcsm provider side");

        const ContentState currState = getCurrentState(contentID);
        const ContentState lastState = currState;
        switch (currState)
        {
        case ContentState::Available:
            return addErrorEntry("DcsmRenderer:showContent: cannot show content if it is not ready yet");
        case ContentState::Ready:
        case ContentState::Shown: // execute also if already shown in case timing info is updated
            CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Shown, timingInfo));
            scheduleSceneStateChange(contentID, ramses_display_manager::SceneState::Rendered, timingInfo.startTime);
            break;
        case ContentState::Invalid:
            assert(false);
        }
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmRendererImpl::hideContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:hideContent: content " << contentID.getValue() << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set state of unknown content");

        const ContentState currState = getCurrentState(contentID);
        const ContentState lastState = currState;
        switch (currState)
        {
        case ContentState::Available:
            return addErrorEntry("DcsmRenderer:hideContent: content is not shown, cannot hide");
        case ContentState::Ready: // execute also if already hidden (ready) in case timing info is updated
        case ContentState::Shown:
            CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, timingInfo));
            scheduleSceneStateChange(contentID, ramses_display_manager::SceneState::Ready, timingInfo.finishTime);
            break;
        case ContentState::Invalid:
            assert(false);
        }
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmRendererImpl::releaseContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:releaseContent: content " << contentID.getValue() << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set state of unknown content");

        const ContentState lastState = getCurrentState(contentID);
        CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Assigned, timingInfo));
        contentIt->second.dcsmReady = false;
        contentIt->second.readyRequested = false;

        scheduleSceneStateChange(contentID, ramses_display_manager::SceneState::Available, timingInfo.finishTime);
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmRendererImpl::setCategorySize(Category categoryId, SizeInfo size, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:setCategorySize: category " << categoryId.getValue() << " size " << size.width << "x" << size.height << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        auto it = m_categories.find(categoryId);
        if (it == m_categories.end())
            return addErrorEntry("DcsmRenderer: cannot set size of unknown category, make sure category is added to DcsmRendererConfig at creation time");

        status_t combinedStat = StatusOK;
        for (const auto content : it->second.assignedContentIds)
        {
            const auto stat = m_dcsmConsumer.contentSizeChange(content, size, timingInfo);
            if (stat != StatusOK)
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to set content size on Dcsm consumer for content " << content.getValue());
                combinedStat = stat;
            }
        }
        CHECK_RETURN_ERR(combinedStat);

        // size updated immediately in order to report the latest size to newly offered contents to this category
        it->second.size = size;

        return StatusOK;
    }

    status_t DcsmRendererImpl::acceptStopOffer(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:acceptStopOffer: content " << contentID.getValue() << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set accept stop offer of unknown content");

        const auto status = m_dcsmConsumer.acceptStopOffer(contentID, timingInfo);
        if (status != StatusOK)
            return status;

        contentIt->second.dcsmReady = false;
        contentIt->second.readyRequested = false;
        scheduleSceneStateChange(contentID, ramses_display_manager::SceneState::Unavailable, timingInfo.finishTime);

        Command cmd{ CommandType::RemoveContent, timingInfo.finishTime };
        cmd.contentId = contentID;
        m_pendingCommands.push_back(cmd);

        return StatusOK;
    }

    status_t DcsmRendererImpl::update(uint64_t timeStampNow, IDcsmRendererEventHandler& eventHandler, IRendererEventHandler* customRendererEventHandler)
    {
        const auto stat = m_dcsmConsumer.dispatchEvents(*this);
        if (stat != StatusOK)
            return stat;
        m_displayManager->dispatchAndFlush(this, customRendererEventHandler);
        executePendingCommands(timeStampNow);
        dispatchPendingEvents(eventHandler);

        return StatusOK;
    }

    void DcsmRendererImpl::contentOffered(ramses::ContentID contentID, ramses::Category category)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT OFFERED for content " << contentID.getValue() << " category " << category.getValue());
        auto it = m_categories.find(category);
        if (it != m_categories.end())
        {
            const auto contentIt = m_contents.find(contentID);
            if (contentIt != m_contents.cend() && contentIt->second.category != category)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content offered " << contentID.getValue() << " is already assigned to another category " << contentIt->second.category.getValue()
                    << ", now requesting category " << category.getValue() << ". Ignoring, stop offer first before changing category.");
                return;
            }

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: assigning content " << contentID.getValue() << " to category " << category.getValue());
            auto& catInfo = it->second;
            catInfo.assignedContentIds.insert(contentID);
            m_contents.emplace(std::make_pair(contentID, ContentInfo{ category, false, 0, 0, 0, 0 }));
            m_dcsmConsumer.assignContentToConsumer(contentID, catInfo.size);

            handleContentStateChange(contentID, ContentState::Invalid); // using invalid last state to force event emit
        }
        else
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: not interested in content offer " << contentID.getValue() << " for category " << category.getValue());
    }

    void DcsmRendererImpl::contentReady(ramses::ContentID contentID, ramses::ETechnicalContentType contentType, ramses::TechnicalContentDescriptor contentDescriptor)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT READY for content " << contentID.getValue() << " content type " << int(contentType) << " content descriptor " << contentDescriptor.getValue());
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID.getValue() << " not assigned, ignoring state change.");
        }
        else
        {
            assert(m_categories.count(contentIt->second.category) > 0);
            if (contentIt->second.readyRequested)
            {
                const ContentState lastState = getCurrentState(contentID);
                contentIt->second.dcsmReady = true;
                switch (contentType)
                {
                case ETechnicalContentType::RamsesSceneID:
                {
                    const sceneId_t sceneId{ contentDescriptor.getValue() };
                    m_scenes[sceneId].associatedContents.insert(contentID);
                    requestSceneState(contentID, ramses_display_manager::SceneState::Ready);
                    handleContentStateChange(contentID, lastState);
                    break;
                }
                }
            }
            else
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID.getValue() << " not requested to be ready or released after request, ignoring state change.");
        }
    }

    void DcsmRendererImpl::contentFocusRequest(ramses::ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT FOCUS REQUEST for content " << contentID.getValue());
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID.getValue() << " not assigned, ignoring request.");
        }
        else
            m_pendingEvents.push_back({ EventType::ContentFocusRequested, contentID, Category{0}, {}, {} });
    }

    void DcsmRendererImpl::contentStopOfferRequest(ramses::ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT STOP OFFER REQUEST for content " << contentID.getValue());
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID.getValue() << " not assigned, confirming stop offer right away.");
            m_dcsmConsumer.acceptStopOffer(contentID, { 0, 0 });
        }
        else
            m_pendingEvents.push_back({ EventType::ContentStopOfferRequested, contentID, Category{0}, {}, {} });
    }

    void DcsmRendererImpl::forceContentOfferStopped(ramses::ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event FORCE CONTENT OFFER STOPPED for content " << contentID.getValue());
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID.getValue() << " not assigned, nothing to do.");
        }
        else
        {
            removeContent(contentID);
            for (const auto& sceneIt : m_scenes)
                if (sceneIt.second.associatedContents.count(contentID) > 0)
                    if (!m_displayManager->setSceneState(sceneIt.first, ramses_display_manager::SceneState::Unavailable))
                        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to set scene state.");
            m_pendingEvents.push_back({ EventType::ContentNotAvailable, contentID, Category{0}, {}, {} });
        }
    }

    void DcsmRendererImpl::sceneStateChanged(ramses::sceneId_t sceneId, ramses_display_manager::SceneState state, ramses::displayId_t displaySceneIsMappedTo)
    {
        auto& sceneInfo = m_scenes[sceneId];
        if (sceneInfo.sharedState.getActualState() < ramses_display_manager::SceneState::Ready && state == ramses_display_manager::SceneState::Ready)
        {
            if (std::none_of(sceneInfo.associatedContents.cbegin(), sceneInfo.associatedContents.cend(), [&](ContentID contentID) { return m_contents.find(contentID)->second.readyRequested; }))
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: scene " << sceneId << " changed state from "
                    << ramses_display_manager::SceneStateName(sceneInfo.sharedState.getActualState())
                    << " to " << ramses_display_manager::SceneStateName(state)
                    << " but no associated content requested this change - ignoring. This is OK if ready request canceled before finishing");
                return;
            }
        }

        // first collect current state of all associated contents to be used as 'last' state when checking if changed
        std::unordered_map<ContentID, ContentState> lastStates;
        for (const auto contentID : sceneInfo.associatedContents)
            lastStates[contentID] = getCurrentState(contentID);

        // set new actual scene state which might modify consolidated state of some of the associated contents
        sceneInfo.sharedState.setActualState(state);

        for (const auto contentID : sceneInfo.associatedContents)
        {
            if (state == ramses_display_manager::SceneState::Ready)
            {
                assert(m_contents.count(contentID) > 0);
                assert(m_categories.count(m_contents.find(contentID)->second.category) > 0);
                const auto category = m_contents.find(contentID)->second.category;
                const auto& catInfo = m_categories[category];
                if (displaySceneIsMappedTo != InvalidDisplayId && (m_categories[category].display != displaySceneIsMappedTo))
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: unexpected: scene " << sceneId << " belonging to content " << contentID.getValue() << " category " << category.getValue()
                        << " is mapped to display " << displaySceneIsMappedTo << " but category's display was set to " << catInfo.display);
            }

            // emit event if state changed for any of the associated contents
            handleContentStateChange(contentID, lastStates[contentID]);
        }
    }

    void DcsmRendererImpl::executePendingCommands(uint64_t timeStampNow)
    {
        std::vector<ContentID> contentsToRemove;
        auto itEnd = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [&](const Command& cmd)
        {
            if (cmd.timePoint > timeStampNow)
                return false;

            switch (cmd.type)
            {
            case CommandType::SceneStateChange:
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: executing scheduled command for content " << cmd.contentId.getValue() << " to request scene state change to " << ramses_display_manager::SceneStateName(cmd.sceneState));
                const auto lastState = getCurrentState(cmd.contentId);
                requestSceneState(cmd.contentId, cmd.sceneState);
                handleContentStateChange(cmd.contentId, lastState);
                break;
            }
            case CommandType::RemoveContent:
                contentsToRemove.push_back(cmd.contentId);
                break;
            }

            return true;
        });
        m_pendingCommands.erase(itEnd, m_pendingCommands.end());

        for (auto contentId : contentsToRemove)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: executing scheduled command to remove content " << contentId.getValue());
            removeContent(contentId);
        }
    }

    void DcsmRendererImpl::dispatchPendingEvents(IDcsmRendererEventHandler& eventHandler)
    {
        // event handler is allowed to request changes which might result in new events,
        // therefore use a temp container for the execution to avoid container modifications while iterating it
        m_pendingEventsTmp.clear();
        m_pendingEventsTmp.swap(m_pendingEvents);
        for (const auto& evt : m_pendingEventsTmp)
        {
            switch (evt.type)
            {
            case EventType::ContentStateChanged:
                switch (evt.contentState)
                {
                case ContentState::Available:
                    if (evt.previousState == ContentState::Ready || evt.previousState == ContentState::Shown)
                        eventHandler.contentReleased(evt.contentID);
                    else
                        eventHandler.contentAvailable(evt.contentID, evt.category);
                    break;
                case ContentState::Ready:
                    if (evt.previousState == ContentState::Available)
                        eventHandler.contentReady(evt.contentID, DcsmRendererEventResult::OK);
                    else
                        eventHandler.contentHidden(evt.contentID);
                    break;
                case ContentState::Shown:
                    eventHandler.contentShown(evt.contentID);
                    break;
                case ContentState::Invalid:
                    assert(false);
                    break;
                }
                break;
            case EventType::ContentFocusRequested:
                eventHandler.contentFocusRequested(evt.contentID);
                break;
            case EventType::ContentStopOfferRequested:
                eventHandler.contentStopOfferRequested(evt.contentID);
                break;
            case EventType::ContentNotAvailable:
                eventHandler.contentNotAvailable(evt.contentID);
                break;
            }
        }
    }

    void DcsmRendererImpl::requestSceneState(ContentID contentID, ramses_display_manager::SceneState state)
    {
        auto sceneIt = std::find_if(m_scenes.begin(), m_scenes.end(), [contentID](const auto& s)
        {
            return s.second.associatedContents.count(contentID) != 0;
        });
        if (sceneIt != m_scenes.end())
        {
            auto& sharedState = sceneIt->second.sharedState;
            sharedState.setDesiredState(contentID, state);

            const auto actualState = sharedState.getActualState();
            const auto consolidatedDesiredState = sharedState.getConsolidatedDesiredState();
            if (consolidatedDesiredState != actualState)
            {
                const sceneId_t sceneId = sceneIt->first;
                if (consolidatedDesiredState >= ramses_display_manager::SceneState::Ready && actualState < ramses_display_manager::SceneState::Ready)
                {
                    assert(m_contents.count(contentID) > 0);
                    const ContentInfo& contentInfo = m_contents.find(contentID)->second;
                    const auto categoryId = contentInfo.category;
                    assert(m_categories.count(categoryId) > 0);
                    const auto displayId = m_categories.find(categoryId)->second.display;
                    if (contentInfo.obWidth > 0 && contentInfo.obHeight > 0)
                    {
                        if (!m_displayManager->setSceneOffscreenBufferMapping(sceneId, displayId, contentInfo.obWidth, contentInfo.obHeight, contentInfo.consumerSceneID, contentInfo.consumerDataID))
                            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to set scene offscreen buffer mapping.");
                    }
                    else
                    {
                        if (!m_displayManager->setSceneMapping(sceneId, displayId))
                            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to set scene mapping.");
                    }
                }

                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: requesting scene state change for content " << contentID.getValue() << " scene " << sceneId << " state change to " << ramses_display_manager::SceneStateName(consolidatedDesiredState));
                if (!m_displayManager->setSceneState(sceneId, consolidatedDesiredState))
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to request state for scene " << sceneId << " to " << ramses_display_manager::SceneStateName(consolidatedDesiredState));
            }
        }
    }

    void DcsmRendererImpl::handleContentStateChange(ContentID contentID, ContentState lastState)
    {
        const ContentState currState = getCurrentState(contentID);
        if (currState != lastState)
        {
            assert(m_contents.count(contentID) > 0);
            auto& contentInfo = m_contents.find(contentID)->second;
            if (currState > ContentState::Available)
                contentInfo.readyRequested = false;

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID.getValue() << " state changed from " << ContentStateName(lastState) << " to " << ContentStateName(currState));
            m_pendingEvents.push_back({ EventType::ContentStateChanged, contentID, contentInfo.category, currState, lastState });
        }
    }

    void DcsmRendererImpl::removeContent(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: removing content " << contentID.getValue() << " and any pending commands associated with it");
        m_contents.erase(contentID);
        for (auto& catIt : m_categories)
            catIt.second.assignedContentIds.erase(contentID);
        for (auto& sceneIt : m_scenes)
            sceneIt.second.associatedContents.erase(contentID);
        const auto it = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [contentID](const Command& cmd)
        {
            if (cmd.contentId == contentID)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: there is a scheduled change for content " << contentID.getValue() << " being removed, removing the scheduled change "
                    << int(cmd.type) << " scene state change " << ramses_display_manager::SceneStateName(cmd.sceneState) << " at TS " << cmd.timePoint);
                return true;
            }
            return false;
        });
        m_pendingCommands.erase(it, m_pendingCommands.end());
    }

    void DcsmRendererImpl::scheduleSceneStateChange(ContentID contentThatTrigerredChange, ramses_display_manager::SceneState sceneState, uint64_t ts)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: scheduling content's scene state change for content " << contentThatTrigerredChange.getValue()
            << " to scene state " << ramses_display_manager::SceneStateName(sceneState) << " at TS " << ts);

        // Remove any other state change scheduled for this content
        auto itEnd = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [contentThatTrigerredChange](const Command& cmd)
        {
            if (cmd.type == CommandType::SceneStateChange && cmd.contentId == contentThatTrigerredChange)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: there is already a scheduled change for this content's scene, overriding previous change to scene state "
                    << ramses_display_manager::SceneStateName(cmd.sceneState) << " at TS " << cmd.timePoint);
                return true;
            }

            return false;
        });
        m_pendingCommands.erase(itEnd, m_pendingCommands.end());

        Command cmd{ CommandType::SceneStateChange, ts };
        cmd.contentId = contentThatTrigerredChange;
        cmd.sceneState = sceneState;
        m_pendingCommands.push_back(cmd);
    }

    ContentState DcsmRendererImpl::getCurrentState(ContentID contentID) const
    {
        assert(m_contents.count(contentID) > 0);
        const auto sceneIt = std::find_if(m_scenes.cbegin(), m_scenes.cend(), [contentID](const auto& s)
        {
            return s.second.associatedContents.count(contentID) != 0;
        });

        // if scene is available for content, check the shared scene state to determine content compound state
        if (sceneIt != m_scenes.cend())
        {
            const auto sceneState = sceneIt->second.sharedState.getCurrentStateForContent(contentID);
            switch (sceneState)
            {
            case ramses_display_manager::SceneState::Ready:
                return ContentState::Ready;
            case ramses_display_manager::SceneState::Rendered:
                return ContentState::Shown;
            case ramses_display_manager::SceneState::Available:
            case ramses_display_manager::SceneState::Unavailable: // even if scene is unavailable the content is known and available to request ready
                return ContentState::Available;
            }
        }

        return ContentState::Available;
    }
}
