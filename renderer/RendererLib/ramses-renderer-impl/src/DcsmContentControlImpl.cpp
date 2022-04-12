//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmContentControlImpl.h"
#include "ramses-renderer-api/IDcsmContentControlEventHandler.h"
#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "ramses-framework-api/DcsmStatusMessage.h"
#include "DcsmMetadataUpdateImpl.h"
#include "IDcsmConsumerImpl.h"
#include "RendererSceneControlImpl.h"
#include "RamsesFrameworkTypesImpl.h"
#include "Utils/LogMacros.h"
#include "Utils/LoggingUtils.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "absl/algorithm/container.h"
#include <array>

namespace ramses
{
    DcsmContentControlImpl::DcsmContentControlImpl(IDcsmConsumerImpl& dcsmConsumer, IRendererSceneControl& sceneControl)
        : m_sceneControl(sceneControl)
        , m_dcsmConsumer(dcsmConsumer)
    {
    }

    status_t DcsmContentControlImpl::addContentCategory(Category category, displayId_t display, const CategoryInfoUpdate& categoryInformation)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::addContentCategory: category:" << category << " display: " << display
            << " renderSize: " << categoryInformation.getRenderSize().width << "x" << categoryInformation.getRenderSize().height
            << " categoryRect: " << categoryInformation.getCategoryRect().x << "x" << categoryInformation.getCategoryRect().y
            << "x" << categoryInformation.getCategoryRect().width << "x" << categoryInformation.getCategoryRect().height
            << " safeRect: " << categoryInformation.getSafeRect().x << "x" << categoryInformation.getSafeRect().y
            << "x" << categoryInformation.getSafeRect().width << "x" << categoryInformation.getSafeRect().height);

        if (m_categories.find(category) != m_categories.end())
            return addErrorEntry(fmt::format("{}{}{}", "DcsmContentControl::addContentCategory: cannot add category that has already been added before (category: ", category, ")"));

        if (categoryInformation.getCategoryRect().width == 0 ||
            categoryInformation.getCategoryRect().height == 0 ||
            categoryInformation.getRenderSize().width == 0 ||
            categoryInformation.getRenderSize().height == 0)
        {
            return addErrorEntry("DcsmContentControl::addContentCategory: cannot add category without setting at least renderSize and categoryRect for category");
        }

        ramses_internal::CategoryInfo info = categoryInformation.impl.getCategoryInfo();
        m_categories.insert({ category, {info, display, {} }});

        auto it = m_offeredContentsForOtherCategories.begin();
        while (it != m_offeredContentsForOtherCategories.end())
        {
            if (it->category == category)
            {
                // generate events now that these contents are relevant for new category
                contentOffered(it->contentID, it->category, it->contentType);
                // take out from list
                it = m_offeredContentsForOtherCategories.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return StatusOK;
    }

    status_t DcsmContentControlImpl::removeContentCategory(Category category)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::removeContentCategory: category:" << category);

        auto it = m_categories.find(category);
        if (it != m_categories.end())
        {
            const auto copyOfAssignedScenes = it->second.assignedContentIds;
            for (auto& assignedContent : copyOfAssignedScenes)
            {
                auto contentType = m_contents[assignedContent].contentType;
                removeContent(assignedContent);
                m_offeredContentsForOtherCategories.push_back({category, assignedContent, contentType});
            }
            m_categories.erase(it);
        }
        return StatusOK;
    }

    status_t DcsmContentControlImpl::requestContentReady(ContentID contentID, uint64_t timeOut)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:requestContentReady: content " << contentID << " timeOut " << timeOut << " (timeNow=" << m_timeStampNow << ")");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:requestContentReady: cannot set state of unknown content");

        auto& contentInfo = contentIt->second;
        const ContentState currState = determineCurrentContentState(contentID);
        switch (currState)
        {
        case ContentState::Available:
            // send DCSM ready request
            contentInfo.readyRequestTimeOut = (timeOut > 0 ? m_timeStampNow + timeOut : std::numeric_limits<uint64_t>::max());
            CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, { 0, 0 }));
            contentInfo.dcsmState = ContentDcsmState::ReadyRequested;

            // send scene ready request
            requestSceneState(contentID, RendererSceneState::Ready);
            handleContentStateChange(contentID, currState);
            break;
        case ContentState::Ready:
            contentInfo.readyRequestTimeOut = (timeOut > 0 ? m_timeStampNow + timeOut : std::numeric_limits<uint64_t>::max());
            break;
        case ContentState::Shown:
            return addErrorEntry("DcsmContentControl:requestContentReady: content is shown already, cannot request ready");
        case ContentState::Invalid:
            assert(false);
        }

        return StatusOK;
    }

    status_t DcsmContentControlImpl::showContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:showContent: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:showContent: cannot set state of unknown content");

        if (contentIt->second.dcsmState != ContentDcsmState::Ready)
            return addErrorEntry("DcsmContentControl:showContent: cannot show content if it is not ready from Dcsm provider side");

        const ContentState currState = determineCurrentContentState(contentID);
        const ContentState lastState = currState;
        switch (currState)
        {
        case ContentState::Available:
            return addErrorEntry("DcsmContentControl:showContent: cannot show content if it is not ready yet");
        case ContentState::Ready:
        case ContentState::Shown: // execute also if already shown in case timing info is updated
            CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Shown, timingInfo));
            contentIt->second.dcsmState = ContentDcsmState::Shown;

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:showContent: scheduling content" << contentID << "'s scene state change to Rendered at TS " << timingInfo.startTime);
            m_pendingSceneStateChangeCommands[{ contentID, false }] = { timingInfo.startTime, contentIt->second.contentType, getTechnicalContentAssociatedWithContent(contentID), RendererSceneState::Rendered };
            break;
        case ContentState::Invalid:
            assert(false);
        }
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmContentControlImpl::hideContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:hideContent: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:hideContent: cannot set state of unknown content");

        const ContentState currState = determineCurrentContentState(contentID);
        const ContentState lastState = currState;
        switch (currState)
        {
        case ContentState::Available:
            return addErrorEntry("DcsmContentControl:hideContent: content is not shown, cannot hide");
        case ContentState::Ready: // execute also if already hidden (ready) in case timing info is updated
        case ContentState::Shown:
            CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, timingInfo));
            scheduleHideAnimation(contentID, timingInfo, RendererSceneState::Ready);
            contentIt->second.dcsmState = ContentDcsmState::Ready;
            break;
        case ContentState::Invalid:
            assert(false);
        }
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmContentControlImpl::releaseContent(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:releaseContent: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:releaseContent: cannot set state of unknown content");

        const ContentState lastState = determineCurrentContentState(contentID);
        CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Assigned, timingInfo));
        contentIt->second.dcsmState = ContentDcsmState::Assigned;

        if (lastState == ContentState::Shown)
            scheduleHideAnimation(contentID, timingInfo, RendererSceneState::Available);
        else
            requestSceneState(contentID, RendererSceneState::Available);

        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmContentControlImpl::setCategoryInfo(Category categoryId, const CategoryInfoUpdate& categoryInfo, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:setCategoryInfo: category " << categoryId << ", " << categoryInfo << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        auto it = m_categories.find(categoryId);
        if (it == m_categories.end())
            return addErrorEntry("DcsmContentControl:setCategoryInfo: cannot set info of unknown category");

        status_t combinedStat = StatusOK;
        for (const auto content : it->second.assignedContentIds)
        {
            const auto stat = m_dcsmConsumer.contentSizeChange(content, categoryInfo, timingInfo);
            if (stat != StatusOK)
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:setCategoryInfo: failed to set content size on Dcsm consumer for content " << content);
                combinedStat = stat;
            }
        }
        CHECK_RETURN_ERR(combinedStat);

        // merge updated parts of new info to saved info to be able to send merged info to new providers
        it->second.categoryInfo.updateSelf(categoryInfo.impl.getCategoryInfo());

        return StatusOK;
    }

    status_t DcsmContentControlImpl::acceptStopOffer(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:acceptStopOffer: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:acceptStopOffer: cannot set accept stop offer of unknown content");


        const auto status = m_dcsmConsumer.acceptStopOffer(contentID, timingInfo);
        if (status != StatusOK)
            return status;

        if (determineCurrentContentState(contentID) == ContentState::Shown)
            scheduleHideAnimation(contentID, timingInfo, RendererSceneState::Available);
        else
            requestSceneState(contentID, RendererSceneState::Available);

        // no event emitted here, user knows content will be not available anymore

        removeContent(contentID);
        return StatusOK;
    }

    status_t DcsmContentControlImpl::assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:assignContentToDisplayBuffer: content " << contentID << " displayBuffer " << displayBuffer
            << " renderOrder " << renderOrder);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:assignContentToDisplayBuffer: cannot assign unknown content");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
        {
            return addErrorEntry("DcsmContentControl::assignContentToDisplayBuffer: Content of type WaylandIviSurfaceID can not be assigned to a displaybuffer");
        }
        else if (contentIt->second.contentType == ETechnicalContentType::RamsesSceneID)
        {
            const auto techId = getTechnicalContentAssociatedWithContent(contentID);
            if (!techId.isValid() || contentIt->second.dcsmState < ContentDcsmState::Ready)
                return addErrorEntry("DcsmContentControl:assignContentToDisplayBuffer: content must be ready (at least reported as ready from DCSM provider) in order to be able to assign it to a display buffer");

            // Remember sceneId to displaybuffer mapping
            contentIt->second.displayBufferAssignment = displayBuffer;
            return m_sceneControl.setSceneDisplayBufferAssignment(sceneId_t{ techId.getValue() }, displayBuffer, renderOrder);
        }

        return addErrorEntry("DcsmContentControl: cannot assign unknown content type");
    }

    status_t DcsmContentControlImpl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:linkOffscreenBuffer: offscreenBufferId " << offscreenBufferId
            << " consumerContent " << consumerContentID << " consumerId " << consumerId);

        const auto contentIt = m_contents.find(consumerContentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:linkOffscreenBuffer: failed to link offscreen buffer, consumer content unknown.");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl:linkOffscreenBuffer: Linking offscreen buffer cannot be used with content of type WaylandIviSurfaceID");

        const auto consumerTechId = getTechnicalContentAssociatedWithContent(consumerContentID);
        if (!consumerTechId.isValid() || contentIt->second.dcsmState < ContentDcsmState::Ready)
            return addErrorEntry("DcsmContentControl:linkOffscreenBuffer: failed to link offscreen buffer, consumer content's scene unknown. Make content ready at least once before linking.");

        return m_sceneControl.linkOffscreenBuffer(offscreenBufferId, sceneId_t{ consumerTechId.getValue() }, consumerId);
    }

    status_t DcsmContentControlImpl::linkContentToTextureConsumer(ContentID contentID, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:linkContentToTextureConsumer: contentID " << contentID << " consumerContent " << consumerContentID << " consumerId " << consumerId);

        // check consumer validity (consumer is ramses scene also in case of wayland content type)
        const auto consumerContentIt = m_contents.find(consumerContentID);
        if (consumerContentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:linkContentToTextureConsumer: failed to link content, consumer content unknown.");

        if (consumerContentIt->second.contentType != ETechnicalContentType::RamsesSceneID)
            return addErrorEntry("DcsmContentControl:linkContentToTextureConsumer: failed to link to wrong content type. Only possible to link to ramses scene contents.");

        const auto consumerTechId = getTechnicalContentAssociatedWithContent(consumerContentID);
        if (!consumerTechId.isValid() || consumerContentIt->second.dcsmState < ContentDcsmState::Ready || m_techContents[consumerTechId].sharedState.getReportedState() < RendererSceneState::Ready)
            return addErrorEntry("DcsmContentControl:linkContentToTextureConsumer: failed to link content, consumer content's scene is not ready. Make content ready at least once before linking.");

        const sceneId_t consumerSceneId{ consumerTechId.getValue() };
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:linkContentToTextureConsumer: failed to link content, content unknown.");

        const auto technDescriptor = getTechnicalContentAssociatedWithContent(contentID);
        const auto& it = m_techContents.find(technDescriptor);
        if (it == m_techContents.end())
            return addErrorEntry("DcsmContentControl::linkContentToTextureConsumer: failed to link content, could not link technical content");

        if (contentIt->second.contentType == ETechnicalContentType::RamsesSceneID)
        {
            // displaybuffer must have been assigned before
            const displayBufferId_t previouslyAssignedOB = contentIt->second.displayBufferAssignment;
            if (!previouslyAssignedOB.isValid())
                return addErrorEntry("DcsmContentControl:linkContentToTextureConsumer: failed to link content, contentid not assigned to an offscreenbuffer");

            return m_sceneControl.linkOffscreenBuffer(previouslyAssignedOB, consumerSceneId, consumerId);
        }
        else if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
        {
            const waylandIviSurfaceId_t streamId {static_cast<uint32_t>(technDescriptor.getValue())};
            if (!streamId.isValid())
                return addErrorEntry("DcsmContentControl::linkContentToTextureConsumer: failed to link content, contentid unknown or not assigned");

            auto streamBufferIt = m_streamBuffers.find(streamId);
            if (streamBufferIt == m_streamBuffers.end())
                return addErrorEntry("DcsmContentControl::linkContentToTextureConsumer: failed to link content, internal streambuffer not existing");

            const auto& streamBuffer = streamBufferIt->second;

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:linkContentToTextureConsumer: linking streambuffer " << streamBuffer.getValue() << " to consumerScene " << consumerSceneId << " consumerId " << consumerId);
            return m_sceneControl.linkStreamBuffer(streamBuffer, consumerSceneId, consumerId);
        }

        return addErrorEntry("DcsmContentControl:linkContentToTextureConsumer: Cannot link content of unknown type");
    }

    status_t DcsmContentControlImpl::linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:linkData: providerContent " << providerContentID << " providerId " << providerId
            << " consumerContent " << consumerContentID << " consumerId " << consumerId);

        const auto contentItProvider = m_contents.find(providerContentID);
        if (contentItProvider == m_contents.cend())
            return addErrorEntry("DcsmContentControl:linkData: failed to link data, provider content unknown.");
        const auto contentItConsumer = m_contents.find(consumerContentID);
        if (contentItConsumer == m_contents.cend())
            return addErrorEntry("DcsmContentControl:linkData: failed to link data, consumer content unknown.");

        if (contentItProvider->second.contentType == ETechnicalContentType::WaylandIviSurfaceID
                || contentItConsumer->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl:linkData: Data linking cannot be used with content of type WaylandIviSurfaceID");

        const auto providerTechId = getTechnicalContentAssociatedWithContent(providerContentID);
        const auto consumerTechId = getTechnicalContentAssociatedWithContent(consumerContentID);
        if (!providerTechId.isValid())
            return addErrorEntry("DcsmContentControl:linkData: failed to link data, provider content's scene unknown. Make content ready at least once before linking.");
        if (!consumerTechId.isValid())
            return addErrorEntry("DcsmContentControl:linkData: failed to link data, consumer content's scene unknown. Make content ready at least once before linking.");
        sceneId_t providerSceneId{ providerTechId.getValue() };
        sceneId_t consumerSceneId{ consumerTechId.getValue() };
        return m_sceneControl.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
    }

    status_t DcsmContentControlImpl::unlinkData(ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:unlinkData: consumerContentID " << consumerContentID << " consumerId " << consumerId);

        const auto contentItConsumer = m_contents.find(consumerContentID);
        if (contentItConsumer == m_contents.cend())
            return addErrorEntry("DcsmContentControl:unlinkData: failed to unlink data, consumer content unknown.");

        if (contentItConsumer->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl:unlinkData: Data linking cannot be used with content of type WaylandIviSurfaceID");

        const auto consumerTechId = getTechnicalContentAssociatedWithContent(consumerContentID);
        if (!consumerTechId.isValid())
            return addErrorEntry("DcsmContentControl:unlinkData: failed to unlink data, consumer content's scene unknown.");

        return m_sceneControl.unlinkData(sceneId_t{ consumerTechId.getValue() }, consumerId);
    }

    status_t DcsmContentControlImpl::handlePickEvent(ContentID contentID, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:handlePickEvent: content " << contentID << " bufferNormalizedCoordX " << bufferNormalizedCoordX
            << " bufferNormalizedCoordY " << bufferNormalizedCoordY);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl:handlePickEvent: failed to handle pick event, content unknown.");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl:handlePickEvent: Pick event handling cannot be used with content of type WaylandIviSurfaceID");

        const auto sceneTechId = getTechnicalContentAssociatedWithContent(contentID);
        if (!sceneTechId.isValid())
            return addErrorEntry("DcsmContentControl:handlePickEvent: failed to handle pick event, content's scene unknown. Make content ready at least once before picking");

        return m_sceneControl.handlePickEvent(sceneId_t{ sceneTechId.getValue() }, bufferNormalizedCoordX, bufferNormalizedCoordY);
    }

    status_t DcsmContentControlImpl::update(uint64_t timeStampNow, IDcsmContentControlEventHandler& eventHandler)
    {
        if (timeStampNow < m_timeStampNow)
            return addErrorEntry("DcsmContentControl::update called with timeStampNow older than previous timestamp");
        m_timeStampNow = timeStampNow;

        auto stat = m_dcsmConsumer.dispatchEvents(*this);
        if (stat != StatusOK)
            return stat;
        stat = m_sceneControl.dispatchEvents(*this);
        if (stat != StatusOK)
            return stat;
        // todo(jonathan) cleanup with next major version
        stat = m_sceneControl.dispatchSpecialEvents(*this);
        if (stat != StatusOK)
            return stat;
        stat = m_sceneControl.flush();
        if (stat != StatusOK)
            return stat;

        executePendingCommands();
        processTimedOutRequests();
        dispatchPendingEvents(eventHandler);

        return StatusOK;
    }

    void DcsmContentControlImpl::contentOffered(ContentID contentID, Category category, ETechnicalContentType contentType)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentOffered: received DCSM event CONTENT OFFERED for content " << contentID << " category " << category << " content type " << int(contentType));
        auto it = m_categories.find(category);
        if (it != m_categories.end())
        {
            const auto contentIt = m_contents.find(contentID);
            if (contentIt != m_contents.cend() && contentIt->second.category != category)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentOffered: content offered " << contentID << " is already assigned to another category " << contentIt->second.category
                    << ", now requesting category " << category << ". Ignoring, stop offer first before changing category.");
                return;
            }

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentOffered: assigning content " << contentID << " to category " << category);
            auto& catInfo = it->second;
            CategoryInfoUpdate update;
            update.impl.setCategoryInfo(catInfo.categoryInfo);
            auto status = m_dcsmConsumer.assignContentToConsumer(contentID, update);
            if (status != ramses::StatusOK)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentOffered: could not assign content " << contentID << " to category " << category);
                return;
            }
            catInfo.assignedContentIds.insert(contentID);
            m_contents.emplace(std::make_pair(contentID, ContentInfo{ category, contentType, ContentDcsmState::Assigned, {}, {}, 0 }));

            handleContentStateChange(contentID, ContentState::Invalid); // using invalid last state to force event emit
        }
        else
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentOffered: not interested in content offer " << contentID << " for category " << category);
            m_offeredContentsForOtherCategories.push_back({category, contentID, contentType});
        }
    }

    void DcsmContentControlImpl::contentDescription(ContentID contentID, TechnicalContentDescriptor contentDescriptor)
    {
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentDescription: content {} not assigned, ignoring content description {}", contentID, contentDescriptor);
            return;
        }

        ContentInfo& contentInfo = contentIt->second;
        LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentDescription: received DCSM content description for content {} of type {}, content descriptor {}", contentID, contentInfo.contentType, contentDescriptor);

        contentInfo.descriptor = contentDescriptor;
        auto& techContentInfo = m_techContents[contentDescriptor];

        assert(m_categories.count(contentInfo.category) > 0);
        const auto displayId = m_categories.find(contentInfo.category)->second.display;
        assert(!techContentInfo.display.isValid() || techContentInfo.display == displayId);

        techContentInfo.display = displayId;
        techContentInfo.associatedContents.insert(contentID);
        techContentInfo.techType = contentInfo.contentType;

        if (contentInfo.contentType == ETechnicalContentType::WaylandIviSurfaceID)
        {
            if (techContentInfo.sharedState.getReportedState() == RendererSceneState::Unavailable)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::contentDescription: new content descriptor " << contentDescriptor);
                techContentInfo.sharedState.setReportedState(RendererSceneState::Available);
            }

            const waylandIviSurfaceId_t waylandId = waylandIviSurfaceId_t(static_cast<uint32_t>(contentInfo.descriptor.getValue()));
            if (m_streamBuffers.count(waylandId) == 0 && m_availableStreams.count(waylandId) != 0)
            {
                createStreamBuffer(displayId, waylandId);
                techContentInfo.sharedState.setReportedState(RendererSceneState::Ready);
            }

            if (techContentInfo.sharedState.getRequestedState() < RendererSceneState::Ready)
                techContentInfo.sharedState.setRequestedState(RendererSceneState::Ready);
        }

        // already ready requested and just waiting for sceneid?
        if (contentInfo.dcsmState == ContentDcsmState::ReadyRequested)
            requestSceneState(contentID, RendererSceneState::Ready);
        else
            requestSceneState(contentID, RendererSceneState::Available);
    }

    void DcsmContentControlImpl::contentReady(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentReady: received DCSM event CONTENT READY for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentReady: content " << contentID << " not assigned, ignoring state change.");
        }
        else
        {
            assert(m_categories.count(contentIt->second.category) > 0);
            if (contentIt->second.dcsmState == ContentDcsmState::ReadyRequested)
            {
                const ContentState lastState = determineCurrentContentState(contentID);
                contentIt->second.dcsmState = ContentDcsmState::Ready;
                handleContentStateChange(contentID, lastState);
            }
            else
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentReady: content " << contentID << " not requested to be ready or released after request, ignoring state change.");
        }
    }

    void DcsmContentControlImpl::contentStopOfferRequest(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentStopOfferRequest: received DCSM event CONTENT STOP OFFER REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            // no need to confirm, because not assigned
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentStopOfferRequest: content " << contentID << " not assigned, ignoring stopofferrequest.");
            auto itEnd = std::remove_if(m_offeredContentsForOtherCategories.begin(), m_offeredContentsForOtherCategories.end(), [&](const OfferedContents& offer)
            {
                return (offer.contentID == contentID);
            });
            m_offeredContentsForOtherCategories.erase(itEnd, m_offeredContentsForOtherCategories.end());
        }
        else
            m_pendingEvents.push_back({ EventType::ContentStopOfferRequested, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK });
    }

    void DcsmContentControlImpl::forceContentOfferStopped(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:forceContentOfferStopped: received DCSM event FORCE CONTENT OFFER STOPPED for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:forceContentOfferStopped: content " << contentID << " not assigned, nothing to do.");
            auto itEnd = std::remove_if(m_offeredContentsForOtherCategories.begin(), m_offeredContentsForOtherCategories.end(), [&](const OfferedContents& offer)
                {
                    return (offer.contentID == contentID);
                });
            m_offeredContentsForOtherCategories.erase(itEnd, m_offeredContentsForOtherCategories.end());
        }
        else
        {
            requestSceneState(contentID, RendererSceneState::Available);
            removeContent(contentID);
            m_pendingEvents.push_back({ EventType::ContentNotAvailable, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK });
        }
    }

    void DcsmContentControlImpl::contentMetadataUpdated(ContentID contentID, const DcsmMetadataUpdate& metadataUpdate)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentMetadataUpdated: received DCSM event UPDATE CONTENT METADATA for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentMetadataUpdated: content " << contentID << " not assigned, nothing to do.");
        }
        else
            m_pendingEvents.push_back({ EventType::ContentMetadataUpdate, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK, metadataUpdate.impl.getMetadata() });
    }

    void DcsmContentControlImpl::resetContentStateAfterTechnicalContentReset(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:resetContentStateAfterTechnicalContentReset: content " << contentID << " will be reset by unassigning and reassigning");
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return;

        if (StatusOK == m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Offered, {}))
        {
            // signal temporary NotAvailable to user to indicate failure, will be immediately followed by Available event triggered by contentOffered
            m_pendingEvents.push_back({ EventType::ContentNotAvailable, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK });

            const auto category = contentIt->second.category;
            const auto contentType = contentIt->second.contentType;

            removeContent(contentID); // remove all associated states and data as we are supposed to get all (potentially conflicting) data again ramping up
            contentOffered(contentID, category, contentType); // fake the now unknown content being newly offered
        }
        else
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:resetContentStateAfterTechnicalContentReset: error unassigning content " << contentID);

    }

    void DcsmContentControlImpl::applyTechnicalStateChange(TechnicalContentDescriptor techId, RendererSceneState state)
    {
        auto& sceneInfo = m_techContents[techId];
        // first collect current state of all associated contents to be used as 'last' state when checking if changed
        std::unordered_map<ContentID, ContentState> lastStates;
        for (const auto contentID : sceneInfo.associatedContents)
            lastStates[contentID] = determineCurrentContentState(contentID);

        // set new actual scene state which might modify consolidated state of some of the associated contents
        sceneInfo.sharedState.setReportedState(state);
        if (state == RendererSceneState::Unavailable)
            sceneInfo.sharedState.setRequestedState(RendererSceneState::Unavailable);

        // emit event if state changed for any of the associated contents
        std::vector<ContentID> contentsToReset;
        for (const auto contentID : sceneInfo.associatedContents)
        {
            if (state == RendererSceneState::Unavailable)
                contentsToReset.push_back(contentID);
            else
                handleContentStateChange(contentID, lastStates[contentID]);
        }

        for (const auto contentID : contentsToReset)
            resetContentStateAfterTechnicalContentReset(contentID);


        // trigger a potential state change that might result out of the new actual state
        goToConsolidatedDesiredState(techId);
    }

    void DcsmContentControlImpl::sceneStateChanged(sceneId_t sceneId, RendererSceneState state)
    {
        LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:sceneStateChanged: received scene {} state changed event {}", sceneId, state);
        applyTechnicalStateChange(TechnicalContentDescriptor{ sceneId.getValue() }, state);
    }

    void DcsmContentControlImpl::offscreenBufferLinked(displayBufferId_t offscreenBufferId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success)
    {
        LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:offscreenBufferLinked: received offscreen buffer {} linked event to scene consumer {} consumerId {}, success={}.",
            offscreenBufferId, consumerScene, consumerId, success);

        auto contents = findContentsAssociatingSceneId(consumerScene);
        if (contents.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:offscreenBufferLinked: received offscreen buffer linked event but cannot find corresponding consumer content for scene {}"
                " this can happen if content or its scene became unavailable after offscreen buffer linked processed."
                " Event will still be emitted but with content ID set to invalid value.", consumerScene);
            contents.push_back(ContentID::Invalid());
        }
        else if (contents.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl::offscreenBufferLinked: received offscreen buffer linked event for scene " << consumerScene << " which is associated with multiple contents:";
                for (const auto c : contents)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        const auto consumerContent = contents.front();
        LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::offscreenBufferLinked: add event OffscreenBufferLinked - ob {} to consumer {}:{} - {}",
            offscreenBufferId, consumerContent, consumerId, success ? "success" : "failed");
        Event evt{ EventType::OffscreenBufferLinked };
        evt.displayBuffer = offscreenBufferId;
        evt.consumerContentID = consumerContent;
        evt.consumerID = consumerId;
        evt.result = (success ? DcsmContentControlEventResult::OK : DcsmContentControlEventResult::TimedOut);
        m_pendingEvents.push_back(evt);

        const auto providerContentIt = std::find_if(m_contents.begin(), m_contents.end(), [offscreenBufferId](auto const& entry) { return entry.second.displayBufferAssignment == offscreenBufferId; });
        if (providerContentIt != m_contents.end())
        {
            LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::offscreenBufferLinked: add event ContentLinkedToTextureConsumer, provider {} to consumer {}:{} - {}",
                providerContentIt->first, consumerContent, consumerId, success ? "success" : "failed");
            Event evt2{ EventType::ContentLinkedToTextureConsumer };
            evt2.providerContentID = providerContentIt->first;
            evt2.consumerContentID = consumerContent;
            evt2.consumerID = consumerId;
            evt2.result = (success ? DcsmContentControlEventResult::OK : DcsmContentControlEventResult::TimedOut);
            m_pendingEvents.push_back(evt2);
        }
    }

    void DcsmContentControlImpl::streamBufferLinked(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId, bool success)
    {
        LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::streamBufferLinked: streambuffer {}, consumerScene {}, consumer {}, success {}",
            streamBufferId, consumerSceneId, consumerDataSlotId, success);

        auto streamIt = std::find_if(m_streamBuffers.begin(), m_streamBuffers.end(), [streamBufferId](auto& s) { return s.second == streamBufferId; });
        const auto surfaceId = streamIt == m_streamBuffers.end() ? waylandIviSurfaceId_t::Invalid() : streamIt->first;
        const auto providerContents = findContentsAssociatingSurfaceId(surfaceId);
        if (providerContents.empty())
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:streamBufferLinked: received stream buffer linked event but cannot find corresponding provider content for surfaceid {}, streambufferid {}"
                " this can happen if content or its stream became unavailable after stream buffer linked processed."
                " Event will still be emitted but with content ID set to invalid value.", surfaceId, streamBufferId);

        const auto consumerContents = findContentsAssociatingSceneId(consumerSceneId);
        if (consumerContents.empty())
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:streamBufferLinked: received stream buffer linked event but cannot find corresponding consumer content for sceneid {}"
                " this can happen if content or its scene became unavailable after stream buffer linked processed."
                " Event will still be emitted but with content ID set to invalid value.", consumerSceneId);

        Event evt{ EventType::ContentLinkedToTextureConsumer };
        evt.providerContentID = providerContents.empty() ? ContentID::Invalid() : providerContents.front();
        evt.consumerContentID = consumerContents.empty() ? ContentID::Invalid() : consumerContents.front();
        evt.consumerID = consumerDataSlotId;
        evt.result = success ? DcsmContentControlEventResult::OK : DcsmContentControlEventResult::TimedOut;
        m_pendingEvents.push_back(evt);
    }

    void DcsmContentControlImpl::dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success)
    {
        auto contentsProvider = findContentsAssociatingSceneId(providerScene);
        if (contentsProvider.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataLinked: received data linked event but cannot find corresponding provider content for provider scene {},"
                " this can happen if content or its scene became unavailable after data linked processed. Event will still be emitted but with content ID set to invalid value.", providerScene);
            contentsProvider.push_back(ContentID::Invalid());
        }
        else if (contentsProvider.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl:dataLinked: received data linked event for provider scene " << providerScene << " which is associated with multiple contents:";
                for (const auto c : contentsProvider)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        auto contentsConsumer = findContentsAssociatingSceneId(consumerScene);
        if (contentsConsumer.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataLinked: received data linked event but cannot find corresponding consumer content for consumer scene {},"
                " this can happen if content or its scene became unavailable after data linked processed. Event will still be emitted but with content ID set to invalid value.", consumerScene);
            contentsConsumer.push_back(ContentID::Invalid());
        }
        else if (contentsConsumer.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl:dataLinked: received data linked event for consumer scene " << consumerScene << " which is associated with multiple contents:";
                for (const auto c : contentsConsumer)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        const ContentID providerContent = contentsProvider.front();
        const ContentID consumerContent = contentsConsumer.front();

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataLinked: data linked - provider content " << providerContent << " dataId " << providerId
            << " to consumer content " << consumerContent << " dataId " << consumerId);
        Event evt{ EventType::DataLinked };
        evt.providerContentID = providerContent;
        evt.consumerContentID = consumerContent;
        evt.providerID = providerId;
        evt.consumerID = consumerId;
        evt.result = (success ? DcsmContentControlEventResult::OK : DcsmContentControlEventResult::TimedOut );
        m_pendingEvents.push_back(evt);
    }

    void DcsmContentControlImpl::dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, bool success)
    {
        auto contentsConsumer = findContentsAssociatingSceneId(consumerScene);
        if (contentsConsumer.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataUnlinked: received data unlinked event but cannot find corresponding consumer content for consumer scene {},"
                " this can happen if content or its scene became unavailable after data unlinked processed. Event will still be emitted but with content ID set to invalid value.", consumerScene);
            contentsConsumer.push_back(ContentID::Invalid());
        }
        else if (contentsConsumer.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl:dataUnlinked: received data unlinked event for consumer scene " << consumerScene << " which is associated with multiple contents:";
                for (const auto c : contentsConsumer)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        const ContentID consumerContent = contentsConsumer.front();

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataUnlinked: data unlinked - consumer content " << consumerContent << " dataId " << consumerId);
        Event evt{ EventType::DataUnlinked };
        evt.consumerContentID = consumerContent;
        evt.consumerID = consumerId;
        evt.result = (success ? DcsmContentControlEventResult::OK : DcsmContentControlEventResult::TimedOut);
        m_pendingEvents.push_back(evt);
    }

    void DcsmContentControlImpl::objectsPicked(sceneId_t scene, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount)
    {
        auto contents = findContentsAssociatingSceneId(scene);
        if (contents.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:objectsPicked: received objects picked event but cannot find corresponding content for scene {},"
                " this can happen if content or its scene became unavailable after objects picked processed. Event will still be emitted but with content ID set to invalid value.", scene);
            contents.push_back(ContentID::Invalid());
        }
        else if (contents.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl:objectsPicked: received " << pickedObjectsCount << " objects picked event for scene " << scene << " which is associated with multiple contents:";
                for (const auto c : contents)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }
        else
            LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:objectsPicked: {} objects picked, scene {}, content {}", pickedObjectsCount, scene, contents.front());

        Event evt{ EventType::ObjectsPicked };
        evt.contentID = contents.front();
        evt.pickedObjectIds = std::vector<pickableObjectId_t>(pickedObjects, pickedObjects + pickedObjectsCount);
        m_pendingEvents.push_back(std::move(evt));
    }

    void DcsmContentControlImpl::dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataProviderCreated: received data provider created event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataProviderCreated: data provider " << dataProviderId << " created in content " << content);
            Event evt{ EventType::DataProviderCreated };
            evt.providerContentID = content;
            evt.providerID = dataProviderId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataProviderDestroyed: received data provider destroyed event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataProviderDestroyed: data provider " << dataProviderId << " destroyed in content " << content);
            Event evt{ EventType::DataProviderDestroyed };
            evt.providerContentID = content;
            evt.providerID = dataProviderId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataConsumerCreated: received data consumer created event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataConsumerCreated: data consumer " << dataConsumerId << " created in content " << content);
            Event evt{ EventType::DataConsumerCreated };
            evt.consumerContentID = content;
            evt.consumerID = dataConsumerId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataConsumerDestroyed: received data consumer destroyed event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:dataConsumerDestroyed: data consumer " << dataConsumerId << " destroyed in content " << content);
            Event evt{ EventType::DataConsumerDestroyed };
            evt.consumerContentID = content;
            evt.consumerID = dataConsumerId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:sceneFlushed: received scene flushed event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:sceneFlushed: content " << content << " flushed with version " << sceneVersionTag);
            Event evt{ EventType::ContentFlushed };
            evt.contentID = content;
            evt.version = sceneVersionTag;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::sceneExpirationMonitoringEnabled(sceneId_t sceneId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received scene expiration monitoring enabled event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << content << " expiration monitoring enabled");
            Event evt{ EventType::ContentExpirationMonitoringEnabled };
            evt.contentID = content;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::sceneExpirationMonitoringDisabled(sceneId_t sceneId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received scene expiration monitoring disabled event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << content << " expiration monitoring disabled");
            Event evt{ EventType::ContentExpirationMonitoringDisabled };
            evt.contentID = content;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::sceneExpired(sceneId_t sceneId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:sceneExpired: received scene expired event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:sceneExpired: content " << content << " expired");
            Event evt{ EventType::ContentExpired };
            evt.contentID = content;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::sceneRecoveredFromExpiration(sceneId_t sceneId)
    {
        auto contents = findContentsAssociatingSceneId(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:sceneRecoveredFromExpiration: received scene recovered from expiration event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:sceneRecoveredFromExpiration: content " << content << " recovered from expiration");
            Event evt{ EventType::ContentRecoveredFromExpiration };
            evt.contentID = content;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:streamAvailabilityChanged: stream " << streamId << " availability changed to " << available);
        RendererSceneState state = RendererSceneState::Available;
        if (available)
        {
            auto& sceneInfo = m_techContents[TechnicalContentDescriptor{streamId.getValue()}];
            if (m_availableStreams.count(streamId) > 0)
                LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::streamAvailabilityChanged: stream {} was available already", streamId);
            m_availableStreams.insert(streamId);

            if (sceneInfo.sharedState.getRequestedState() == RendererSceneState::Ready)
            {
                state = RendererSceneState::Ready;
                if (m_streamBuffers.count(streamId) == 0)
                    createStreamBuffer(sceneInfo.display, streamId);
            }
        }
        else
        {
            state = RendererSceneState::Unavailable;
            if (m_availableStreams.count(streamId) == 0)
                LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::streamAvailabilityChanged: unavailable stream {} received another unavailable event", streamId);
            m_availableStreams.erase(streamId);
        }

        applyTechnicalStateChange(TechnicalContentDescriptor{ streamId.getValue() }, state);

        Event evt{ EventType::StreamAvailable };
        evt.streamSource = streamId;
        evt.streamAvailable = available;
        m_pendingEvents.push_back(evt);
    }

    void DcsmContentControlImpl::createStreamBuffer(displayId_t displayId, waylandIviSurfaceId_t surfaceId)
    {
        streamBufferId_t sbufferId = m_sceneControl.createStreamBuffer(displayId, surfaceId);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::createStreamBuffer: created streambuffer " << sbufferId << " for ivi id " << surfaceId);
        m_streamBuffers.insert({ surfaceId, sbufferId });
    }

    void DcsmContentControlImpl::executePendingCommands()
    {
        for (auto it = m_pendingSceneStateChangeCommands.begin(); it != m_pendingSceneStateChangeCommands.end();)
        {
            SceneStateChangeCommand const& cmd = it->second;
            if (cmd.timePoint > m_timeStampNow)
                ++it;
            else
            {
                ContentID const& contentId = it->first.contentId;
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::executePendingCommands executing scheduled command for content " << contentId << " to request scene " << cmd.technicalId << " state change to " << EnumToString(cmd.sceneState));
                const auto lastState = determineCurrentContentState(contentId);

                assert(cmd.technicalId.isValid());
                auto& sharedState = m_techContents[cmd.technicalId].sharedState;
                sharedState.setDesiredState(it->first, cmd.sceneState);
                goToConsolidatedDesiredState(cmd.technicalId);
                handleContentStateChange(contentId, lastState);
                it = m_pendingSceneStateChangeCommands.erase(it);
            }
        }
    }

    void DcsmContentControlImpl::dispatchPendingEvents(IDcsmContentControlEventHandler& eventHandler)
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
                    eventHandler.contentAvailable(evt.contentID, evt.category);
                    break;
                case ContentState::Ready:
                    eventHandler.contentReady(evt.contentID, evt.result);
                    break;
                case ContentState::Shown:
                    eventHandler.contentShown(evt.contentID);
                    break;
                case ContentState::Invalid:
                    assert(false);
                    break;
                }
                break;
            case EventType::ContentEnableFocusRequest:
                eventHandler.contentEnableFocusRequest(evt.contentID, evt.focusRequest);
                break;
            case EventType::ContentDisableFocusRequest:
                eventHandler.contentDisableFocusRequest(evt.contentID, evt.focusRequest);
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
                eventHandler.offscreenBufferLinked(evt.displayBuffer, evt.consumerContentID, evt.consumerID, evt.result == DcsmContentControlEventResult::OK);
                break;
            case EventType::DataLinked:
                eventHandler.dataLinked(evt.providerContentID, evt.providerID, evt.consumerContentID, evt.consumerID, evt.result == DcsmContentControlEventResult::OK);
                break;
            case EventType::DataUnlinked:
                eventHandler.dataUnlinked(evt.consumerContentID, evt.consumerID, evt.result == DcsmContentControlEventResult::OK);
                break;
            case EventType::ContentFlushed:
                eventHandler.contentFlushed(evt.contentID, evt.version);
                break;
            case EventType::DataProviderCreated:
                eventHandler.dataProviderCreated(evt.providerContentID, evt.providerID);
                break;
            case EventType::DataProviderDestroyed:
                eventHandler.dataProviderDestroyed(evt.providerContentID, evt.providerID);
                break;
            case EventType::DataConsumerCreated:
                eventHandler.dataConsumerCreated(evt.consumerContentID, evt.consumerID);
                break;
            case EventType::DataConsumerDestroyed:
                eventHandler.dataConsumerDestroyed(evt.consumerContentID, evt.consumerID);
                break;
            case EventType::ContentExpirationMonitoringEnabled:
                eventHandler.contentExpirationMonitoringEnabled(evt.contentID);
                break;
            case EventType::ContentExpirationMonitoringDisabled:
                eventHandler.contentExpirationMonitoringDisabled(evt.contentID);
                break;
            case EventType::ContentExpired:
                eventHandler.contentExpired(evt.contentID);
                break;
            case EventType::ContentRecoveredFromExpiration:
                eventHandler.contentRecoveredFromExpiration(evt.contentID);
                break;
            case EventType::StreamAvailable:
                eventHandler.streamAvailabilityChanged(evt.streamSource, evt.streamAvailable);
                break;
            case EventType::ObjectsPicked:
                eventHandler.objectsPicked(evt.contentID, evt.pickedObjectIds.data(), static_cast<uint32_t>(evt.pickedObjectIds.size()));
                break;
            case EventType::ContentLinkedToTextureConsumer:
                eventHandler.contentLinkedToTextureConsumer(evt.providerContentID, evt.consumerContentID, evt.consumerID, (evt.result == DcsmContentControlEventResult::OK));
                break;
            }
        }
    }

    void DcsmContentControlImpl::goToConsolidatedDesiredState(TechnicalContentDescriptor techId)
    {
        if (m_techContents[techId].techType == ETechnicalContentType::RamsesSceneID)
            goToConsolidatedDesiredSceneState(techId);
        else if(m_techContents[techId].techType == ETechnicalContentType::WaylandIviSurfaceID)
            goToConsolidatedDesiredStreamState(techId);
    }

    void DcsmContentControlImpl::goToConsolidatedDesiredSceneState(TechnicalContentDescriptor techId)
    {
        const sceneId_t sceneId{ techId.getValue() };
        auto& sharedState = m_techContents[techId].sharedState;
        const auto reportedState = sharedState.getReportedState();
        const auto consolidatedDesiredState = sharedState.getConsolidatedDesiredState();
        const auto requestedState = sharedState.getRequestedState();

        const bool mustRequest = (requestedState == reportedState || requestedState == RendererSceneState::Unavailable ) && // scene control is not doing anything currently
                                  reportedState != consolidatedDesiredState; // we want another state than the current one

        if (mustRequest &&
            reportedState != RendererSceneState::Unavailable && // can't do anything if scene isn't available/published
            consolidatedDesiredState != RendererSceneState::Unavailable) // unavailable is not a legal state to request
        {
            if (consolidatedDesiredState >= RendererSceneState::Ready && reportedState < RendererSceneState::Ready)
            {
                const auto displayId = m_techContents[techId].display;
                assert(displayId.isValid());
                if (m_sceneControl.setSceneMapping(sceneId, displayId) != StatusOK)
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:goToConsolidatedDesiredSceneState: failed to set scene mapping for scene " << sceneId);
            }

            // make sure to go only one state at a time in direction of our target state
            RendererSceneState newState = reportedState == RendererSceneState::Ready ? consolidatedDesiredState : RendererSceneState::Ready;
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:goToConsolidatedDesiredSceneState: requesting scene state change for scene " << sceneId << " to " << EnumToString(newState));
            if (m_sceneControl.setSceneState(sceneId, newState) == StatusOK)
                sharedState.setRequestedState(newState);
            else
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:goToConsolidatedDesiredSceneState: failed to request state for scene " << sceneId << " to " << EnumToString(newState));
        }
    }

    void DcsmContentControlImpl::goToConsolidatedDesiredStreamState(TechnicalContentDescriptor techId)
    {
        const waylandIviSurfaceId_t waylandId{ static_cast<uint32_t>(techId.getValue()) };
        auto& sharedState = m_techContents[techId].sharedState;
        const auto reportedState = sharedState.getReportedState();
        const auto consolidatedDesiredState = sharedState.getConsolidatedDesiredState();
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::goToConsolidatedDesiredStreamState: wid:" << waylandId << "reported: "<<static_cast<int>(reportedState) << "consolidatedDesiredState"<< static_cast<int>(consolidatedDesiredState));
        const bool mustRequest = reportedState != consolidatedDesiredState; // we want another state than the current one

        auto streamBufferIt = m_streamBuffers.find(waylandId);
        if (streamBufferIt == m_streamBuffers.end())
        {
            LOG_ERROR_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::goToConsolidatedDesiredStreamState: stream buffer for surface id {} not valid", waylandId);
            return;
        }

        auto& streambufferid = streamBufferIt->second;
        auto displayId = m_techContents[techId].display;
        assert(displayId.isValid());

        if (mustRequest &&
            reportedState != RendererSceneState::Unavailable && // can't do anything if stream isn't available
            consolidatedDesiredState != RendererSceneState::Unavailable) // unavailable is not a legal state to request
        {
            if (reportedState == RendererSceneState::Ready && consolidatedDesiredState == RendererSceneState::Rendered)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::goToConsolidatedDesiredStreamState: enable streambuffer " << streambufferid);
                if (m_sceneControl.setStreamBufferState(displayId, streambufferid, true) == StatusOK)
                    sharedState.setRequestedState(consolidatedDesiredState);
            }

            if (reportedState == RendererSceneState::Rendered && consolidatedDesiredState < RendererSceneState::Rendered)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::goToConsolidatedDesiredStreamState: disable streambuffer " << streambufferid);
                if (m_sceneControl.setStreamBufferState(displayId, streambufferid, false) == StatusOK)
                    sharedState.setRequestedState(consolidatedDesiredState);
            }

            if (consolidatedDesiredState == RendererSceneState::Available)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::goToConsolidatedDesiredStreamState: remove streambuffer " << streambufferid);
                if (m_sceneControl.destroyStreamBuffer(displayId, streambufferid) == StatusOK)
                    sharedState.setRequestedState(consolidatedDesiredState);

                m_streamBuffers.erase(waylandId);
                sharedState.setReportedState(RendererSceneState::Available);
            }
        }
    }

    void DcsmContentControlImpl::requestSceneState(ContentID contentID, RendererSceneState state)
    {
        const auto technicalId = getTechnicalContentAssociatedWithContent(contentID);
        if (technicalId.isValid())
        {
            auto& sharedState = m_techContents[technicalId].sharedState;
            sharedState.setDesiredState({ contentID, false }, state);
            goToConsolidatedDesiredState(technicalId);
        }
    }

    void DcsmContentControlImpl::handleContentStateChange(ContentID contentID, ContentState lastState)
    {
        const ContentState currState = determineCurrentContentState(contentID);
        if (currState != lastState)
        {
            assert(m_contents.count(contentID) > 0);
            auto& contentInfo = m_contents.find(contentID)->second;

            if (currState == ContentState::Available || currState == ContentState::Invalid)
                contentInfo.displayBufferAssignment = {};

            if (lastState == ContentState::Available && currState == ContentState::Ready)
                contentInfo.readyRequestTimeOut = std::numeric_limits<uint64_t>::max();

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:handleContentStateChange: content " << contentID << " state changed from " << ContentStateName(lastState) << " to " << ContentStateName(currState));
            m_pendingEvents.push_back({ EventType::ContentStateChanged, contentID, contentInfo.category, currState, lastState, DcsmContentControlEventResult::OK });
        }
    }

    void DcsmContentControlImpl::removeContent(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:removeContent: removing content " << contentID << " and any pending commands associated with it");
        m_contents.erase(contentID);
        for (auto& catIt : m_categories)
            catIt.second.assignedContentIds.erase(contentID);
        for (auto& techIt : m_techContents)
            techIt.second.associatedContents.erase(contentID);
    }

    void DcsmContentControlImpl::scheduleHideAnimation(ContentID contentID, AnimationInformation animTime, RendererSceneState targetState)
    {
        const auto techId = getTechnicalContentAssociatedWithContent(contentID);
        // keep content shown for hide animation
        auto techContentIt = m_techContents.find(techId);
        assert(techContentIt != m_techContents.end());
        techContentIt->second.sharedState.setDesiredState({ contentID, true }, RendererSceneState::Rendered);
        // drop desired scene state actual content
        techContentIt->second.sharedState.setDesiredState({ contentID, false }, targetState);
        // schedule desired scene state to drop for content hide animation at animation end time
        m_pendingSceneStateChangeCommands[{ contentID, true }] = { animTime.finishTime, techContentIt->second.techType, techId, RendererSceneState::Available };
    }

    ContentState DcsmContentControlImpl::determineCurrentContentState(ContentID contentID) const
    {
        if (m_contents.count(contentID) == 0)
            return ContentState::Invalid;
        const auto techDescriptor = getTechnicalContentAssociatedWithContent(contentID);
        if (!techDescriptor.isValid())
            return ContentState::Available;

        // if scene is available for content, check the shared scene state to determine content compound state
        const auto dcsmState = m_contents.find(contentID)->second.dcsmState;
        const auto reportedSceneState = m_techContents.find(techDescriptor)->second.sharedState.getCurrentStateForContent({ contentID, false });
        const auto requestedSceneState = m_techContents.find(techDescriptor)->second.sharedState.getRequestedState();

        static_assert(ContentDcsmState::Assigned < ContentDcsmState::ReadyRequested && ContentDcsmState::ReadyRequested < ContentDcsmState::Ready && ContentDcsmState::Ready < ContentDcsmState::Shown, "update logic below");
        static_assert(RendererSceneState::Unavailable < RendererSceneState::Available && RendererSceneState::Available < RendererSceneState::Ready && RendererSceneState::Ready < RendererSceneState::Rendered, "update logic below");

        // if any state is below ready, overall state is not ready
        if (dcsmState < ContentDcsmState::Ready || reportedSceneState < RendererSceneState::Ready || requestedSceneState < RendererSceneState::Ready)
            return ContentState::Available;

        // both states are ready or shown. if one of them is only ready, overall state is only ready.
        if (dcsmState == ContentDcsmState::Ready || reportedSceneState == RendererSceneState::Ready || requestedSceneState == RendererSceneState::Ready)
            return ContentState::Ready;

        return ContentState::Shown;
    }

    void DcsmContentControlImpl::processTimedOutRequests()
    {
        for (auto& contentIt : m_contents)
        {
            const auto contentID = contentIt.first;
            auto& contentInfo = contentIt.second;
            if ((contentInfo.dcsmState == ContentDcsmState::ReadyRequested || contentInfo.dcsmState == ContentDcsmState::Ready) && m_timeStampNow > contentInfo.readyRequestTimeOut)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:processTimedOutRequests: ready request for content " << contentID << " timed out (timeoutTS=" << contentInfo.readyRequestTimeOut
                    << " nowTS=" << m_timeStampNow << "), releasing content. Request ready again for re-try.");
                m_pendingEvents.push_back({ EventType::ContentStateChanged, contentID, Category{0}, ContentState::Ready, ContentState::Available, DcsmContentControlEventResult::TimedOut });
                contentInfo.dcsmState = ContentDcsmState::Assigned;
                contentInfo.readyRequestTimeOut = std::numeric_limits<uint64_t>::max();

                // scene control is potentially still trying to make the scene ready, but wasn't able to do so until the timeout hit
                // it is also possible that making the scene ready failed and no further event from scene control comes
                // but it is also possible that it comes right after the timeout
                // we have to manually reset the requested state back to previously reported one, this will unblock
                // new calls to RendererSceneControl::setSceneState to reach the new desired state set by releaseContent
                // also request that scene state on scene control to make sure that scene control will eventually reach that state
                const auto techId = getTechnicalContentAssociatedWithContent(contentID);
                auto& sharedState = m_techContents[techId].sharedState;
                if (sharedState.getReportedState() != sharedState.getRequestedState())
                {
                    if (m_techContents[techId].techType == ETechnicalContentType::RamsesSceneID)
                    {
                        if (m_sceneControl.setSceneState(sceneId_t{ techId.getValue() }, sharedState.getReportedState()) == StatusOK)
                            sharedState.setRequestedState(sharedState.getReportedState());
                    }
                    else if (m_techContents[techId].techType == ETechnicalContentType::WaylandIviSurfaceID)
                        sharedState.setRequestedState(sharedState.getReportedState());
                }
                else
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl::processTimedOutRequests: failed to request state for scene " << techId << " to " << EnumToString(sharedState.getReportedState()));

                releaseContent(contentID, { 0, 0 });
            }
        }
    }

    TechnicalContentDescriptor DcsmContentControlImpl::getTechnicalContentAssociatedWithContent(ContentID contentID) const
    {
        const auto& it = m_contents.find(contentID);
        assert(it != m_contents.end());
        if (it == m_contents.end())
            return {};
        return it->second.descriptor;
    }

    std::vector<ContentID> DcsmContentControlImpl::findContentsAssociatingSceneId(sceneId_t sceneId) const
    {
        return findContentsAssociatingTechnicalIdAndType(TechnicalContentDescriptor{ sceneId.getValue() }, ETechnicalContentType::RamsesSceneID);
    }

    std::vector<ContentID> DcsmContentControlImpl::findContentsAssociatingSurfaceId(waylandIviSurfaceId_t surfaceId) const
    {
        return findContentsAssociatingTechnicalIdAndType(TechnicalContentDescriptor{ surfaceId.getValue() }, ETechnicalContentType::WaylandIviSurfaceID);
    }

    std::vector<ContentID> DcsmContentControlImpl::findContentsAssociatingTechnicalIdAndType(TechnicalContentDescriptor descriptor, ETechnicalContentType type) const
    {
        std::vector<ContentID> contents;
        const auto it = std::find_if(m_techContents.begin(), m_techContents.end(), [descriptor, type](auto const& entry){ return entry.first == descriptor && entry.second.techType == type; });
        if (it != m_techContents.cend())
            contents.assign(it->second.associatedContents.cbegin(), it->second.associatedContents.cend());

        return contents;
    }


    void DcsmContentControlImpl::contentEnableFocusRequest(ramses::ContentID contentID, int32_t focusRequest)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentEnableFocusRequest: received DCSM event CONTENT ENABLE FOCUS REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentEnableFocusRequest: content " << contentID << " not assigned, ignoring request.");
        }
        else
            m_pendingEvents.push_back({EventType::ContentEnableFocusRequest, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK, {}, {}, {}, {}, {}, {}, {}, {}, {},  focusRequest});
    }

    void DcsmContentControlImpl::contentDisableFocusRequest(ramses::ContentID contentID, int32_t focusRequest)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentDisableFocusRequest: received DCSM event CONTENT DISABLE FOCUS REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:contentDisableFocusRequest: content " << contentID << " not assigned, ignoring request.");
        }
        else
            m_pendingEvents.push_back({EventType::ContentDisableFocusRequest, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK, {}, {}, {}, {}, {}, {}, {}, {}, {},  focusRequest});
    }

    void DcsmContentControlImpl::streamBufferEnabled(streamBufferId_t streamBufferId, bool status)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::streamBufferEnabled: streambuffer " << streamBufferId << " status:" << status);
        auto streamIt = std::find_if(m_streamBuffers.begin(), m_streamBuffers.end(), [streamBufferId](auto& s) { return s.second == streamBufferId; });
        if (streamIt == m_streamBuffers.end())
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControlImpl::streamBufferEnabled: No stream buffer found for stream buffer id " << streamBufferId);
            return;
        }

        auto state = status ? RendererSceneState::Rendered : RendererSceneState::Ready;
        applyTechnicalStateChange(TechnicalContentDescriptor{ streamIt->first.getValue() }, state);
    }

    status_t DcsmContentControlImpl::sendContentStatus(ContentID contentID, DcsmStatusMessage const& message)
    {
        return m_dcsmConsumer.sendContentStatus(contentID, message);
    }
}
