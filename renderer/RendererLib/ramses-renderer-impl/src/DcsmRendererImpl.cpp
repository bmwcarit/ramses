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
#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "DcsmMetadataUpdateImpl.h"
#include "IDcsmConsumerImpl.h"
#include "DcsmRendererConfigImpl.h"
#include "Utils/LogMacros.h"
#include "RamsesFrameworkTypesImpl.h"
#include <array>

namespace ramses
{
    DcsmRendererImpl::DcsmRendererImpl(const DcsmRendererConfig& config, IDcsmConsumerImpl& dcsmConsumer, std::unique_ptr<ramses_internal::IDisplayManager> displayManager)
        : m_displayManager(std::move(displayManager))
        , m_dcsmConsumer(dcsmConsumer)
    {
        static_assert(ramses_internal::SceneState::Rendered > ramses_internal::SceneState::Ready, "adjust logic");
        static_assert(ramses_internal::SceneState::Ready > ramses_internal::SceneState::Available, "adjust logic");
        static_assert(ramses_internal::SceneState::Available > ramses_internal::SceneState::Unavailable, "adjust logic");

        for (const auto& c : config.m_impl.getCategories())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: registered category " << c.first << " (" << c.second.size.width << "x" << c.second.size.height << ") on display " << c.second.display);
            m_categories.insert({ c.first, { c.second.size, c.second.display, {} } });
        }
    }

    status_t DcsmRendererImpl::requestContentReady(ContentID contentID, uint64_t timeOut)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:requestContentReady: content " << contentID << " timeOut " << timeOut << " (timeNow=" << m_timeStampNow << ")");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set state of unknown content");

        auto& contentInfo = contentIt->second;
        const ContentState currState = getCurrentState(contentID);
        switch (currState)
        {
        case ContentState::Available:
            contentInfo.readyRequested = true;
            contentInfo.readyRequestTimeOut = (timeOut > 0 ? m_timeStampNow + timeOut : std::numeric_limits<uint64_t>::max());
            if (!contentInfo.dcsmReady)
                CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, { 0, 0 }));
            break;
        case ContentState::Ready:
            contentInfo.readyRequestTimeOut = (timeOut > 0 ? m_timeStampNow + timeOut : std::numeric_limits<uint64_t>::max());
            break;
        case ContentState::Shown:
            return addErrorEntry("DcsmRenderer:requestContentReady: content is shown already, cannot request ready");
        case ContentState::Invalid:
            assert(false);
        }

        return StatusOK;
    }

    status_t DcsmRendererImpl::showContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:showContent: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
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
            scheduleSceneStateChange(contentID, ramses_internal::SceneState::Rendered, timingInfo.startTime);
            break;
        case ContentState::Invalid:
            assert(false);
        }
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmRendererImpl::hideContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:hideContent: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
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
            scheduleSceneStateChange(contentID, ramses_internal::SceneState::Ready, timingInfo.finishTime);
            break;
        case ContentState::Invalid:
            assert(false);
        }
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmRendererImpl::releaseContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:releaseContent: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set state of unknown content");

        const ContentState lastState = getCurrentState(contentID);
        CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Assigned, timingInfo));
        contentIt->second.dcsmReady = false;
        contentIt->second.readyRequested = false;

        scheduleSceneStateChange(contentID, ramses_internal::SceneState::Available, timingInfo.finishTime);
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmRendererImpl::setCategorySize(Category categoryId, SizeInfo size, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:setCategorySize: category " << categoryId << " size " << size.width << "x" << size.height << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        auto it = m_categories.find(categoryId);
        if (it == m_categories.end())
            return addErrorEntry("DcsmRenderer: cannot set size of unknown category, make sure category is added to DcsmRendererConfig at creation time");

        status_t combinedStat = StatusOK;
        for (const auto content : it->second.assignedContentIds)
        {
            const auto stat = m_dcsmConsumer.contentSizeChange(content, size, timingInfo);
            if (stat != StatusOK)
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to set content size on Dcsm consumer for content " << content);
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
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:acceptStopOffer: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot set accept stop offer of unknown content");

        const auto status = m_dcsmConsumer.acceptStopOffer(contentID, timingInfo);
        if (status != StatusOK)
            return status;

        contentIt->second.dcsmReady = false;
        contentIt->second.readyRequested = false;
        scheduleSceneStateChange(contentID, ramses_internal::SceneState::Unavailable, timingInfo.finishTime);

        Command cmd{ CommandType::RemoveContent, timingInfo.finishTime };
        cmd.contentId = contentID;
        m_pendingCommands.push_back(cmd);

        return StatusOK;
    }

    status_t DcsmRendererImpl::assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:assignContentToDisplayBuffer: content " << contentID << " displayBuffer " << displayBuffer
            << " renderOrder " << renderOrder);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmRenderer: cannot assign unknown content");

        const auto sceneId = findContentScene(contentID);
        if (sceneId == nullptr)
            return addErrorEntry("DcsmRenderer: content must be ready (at least reported as ready from DCSM provider) in order to be able to assign it to a display buffer");

        if (!m_displayManager->setSceneDisplayBufferAssignment(*sceneId, displayBuffer, renderOrder))
            return addErrorEntry("DcsmRenderer: failed to set scene display buffer assignment");

        return StatusOK;
    }

    status_t DcsmRendererImpl::setDisplayBufferClearColor(displayBufferId_t displayBuffer, float r, float g, float b, float a)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:setDisplayBufferClearColor: displayBuffer " << displayBuffer << " color "
            << r << ", " << g << ", " << b << ", " << a);

        if (!m_displayManager->setDisplayBufferClearColor(displayBuffer, r, g, b, a))
            return addErrorEntry("DcsmRenderer: failed to set display buffer color");

        return StatusOK;
    }

    status_t DcsmRendererImpl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:linkOffscreenBuffer: offscreenBufferId " << offscreenBufferId
            << " consumerContent " << consumerContentID << " consumerId " << consumerId);

        if (m_contents.count(consumerContentID) == 0)
            return addErrorEntry("DcsmRenderer: failed to link offscreen buffer, consumer content unknown.");

        const auto consumerSceneId = findContentScene(consumerContentID);
        if (consumerSceneId == nullptr)
            return addErrorEntry("DcsmRenderer: failed to link offscreen buffer, consumer content's scene unknown. Make content ready at least once before linking.");

        m_displayManager->linkOffscreenBuffer(offscreenBufferId, *consumerSceneId, consumerId);

        return StatusOK;
    }

    status_t DcsmRendererImpl::linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer:linkData: providerContent " << providerContentID << " providerId " << providerId
            << " consumerContent " << consumerContentID << " consumerId " << consumerId);

        if (m_contents.count(providerContentID) == 0)
            return addErrorEntry("DcsmRenderer: failed to link data, provider content unknown.");
        if (m_contents.count(consumerContentID) == 0)
            return addErrorEntry("DcsmRenderer: failed to link data, consumer content unknown.");

        const auto providerSceneId = findContentScene(providerContentID);
        const auto consumerSceneId = findContentScene(consumerContentID);
        if (providerSceneId == nullptr)
            return addErrorEntry("DcsmRenderer: failed to link data, provider content's scene unknown. Make content ready at least once before linking.");
        if (consumerSceneId == nullptr)
            return addErrorEntry("DcsmRenderer: failed to link data, consumer content's scene unknown. Make content ready at least once before linking.");

        m_displayManager->linkData(*providerSceneId, providerId, *consumerSceneId, consumerId);

        return StatusOK;
    }

    status_t DcsmRendererImpl::update(uint64_t timeStampNow, IDcsmRendererEventHandler& eventHandler, IRendererEventHandler* customRendererEventHandler)
    {
        if (timeStampNow < m_timeStampNow)
            return addErrorEntry("DcsmRenderer::update called with timeStampNow older than previous timestamp");
        m_timeStampNow = timeStampNow;

        const auto stat = m_dcsmConsumer.dispatchEvents(*this);
        if (stat != StatusOK)
            return stat;
        m_displayManager->dispatchAndFlush(this, customRendererEventHandler);
        executePendingCommands();
        processTimedOutRequests();
        dispatchPendingEvents(eventHandler);

        return StatusOK;
    }

    void DcsmRendererImpl::contentOffered(ramses::ContentID contentID, ramses::Category category)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT OFFERED for content " << contentID << " category " << category);
        auto it = m_categories.find(category);
        if (it != m_categories.end())
        {
            const auto contentIt = m_contents.find(contentID);
            if (contentIt != m_contents.cend() && contentIt->second.category != category)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content offered " << contentID << " is already assigned to another category " << contentIt->second.category
                    << ", now requesting category " << category << ". Ignoring, stop offer first before changing category.");
                return;
            }

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: assigning content " << contentID << " to category " << category);
            auto& catInfo = it->second;
            catInfo.assignedContentIds.insert(contentID);
            m_contents.emplace(std::make_pair(contentID, ContentInfo{ category, false, 0, 0 }));
            m_dcsmConsumer.assignContentToConsumer(contentID, catInfo.size);

            handleContentStateChange(contentID, ContentState::Invalid); // using invalid last state to force event emit
        }
        else
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: not interested in content offer " << contentID << " for category " << category);
    }

    void DcsmRendererImpl::contentReady(ramses::ContentID contentID, ramses::ETechnicalContentType contentType, ramses::TechnicalContentDescriptor contentDescriptor)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT READY for content " << contentID << " content type " << int(contentType) << " content descriptor " << contentDescriptor);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID << " not assigned, ignoring state change.");
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
                    requestSceneState(contentID, ramses_internal::SceneState::Ready);
                    handleContentStateChange(contentID, lastState);
                    break;
                }
                }
            }
            else
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID << " not requested to be ready or released after request, ignoring state change.");
        }
    }

    void DcsmRendererImpl::contentFocusRequest(ramses::ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT FOCUS REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID << " not assigned, ignoring request.");
        }
        else
            m_pendingEvents.push_back({ EventType::ContentFocusRequested, contentID, Category{0}, {}, {}, DcsmRendererEventResult::OK });
    }

    void DcsmRendererImpl::contentStopOfferRequest(ramses::ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event CONTENT STOP OFFER REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID << " not assigned, confirming stop offer right away.");
            m_dcsmConsumer.acceptStopOffer(contentID, { 0, 0 });
        }
        else
            m_pendingEvents.push_back({ EventType::ContentStopOfferRequested, contentID, Category{0}, {}, {}, DcsmRendererEventResult::OK });
    }

    void DcsmRendererImpl::forceContentOfferStopped(ramses::ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event FORCE CONTENT OFFER STOPPED for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID << " not assigned, nothing to do.");
        }
        else
        {
            removeContent(contentID);
            for (const auto& sceneIt : m_scenes)
                if (sceneIt.second.associatedContents.count(contentID) > 0)
                    if (!m_displayManager->setSceneState(sceneIt.first, ramses_internal::SceneState::Unavailable))
                        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to set scene state.");
            m_pendingEvents.push_back({ EventType::ContentNotAvailable, contentID, Category{0}, {}, {}, DcsmRendererEventResult::OK });
        }
    }

    void DcsmRendererImpl::contentMetadataUpdated(ramses::ContentID contentID, const DcsmMetadataUpdate& metadataUpdate)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received DCSM event UPDATE CONTENT METADATA for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID << " not assigned, nothing to do.");
        }
        else
            m_pendingEvents.push_back({ EventType::ContentMetadataUpdate, contentID, Category{0}, {}, {}, DcsmRendererEventResult::OK, metadataUpdate.impl.getMetadata() });
    }

    void DcsmRendererImpl::scenePublished(ramses::sceneId_t sceneId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: scene " << sceneId << " published");
    }

    void DcsmRendererImpl::sceneStateChanged(ramses::sceneId_t sceneId, ramses_internal::SceneState state, ramses::displayId_t displaySceneIsMappedTo)
    {
        auto& sceneInfo = m_scenes[sceneId];
        if (sceneInfo.sharedState.getActualState() < ramses_internal::SceneState::Ready && state == ramses_internal::SceneState::Ready)
        {
            if (std::none_of(sceneInfo.associatedContents.cbegin(), sceneInfo.associatedContents.cend(), [&](ContentID contentID) { return m_contents.find(contentID)->second.readyRequested; }))
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: scene " << sceneId << " changed state from "
                    << ramses_internal::SceneStateName(sceneInfo.sharedState.getActualState())
                    << " to " << ramses_internal::SceneStateName(state)
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
            if (state == ramses_internal::SceneState::Ready)
            {
                assert(m_contents.count(contentID) > 0);
                assert(m_categories.count(m_contents.find(contentID)->second.category) > 0);
                const auto category = m_contents.find(contentID)->second.category;
                const auto& catInfo = m_categories.find(category)->second;
                if (displaySceneIsMappedTo != displayId_t::Invalid() && (catInfo.display != displaySceneIsMappedTo))
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: unexpected: scene " << sceneId << " belonging to content " << contentID << " category " << category
                        << " is mapped to display " << displaySceneIsMappedTo << " but category's display was set to " << catInfo.display);
            }

            // emit event if state changed for any of the associated contents
            handleContentStateChange(contentID, lastStates[contentID]);
        }
    }

    void DcsmRendererImpl::offscreenBufferLinked(ramses::displayBufferId_t offscreenBufferId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, bool success)
    {
        const auto consumerSceneIt = m_scenes.find(consumerScene);
        const bool consumerContentKnown = (consumerSceneIt != m_scenes.cend() && !consumerSceneIt->second.associatedContents.empty());

        if (!consumerContentKnown)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received offscreen buffer linked event but cannot find corresponding consumer content,"
                " this can happen if content or its scene became unavailable after offscreen buffer linked processed. Event will still be emitted but with content ID set to 0.");
        }
        // there can be multiple contents using same scene, for simplification of reporting we pick only one content which uses the scene linked
        const ContentID consumerContent = (consumerContentKnown ? *consumerSceneIt->second.associatedContents.cbegin() : ContentID{ 0 });

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: offscreen buffer linked - offscreen buffer " << offscreenBufferId << " to consumer content " << consumerContent << " dataId " << consumerId);
        Event evt{ EventType::OffscreenBufferLinked };
        evt.displayBuffer = offscreenBufferId;
        evt.consumerContentID = consumerContent;
        evt.consumerID = consumerId;
        evt.result = (success ? DcsmRendererEventResult::OK : DcsmRendererEventResult::TimedOut);
        m_pendingEvents.push_back(evt);
    }

    void DcsmRendererImpl::dataLinked(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, bool success)
    {
        const auto providerSceneIt = m_scenes.find(providerScene);
        const auto consumerSceneIt = m_scenes.find(consumerScene);
        const bool providerContentKnown = (providerSceneIt != m_scenes.cend() && !providerSceneIt->second.associatedContents.empty());
        const bool consumerContentKnown = (consumerSceneIt != m_scenes.cend() && !consumerSceneIt->second.associatedContents.empty());

        if (!providerContentKnown || !consumerContentKnown)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: received data linked event but cannot find corresponding provider or consumer content,"
                " this can happen if content or its scene became unavailable after data linked processed. Event will still be emitted but with content ID set to 0.");
        }
        // there can be multiple contents using same scene, for simplification of reporting we pick only one content which uses the scene linked
        const ContentID providerContent = (providerContentKnown ? *providerSceneIt->second.associatedContents.cbegin() : ContentID{0});
        const ContentID consumerContent = (consumerContentKnown ? *consumerSceneIt->second.associatedContents.cbegin() : ContentID{0});

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: data linked - provider content " << providerContent << " dataId " << providerId
            << " to consumer content " << consumerContent << " dataId " << consumerId);
        Event evt{ EventType::DataLinked };
        evt.providerContentID = providerContent;
        evt.consumerContentID = consumerContent;
        evt.providerID = providerId;
        evt.consumerID = consumerId;
        evt.result = (success ? DcsmRendererEventResult::OK : DcsmRendererEventResult::TimedOut );
        m_pendingEvents.push_back(evt);
    }

    void DcsmRendererImpl::executePendingCommands()
    {
        std::vector<ContentID> contentsToRemove;
        auto itEnd = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [&](const Command& cmd)
        {
            if (cmd.timePoint > m_timeStampNow)
                return false;

            switch (cmd.type)
            {
            case CommandType::SceneStateChange:
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: executing scheduled command for content " << cmd.contentId << " to request scene state change to " << ramses_internal::SceneStateName(cmd.sceneState));
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
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: executing scheduled command to remove content " << contentId);
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
                        eventHandler.contentReady(evt.contentID, evt.result);
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
            case EventType::ContentMetadataUpdate:
            {
                DcsmMetadataUpdate metadataUpdate(*new DcsmMetadataUpdateImpl);
                metadataUpdate.impl.setMetadata(std::move(evt.metadata));
                eventHandler.contentMetadataUpdated(evt.contentID, metadataUpdate);
                break;
            }
            case EventType::OffscreenBufferLinked:
                eventHandler.offscreenBufferLinked(evt.displayBuffer, evt.consumerContentID, evt.consumerID, evt.result == DcsmRendererEventResult::OK);
                break;
            case EventType::DataLinked:
                eventHandler.dataLinked(evt.providerContentID, evt.providerID, evt.consumerContentID, evt.consumerID, evt.result == DcsmRendererEventResult::OK);
                break;
            }
        }
    }

    void DcsmRendererImpl::requestSceneState(ContentID contentID, ramses_internal::SceneState state)
    {
        const auto sceneIdp = findContentScene(contentID);
        if (sceneIdp != nullptr)
        {
            const sceneId_t sceneId = *sceneIdp;
            auto& sharedState = m_scenes[sceneId].sharedState;
            sharedState.setDesiredState(contentID, state);

            const auto actualState = sharedState.getActualState();
            const auto consolidatedDesiredState = sharedState.getConsolidatedDesiredState();
            if (consolidatedDesiredState != actualState)
            {
                if (consolidatedDesiredState >= ramses_internal::SceneState::Ready && actualState < ramses_internal::SceneState::Ready)
                {
                    assert(m_contents.count(contentID) > 0);
                    const ContentInfo& contentInfo = m_contents.find(contentID)->second;
                    const auto categoryId = contentInfo.category;
                    assert(m_categories.count(categoryId) > 0);
                    const auto displayId = m_categories.find(categoryId)->second.display;
                    if (!m_displayManager->setSceneMapping(sceneId, displayId))
                        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to set scene mapping.");
                }

                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: requesting scene state change for content " << contentID << " scene " << sceneId << " state change to " << ramses_internal::SceneStateName(consolidatedDesiredState));
                if (!m_displayManager->setSceneState(sceneId, consolidatedDesiredState))
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: failed to request state for scene " << sceneId << " to " << ramses_internal::SceneStateName(consolidatedDesiredState));
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

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: content " << contentID << " state changed from " << ContentStateName(lastState) << " to " << ContentStateName(currState));
            m_pendingEvents.push_back({ EventType::ContentStateChanged, contentID, contentInfo.category, currState, lastState, DcsmRendererEventResult::OK });
        }
    }

    void DcsmRendererImpl::removeContent(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: removing content " << contentID << " and any pending commands associated with it");
        m_contents.erase(contentID);
        for (auto& catIt : m_categories)
            catIt.second.assignedContentIds.erase(contentID);
        for (auto& sceneIt : m_scenes)
            sceneIt.second.associatedContents.erase(contentID);
        const auto it = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [contentID](const Command& cmd)
        {
            if (cmd.contentId == contentID)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: there is a scheduled change for content " << contentID << " being removed, removing the scheduled change "
                    << int(cmd.type) << " scene state change " << ramses_internal::SceneStateName(cmd.sceneState) << " at TS " << cmd.timePoint);
                return true;
            }
            return false;
        });
        m_pendingCommands.erase(it, m_pendingCommands.end());
    }

    void DcsmRendererImpl::scheduleSceneStateChange(ContentID contentThatTrigerredChange, ramses_internal::SceneState sceneState, uint64_t ts)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: scheduling content's scene state change for content " << contentThatTrigerredChange
            << " to scene state " << ramses_internal::SceneStateName(sceneState) << " at TS " << ts);

        // Remove any other state change scheduled for this content
        auto itEnd = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [contentThatTrigerredChange](const Command& cmd)
        {
            if (cmd.type == CommandType::SceneStateChange && cmd.contentId == contentThatTrigerredChange)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: there is already a scheduled change for this content's scene, overriding previous change to scene state "
                    << ramses_internal::SceneStateName(cmd.sceneState) << " at TS " << cmd.timePoint);
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
        const auto sceneId = findContentScene(contentID);

        // if scene is available for content, check the shared scene state to determine content compound state
        if (sceneId != nullptr)
        {
            const auto sceneState = m_scenes.find(*sceneId)->second.sharedState.getCurrentStateForContent(contentID);
            switch (sceneState)
            {
            case ramses_internal::SceneState::Ready:
                return ContentState::Ready;
            case ramses_internal::SceneState::Rendered:
                return ContentState::Shown;
            case ramses_internal::SceneState::Available:
            case ramses_internal::SceneState::Unavailable: // even if scene is unavailable the content is known and available to request ready
                return ContentState::Available;
            }
        }

        return ContentState::Available;
    }

    void DcsmRendererImpl::processTimedOutRequests()
    {
        for (const auto& contentIt : m_contents)
        {
            const auto contentID = contentIt.first;
            const auto& contentInfo = contentIt.second;
            if (contentInfo.readyRequested && m_timeStampNow > contentInfo.readyRequestTimeOut)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmRenderer: ready request for content " << contentID << " timed out (timeoutTS=" << contentInfo.readyRequestTimeOut
                    << " nowTS=" << m_timeStampNow << "), releasing content. Request ready again for re-try.");
                m_pendingEvents.push_back({ EventType::ContentStateChanged, contentID, Category{0}, ContentState::Ready, ContentState::Available, DcsmRendererEventResult::TimedOut });
                releaseContent(contentID, { 0, 0 });
            }
        }
    }

    const sceneId_t* DcsmRendererImpl::findContentScene(ContentID contentID) const
    {
        const auto sceneIt = std::find_if(m_scenes.cbegin(), m_scenes.cend(), [contentID](const auto& s)
        {
            return s.second.associatedContents.count(contentID) != 0;
        });

        return (sceneIt == m_scenes.cend() ? nullptr : &sceneIt->first);
    }
}
