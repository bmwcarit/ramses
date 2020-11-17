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
#include "DcsmMetadataUpdateImpl.h"
#include "IDcsmConsumerImpl.h"
#include "RendererSceneControlImpl.h"
#include "RamsesFrameworkTypesImpl.h"
#include "Utils/LogMacros.h"
#include "Utils/LoggingUtils.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "ramses-framework-api/DcsmApiTypes.h"
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

        if (!categoryInformation.getCategoryRect().width || !categoryInformation.getCategoryRect().height || !categoryInformation.getRenderSize().width || !categoryInformation.getRenderSize().height)
            return addErrorEntry("DcsmContentControl::addContentCategory: cannot add category without setting at least renderSize and categoryRect for category");

        ramses_internal::CategoryInfo info;
        info.setCategoryRect(0, 0, categoryInformation.getCategoryRect().width, categoryInformation.getCategoryRect().height);
        info.setRenderSize(categoryInformation.getRenderSize().width, categoryInformation.getRenderSize().height);
        info.setSafeRect(categoryInformation.getSafeRect().x, categoryInformation.getSafeRect().y, categoryInformation.getSafeRect().width, categoryInformation.getSafeRect().height);
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
            return addErrorEntry("DcsmContentControl: cannot set state of unknown content");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Content of type WaylandIviSurfaceID can currently not be controlled with DcsmContentControl, use DcsmConsumer directly");

        auto& contentInfo = contentIt->second;
        const ContentState currState = getCurrentState(contentID);
        switch (currState)
        {
        case ContentState::Available:
            contentInfo.readyRequested = true;
            contentInfo.readyRequestTimeOut = (timeOut > 0 ? m_timeStampNow + timeOut : std::numeric_limits<uint64_t>::max());
            // send DCSM ready request
            if (!contentInfo.dcsmReady)
                CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, { 0, 0 }));
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
            return addErrorEntry("DcsmContentControl: cannot set state of unknown content");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Content of type WaylandIviSurfaceID can currently not be controlled with DcsmContentControl, use DcsmConsumer directly");

        if (!contentIt->second.dcsmReady)
            return addErrorEntry("DcsmContentControl:showContent: cannot show content if it is not ready from Dcsm provider side");

        const ContentState currState = getCurrentState(contentID);
        const ContentState lastState = currState;
        switch (currState)
        {
        case ContentState::Available:
            return addErrorEntry("DcsmContentControl:showContent: cannot show content if it is not ready yet");
        case ContentState::Ready:
        case ContentState::Shown: // execute also if already shown in case timing info is updated
            CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Shown, timingInfo));
            scheduleSceneStateChange(contentID, RendererSceneState::Rendered, timingInfo.startTime);
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
            return addErrorEntry("DcsmContentControl: cannot set state of unknown content");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Content of type WaylandIviSurfaceID can currently not be controlled with DcsmContentControl, use DcsmConsumer directly");

        const ContentState currState = getCurrentState(contentID);
        const ContentState lastState = currState;
        switch (currState)
        {
        case ContentState::Available:
            return addErrorEntry("DcsmContentControl:hideContent: content is not shown, cannot hide");
        case ContentState::Ready: // execute also if already hidden (ready) in case timing info is updated
        case ContentState::Shown:
            CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Ready, timingInfo));
            scheduleSceneStateChange(contentID, RendererSceneState::Ready, timingInfo.finishTime);
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
            return addErrorEntry("DcsmContentControl: cannot set state of unknown content");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Content of type WaylandIviSurfaceID can currently not be controlled with DcsmContentControl, use DcsmConsumer directly");

        const ContentState lastState = getCurrentState(contentID);
        CHECK_RETURN_ERR(m_dcsmConsumer.contentStateChange(contentID, EDcsmState::Assigned, timingInfo));
        contentIt->second.dcsmReady = false;
        contentIt->second.readyRequested = false;

        scheduleSceneStateChange(contentID, RendererSceneState::Available, timingInfo.finishTime);
        handleContentStateChange(contentID, lastState);

        return StatusOK;
    }

    status_t DcsmContentControlImpl::setCategoryInfo(Category categoryId, const CategoryInfoUpdate& categoryInfo, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:setCategoryInfo: category " << categoryId << ", " << categoryInfo << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        auto it = m_categories.find(categoryId);
        if (it == m_categories.end())
            return addErrorEntry("DcsmContentControl: cannot set info of unknown category");

        status_t combinedStat = StatusOK;
        for (const auto content : it->second.assignedContentIds)
        {
            const auto stat = m_dcsmConsumer.contentSizeChange(content, categoryInfo, timingInfo);
            if (stat != StatusOK)
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: failed to set content size on Dcsm consumer for content " << content);
                combinedStat = stat;
            }
        }
        CHECK_RETURN_ERR(combinedStat);

        // size updated immediately in order to report the latest size to newly offered contents to this category
        it->second.categoryInfo = categoryInfo.impl.getCategoryInfo();

        return StatusOK;
    }

    status_t DcsmContentControlImpl::acceptStopOffer(ContentID contentID, AnimationInformation timingInfo)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:acceptStopOffer: content " << contentID << " timing <" << timingInfo.startTime << ";" << timingInfo.finishTime << ">");
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl: cannot set accept stop offer of unknown content");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Content of type WaylandIviSurfaceID can currently not be controlled with DcsmContentControl, use DcsmConsumer directly");

        const auto status = m_dcsmConsumer.acceptStopOffer(contentID, timingInfo);
        if (status != StatusOK)
            return status;

        contentIt->second.dcsmReady = false;
        contentIt->second.readyRequested = false;
        scheduleSceneStateChange(contentID, RendererSceneState::Available, timingInfo.finishTime);

        Command cmd{ CommandType::RemoveContent, timingInfo.finishTime };
        cmd.contentId = contentID;
        m_pendingCommands.push_back(cmd);

        return StatusOK;
    }

    status_t DcsmContentControlImpl::assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:assignContentToDisplayBuffer: content " << contentID << " displayBuffer " << displayBuffer
            << " renderOrder " << renderOrder);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl: cannot assign unknown content");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Content of type WaylandIviSurfaceID can currently not be controlled with DcsmContentControl, use DcsmConsumer directly");

        const auto sceneId = findSceneAssociatedWithContent(contentID);
        if (!sceneId.isValid() || !contentIt->second.dcsmReady)
            return addErrorEntry("DcsmContentControl: content must be ready (at least reported as ready from DCSM provider) in order to be able to assign it to a display buffer");

        return m_sceneControl.setSceneDisplayBufferAssignment(sceneId, displayBuffer, renderOrder);
    }

    status_t DcsmContentControlImpl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:linkOffscreenBuffer: offscreenBufferId " << offscreenBufferId
            << " consumerContent " << consumerContentID << " consumerId " << consumerId);

        const auto contentIt = m_contents.find(consumerContentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl: failed to link offscreen buffer, consumer content unknown.");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Linking offscreen buffer cannot be used with content of type WaylandIviSurfaceID");

        const auto consumerSceneId = findSceneAssociatedWithContent(consumerContentID);
        if (!consumerSceneId.isValid() || !contentIt->second.dcsmReady)
            return addErrorEntry("DcsmContentControl: failed to link offscreen buffer, consumer content's scene unknown. Make content ready at least once before linking.");

        return m_sceneControl.linkOffscreenBuffer(offscreenBufferId, consumerSceneId, consumerId);
    }


    status_t DcsmContentControlImpl::linkContentToTextureConsumer(ContentID /*contentID*/, ContentID /*consumerContentID*/, dataConsumerId_t /*consumerId*/)
    {
        LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:linkContentToTextureConsumer: not implemented yet.");
        return addErrorEntry("DcsmContentControl:linkContentAsTexture: not implemented yet");
    }


    status_t DcsmContentControlImpl::linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:linkData: providerContent " << providerContentID << " providerId " << providerId
            << " consumerContent " << consumerContentID << " consumerId " << consumerId);

        const auto contentItProvider = m_contents.find(providerContentID);
        if (contentItProvider == m_contents.cend())
            return addErrorEntry("DcsmContentControl: failed to link data, provider content unknown.");
        const auto contentItConsumer = m_contents.find(consumerContentID);
        if (contentItConsumer == m_contents.cend())
            return addErrorEntry("DcsmContentControl: failed to link data, consumer content unknown.");

        if (contentItProvider->second.contentType == ETechnicalContentType::WaylandIviSurfaceID
                || contentItConsumer->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Data linking cannot be used with content of type WaylandIviSurfaceID");

        const auto providerSceneId = findSceneAssociatedWithContent(providerContentID);
        const auto consumerSceneId = findSceneAssociatedWithContent(consumerContentID);
        if (!providerSceneId.isValid())
            return addErrorEntry("DcsmContentControl: failed to link data, provider content's scene unknown. Make content ready at least once before linking.");
        if (!consumerSceneId.isValid())
            return addErrorEntry("DcsmContentControl: failed to link data, consumer content's scene unknown. Make content ready at least once before linking.");

        return m_sceneControl.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
    }

    status_t DcsmContentControlImpl::unlinkData(ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:unlinkData: consumerContentID " << consumerContentID << " consumerId " << consumerId);

        const auto contentItConsumer = m_contents.find(consumerContentID);
        if (contentItConsumer == m_contents.cend())
            return addErrorEntry("DcsmContentControl: failed to unlink data, consumer content unknown.");

        if (contentItConsumer->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Data linking cannot be used with content of type WaylandIviSurfaceID");

        const auto consumerSceneId = findSceneAssociatedWithContent(consumerContentID);
        if (!consumerSceneId.isValid())
            return addErrorEntry("DcsmContentControl: failed to unlink data, consumer content's scene unknown.");

        return m_sceneControl.unlinkData(consumerSceneId, consumerId);
    }

    status_t DcsmContentControlImpl::handlePickEvent(ContentID contentID, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl:handlePickEvent: content " << contentID << " bufferNormalizedCoordX " << bufferNormalizedCoordX
            << " bufferNormalizedCoordY " << bufferNormalizedCoordY);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.cend())
            return addErrorEntry("DcsmContentControl: failed to handle pick event, content unknown.");

        if (contentIt->second.contentType == ETechnicalContentType::WaylandIviSurfaceID)
            return addErrorEntry("DcsmContentControl: Pick event handling cannot be used with content of type WaylandIviSurfaceID");

        const auto sceneId = findSceneAssociatedWithContent(contentID);
        if (!sceneId.isValid())
            return addErrorEntry("DcsmContentControl: failed to handle pick event, content's scene unknown. Make content ready at least once before picking");

        return m_sceneControl.handlePickEvent(sceneId, bufferNormalizedCoordX, bufferNormalizedCoordY);
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
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM event CONTENT OFFERED for content " << contentID << " category " << category << " content type " << int(contentType));
        auto it = m_categories.find(category);
        if (it != m_categories.end())
        {
            const auto contentIt = m_contents.find(contentID);
            if (contentIt != m_contents.cend() && contentIt->second.category != category)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content offered " << contentID << " is already assigned to another category " << contentIt->second.category
                    << ", now requesting category " << category << ". Ignoring, stop offer first before changing category.");
                return;
            }

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: assigning content " << contentID << " to category " << category);
            auto& catInfo = it->second;
            catInfo.assignedContentIds.insert(contentID);
            m_contents.emplace(std::make_pair(contentID, ContentInfo{ category, contentType, false, false, 0 }));
            CategoryInfoUpdate update;
            update.impl.setCategoryInfo(catInfo.categoryInfo);
            m_dcsmConsumer.assignContentToConsumer(contentID, update);

            handleContentStateChange(contentID, ContentState::Invalid); // using invalid last state to force event emit
        }
        else
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: not interested in content offer " << contentID << " for category " << category);
            m_offeredContentsForOtherCategories.push_back({category, contentID, contentType});
        }
    }

    void DcsmContentControlImpl::contentDescription(ContentID contentID, TechnicalContentDescriptor contentDescriptor)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM content description for content " << contentID << " content descriptor " << contentDescriptor);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not assigned, ignoring content description.");
        }
        else
        {
            assert(m_categories.count(contentIt->second.category) > 0);
            switch (contentIt->second.contentType)
            {
            case ETechnicalContentType::RamsesSceneID:
                m_scenes[sceneId_t{ contentDescriptor.getValue() }].associatedContents.insert(contentID);
                // already ready requested and just waiting for sceneid?
                if (contentIt->second.readyRequested)
                {
                    requestSceneState(contentID, RendererSceneState::Ready);
                }
                else
                {
                    requestSceneState(contentID, RendererSceneState::Available);
                }
                break;
            case ETechnicalContentType::WaylandIviSurfaceID:
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " has content type WaylandIviSurfaceID, this content type can currently not controlled with DcsmContentControl");
                break;
            }
        }
    }

    void DcsmContentControlImpl::contentReady(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM event CONTENT READY for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not assigned, ignoring state change.");
        }
        else
        {
            assert(m_categories.count(contentIt->second.category) > 0);
            if (contentIt->second.readyRequested)
            {
                const ContentState lastState = getCurrentState(contentID);
                contentIt->second.dcsmReady = true;
                handleContentStateChange(contentID, lastState);
            }
            else
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not requested to be ready or released after request, ignoring state change.");
        }
    }

    void DcsmContentControlImpl::contentStopOfferRequest(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM event CONTENT STOP OFFER REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            // no need to confirm, because not assigned
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not assigned, ignoring stopofferrequest.");
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
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM event FORCE CONTENT OFFER STOPPED for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not assigned, nothing to do.");
            auto itEnd = std::remove_if(m_offeredContentsForOtherCategories.begin(), m_offeredContentsForOtherCategories.end(), [&](const OfferedContents& offer)
                {
                    return (offer.contentID == contentID);
                });
            m_offeredContentsForOtherCategories.erase(itEnd, m_offeredContentsForOtherCategories.end());
        }
        else
        {
            removeContent(contentID);
            for (const auto& sceneIt : m_scenes)
                if (sceneIt.second.associatedContents.count(contentID) > 0)
                    if (m_sceneControl.setSceneState(sceneIt.first, RendererSceneState::Available) != StatusOK)
                        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: failed to set scene state " << RendererSceneState::Available << " to scene " << sceneIt.first << " content " << contentID);
            m_pendingEvents.push_back({ EventType::ContentNotAvailable, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK });
        }
    }

    void DcsmContentControlImpl::contentMetadataUpdated(ContentID contentID, const DcsmMetadataUpdate& metadataUpdate)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM event UPDATE CONTENT METADATA for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not assigned, nothing to do.");
        }
        else
            m_pendingEvents.push_back({ EventType::ContentMetadataUpdate, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK, metadataUpdate.impl.getMetadata() });
    }

    void DcsmContentControlImpl::sceneStateChanged(sceneId_t sceneId, RendererSceneState state)
    {
        auto& sceneInfo = m_scenes[sceneId];
        if (sceneInfo.sharedState.getActualState() < RendererSceneState::Ready && state == RendererSceneState::Ready)
        {
            if (std::none_of(sceneInfo.associatedContents.cbegin(), sceneInfo.associatedContents.cend(), [&](ContentID contentID) { return m_contents.find(contentID)->second.readyRequested; }))
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: scene " << sceneId << " changed state from "
                    << EnumToString(sceneInfo.sharedState.getActualState())
                    << " to " << EnumToString(state)
                    << " but no associated content requested this change - ignoring. This is OK if ready request canceled before finishing");
                // even though not assigned to content, keep its actual state up-to-date if it gets assigned later
                sceneInfo.sharedState.setActualState(state);
                return;
            }
        }

        // first collect current state of all associated contents to be used as 'last' state when checking if changed
        std::unordered_map<ContentID, ContentState> lastStates;
        for (const auto contentID : sceneInfo.associatedContents)
            lastStates[contentID] = getCurrentState(contentID);

        // set new actual scene state which might modify consolidated state of some of the associated contents
        sceneInfo.sharedState.setActualState(state);

        // emit event if state changed for any of the associated contents
        for (const auto contentID : sceneInfo.associatedContents)
            handleContentStateChange(contentID, lastStates[contentID]);
    }

    void DcsmContentControlImpl::offscreenBufferLinked(displayBufferId_t offscreenBufferId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success)
    {
        LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received offscreen buffer {} linked event to scene consumer {} consumerId {}, success={}.",
            offscreenBufferId, consumerScene, consumerId, success);

        auto contents = findContentsAssociatingScene(consumerScene);
        if (contents.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received offscreen buffer linked event but cannot find corresponding consumer content for scene {}"
                " this can happen if content or its scene became unavailable after offscreen buffer linked processed."
                " Event will still be emitted but with content ID set to invalid value.", consumerScene);
            contents.push_back(ContentID::Invalid());
        }
        else if (contents.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl: received offscreen buffer linked event for scene " << consumerScene << " which is associated with multiple contents:";
                for (const auto c : contents)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        const auto consumerContent = contents.front();
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: offscreen buffer linked - offscreen buffer " << offscreenBufferId << " to consumer content " << consumerContent << " dataId " << consumerId);
        Event evt{ EventType::OffscreenBufferLinked };
        evt.displayBuffer = offscreenBufferId;
        evt.consumerContentID = consumerContent;
        evt.consumerID = consumerId;
        evt.result = (success ? DcsmContentControlEventResult::OK : DcsmContentControlEventResult::TimedOut);
        m_pendingEvents.push_back(evt);
    }

    void DcsmContentControlImpl::dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success)
    {
        auto contentsProvider = findContentsAssociatingScene(providerScene);
        if (contentsProvider.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received data linked event but cannot find corresponding provider content for provider scene {},"
                " this can happen if content or its scene became unavailable after data linked processed. Event will still be emitted but with content ID set to invalid value.", providerScene);
            contentsProvider.push_back(ContentID::Invalid());
        }
        else if (contentsProvider.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl: received data linked event for provider scene " << providerScene << " which is associated with multiple contents:";
                for (const auto c : contentsProvider)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        auto contentsConsumer = findContentsAssociatingScene(consumerScene);
        if (contentsConsumer.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received data linked event but cannot find corresponding consumer content for consumer scene {},"
                " this can happen if content or its scene became unavailable after data linked processed. Event will still be emitted but with content ID set to invalid value.", consumerScene);
            contentsConsumer.push_back(ContentID::Invalid());
        }
        else if (contentsConsumer.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl: received data linked event for consumer scene " << consumerScene << " which is associated with multiple contents:";
                for (const auto c : contentsConsumer)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        const ContentID providerContent = contentsProvider.front();
        const ContentID consumerContent = contentsConsumer.front();

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: data linked - provider content " << providerContent << " dataId " << providerId
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
        auto contentsConsumer = findContentsAssociatingScene(consumerScene);
        if (contentsConsumer.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received data unlinked event but cannot find corresponding consumer content for consumer scene {},"
                " this can happen if content or its scene became unavailable after data unlinked processed. Event will still be emitted but with content ID set to invalid value.", consumerScene);
            contentsConsumer.push_back(ContentID::Invalid());
        }
        else if (contentsConsumer.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl: received data unlinked event for consumer scene " << consumerScene << " which is associated with multiple contents:";
                for (const auto c : contentsConsumer)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }

        const ContentID consumerContent = contentsConsumer.front();

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: data unlinked - consumer content " << consumerContent << " dataId " << consumerId);
        Event evt{ EventType::DataUnlinked };
        evt.consumerContentID = consumerContent;
        evt.consumerID = consumerId;
        evt.result = (success ? DcsmContentControlEventResult::OK : DcsmContentControlEventResult::TimedOut);
        m_pendingEvents.push_back(evt);
    }

    void DcsmContentControlImpl::objectsPicked(sceneId_t scene, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount)
    {
        auto contents = findContentsAssociatingScene(scene);
        if (contents.empty())
        {
            LOG_WARN_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received objects picked event but cannot find corresponding content for scene {},"
                " this can happen if content or its scene became unavailable after objects picked processed. Event will still be emitted but with content ID set to invalid value.", scene);
            contents.push_back(ContentID::Invalid());
        }
        else if (contents.size() > 1)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DcsmContentControl: received " << pickedObjectsCount << " objects picked event for scene " << scene << " which is associated with multiple contents:";
                for (const auto c : contents)
                    sos << "  " << c;
                sos << ". Event will be emitted for the first one only.";
            });
        }
        else
            LOG_INFO_P(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: {} objects picked, scene {}, content {}", pickedObjectsCount, scene, contents.front());

        Event evt{ EventType::ObjectsPicked };
        evt.contentID = contents.front();
        evt.pickedObjectIds = std::vector<pickableObjectId_t>(pickedObjects, pickedObjects + pickedObjectsCount);
        m_pendingEvents.push_back(std::move(evt));
    }

    void DcsmContentControlImpl::dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId)
    {
        auto contents = findContentsAssociatingScene(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received data provider created event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: data provider " << dataProviderId << " created in content " << content);
            Event evt{ EventType::DataProviderCreated };
            evt.providerContentID = content;
            evt.providerID = dataProviderId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId)
    {
        auto contents = findContentsAssociatingScene(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received data provider destroyed event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: data provider " << dataProviderId << " destroyed in content " << content);
            Event evt{ EventType::DataProviderDestroyed };
            evt.providerContentID = content;
            evt.providerID = dataProviderId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId)
    {
        auto contents = findContentsAssociatingScene(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received data consumer created event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: data consumer " << dataConsumerId << " created in content " << content);
            Event evt{ EventType::DataConsumerCreated };
            evt.consumerContentID = content;
            evt.consumerID = dataConsumerId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId)
    {
        auto contents = findContentsAssociatingScene(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received data consumer destroyed event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: data consumer " << dataConsumerId << " destroyed in content " << content);
            Event evt{ EventType::DataConsumerDestroyed };
            evt.consumerContentID = content;
            evt.consumerID = dataConsumerId;
            m_pendingEvents.push_back(std::move(evt));
        }
    }

    void DcsmContentControlImpl::sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag)
    {
        auto contents = findContentsAssociatingScene(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received scene flushed event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << content << " flushed with version " << sceneVersionTag);
            Event evt{ EventType::ContentFlushed };
            evt.contentID = content;
            evt.version = sceneVersionTag;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::sceneExpirationMonitoringEnabled(sceneId_t sceneId)
    {
        auto contents = findContentsAssociatingScene(sceneId);
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
        auto contents = findContentsAssociatingScene(sceneId);
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
        auto contents = findContentsAssociatingScene(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received scene expired event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << content << " expired");
            Event evt{ EventType::ContentExpired };
            evt.contentID = content;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::sceneRecoveredFromExpiration(sceneId_t sceneId)
    {
        auto contents = findContentsAssociatingScene(sceneId);
        if (contents.empty())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received scene recovered from expiration event but cannot find corresponding content for scene " << sceneId
                << " this can happen if content or its scene became unavailable after event processed.");
        }

        for (const auto content : contents)
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << content << " recovered from expiration");
            Event evt{ EventType::ContentRecoveredFromExpiration };
            evt.contentID = content;
            m_pendingEvents.push_back(evt);
        }
    }

    void DcsmContentControlImpl::streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: stream " << streamId << " availability changed to " << available);
        Event evt{ EventType::StreamAvailable };
        evt.streamSource = streamId;
        evt.streamAvailable = available;
        m_pendingEvents.push_back(evt);
    }

    void DcsmContentControlImpl::executePendingCommands()
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
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: executing scheduled command for content " << cmd.contentId << " to request scene state change to " << EnumToString(cmd.sceneState));
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
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: executing scheduled command to remove content " << contentId);
            removeContent(contentId);
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
            }
        }
    }

    void DcsmContentControlImpl::requestSceneState(ContentID contentID, RendererSceneState state)
    {
        const auto sceneId = findSceneAssociatedWithContent(contentID);
        if (sceneId.isValid())
        {
            auto& sharedState = m_scenes[sceneId].sharedState;
            sharedState.setDesiredState(contentID, state);

            const auto actualState = sharedState.getActualState();
            const auto consolidatedDesiredState = sharedState.getConsolidatedDesiredState();
            if (consolidatedDesiredState != actualState)
            {
                if (consolidatedDesiredState >= RendererSceneState::Ready && actualState < RendererSceneState::Ready)
                {
                    assert(m_contents.count(contentID) > 0);
                    const ContentInfo& contentInfo = m_contents.find(contentID)->second;
                    const auto categoryId = contentInfo.category;
                    assert(m_categories.count(categoryId) > 0);
                    const auto displayId = m_categories.find(categoryId)->second.display;
                    if (m_sceneControl.setSceneMapping(sceneId, displayId) != StatusOK)
                        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: failed to set scene mapping for scene " << sceneId << " content " << contentID);
                }

                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: requesting scene state change for content " << contentID << " scene " << sceneId << " state change to " << EnumToString(consolidatedDesiredState));
                if (m_sceneControl.setSceneState(sceneId, consolidatedDesiredState) != StatusOK)
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: failed to request state for scene " << sceneId << " to " << EnumToString(consolidatedDesiredState));
            }
        }
    }

    void DcsmContentControlImpl::handleContentStateChange(ContentID contentID, ContentState lastState)
    {
        const ContentState currState = getCurrentState(contentID);
        if (currState != lastState)
        {
            assert(m_contents.count(contentID) > 0);
            auto& contentInfo = m_contents.find(contentID)->second;
            if (currState > ContentState::Available)
                contentInfo.readyRequested = false;

            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " state changed from " << ContentStateName(lastState) << " to " << ContentStateName(currState));
            m_pendingEvents.push_back({ EventType::ContentStateChanged, contentID, contentInfo.category, currState, lastState, DcsmContentControlEventResult::OK });
        }
    }

    void DcsmContentControlImpl::removeContent(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: removing content " << contentID << " and any pending commands associated with it");
        m_contents.erase(contentID);
        for (auto& catIt : m_categories)
            catIt.second.assignedContentIds.erase(contentID);
        for (auto& sceneIt : m_scenes)
            sceneIt.second.associatedContents.erase(contentID);
        const auto it = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [contentID](const Command& cmd)
        {
            if (cmd.contentId == contentID)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: there is a scheduled change for content " << contentID << " being removed, removing the scheduled change "
                    << int(cmd.type) << " scene state change " << EnumToString(cmd.sceneState) << " at TS " << cmd.timePoint);
                return true;
            }
            return false;
        });
        m_pendingCommands.erase(it, m_pendingCommands.end());
    }

    void DcsmContentControlImpl::scheduleSceneStateChange(ContentID contentThatTrigerredChange, RendererSceneState sceneState, uint64_t ts)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: scheduling content's scene state change for content " << contentThatTrigerredChange
            << " to scene state " << EnumToString(sceneState) << " at TS " << ts);

        // Remove any other state change scheduled for this content
        auto itEnd = std::remove_if(m_pendingCommands.begin(), m_pendingCommands.end(), [contentThatTrigerredChange](const Command& cmd)
        {
            if (cmd.type == CommandType::SceneStateChange && cmd.contentId == contentThatTrigerredChange)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: there is already a scheduled change for this content's scene, overriding previous change to scene state "
                    << EnumToString(cmd.sceneState) << " at TS " << cmd.timePoint);
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

    ContentState DcsmContentControlImpl::getCurrentState(ContentID contentID) const
    {
        assert(m_contents.count(contentID) > 0);
        const auto sceneId = findSceneAssociatedWithContent(contentID);

        // if scene is available for content, check the shared scene state to determine content compound state
        if (sceneId.isValid())
        {
            const bool isDcsmReady = m_contents.find(contentID)->second.dcsmReady;
            const auto sceneState = m_scenes.find(sceneId)->second.sharedState.getCurrentStateForContent(contentID);
            switch (sceneState)
            {
            case RendererSceneState::Ready:
                return (isDcsmReady ? ContentState::Ready : ContentState::Available);
            case RendererSceneState::Rendered:
                return ContentState::Shown;
            case RendererSceneState::Available:
            case RendererSceneState::Unavailable: // even if scene is unavailable the content is known and available to request ready
                return ContentState::Available;
            }
        }

        return ContentState::Available;
    }

    void DcsmContentControlImpl::processTimedOutRequests()
    {
        for (const auto& contentIt : m_contents)
        {
            const auto contentID = contentIt.first;
            const auto& contentInfo = contentIt.second;
            if (contentInfo.readyRequested && m_timeStampNow > contentInfo.readyRequestTimeOut)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: ready request for content " << contentID << " timed out (timeoutTS=" << contentInfo.readyRequestTimeOut
                    << " nowTS=" << m_timeStampNow << "), releasing content. Request ready again for re-try.");
                m_pendingEvents.push_back({ EventType::ContentStateChanged, contentID, Category{0}, ContentState::Ready, ContentState::Available, DcsmContentControlEventResult::TimedOut });
                releaseContent(contentID, { 0, 0 });
            }
        }
    }

    sceneId_t DcsmContentControlImpl::findSceneAssociatedWithContent(ContentID contentID) const
    {
        const auto sceneIt = std::find_if(m_scenes.cbegin(), m_scenes.cend(), [contentID](const auto& s)
        {
            return s.second.associatedContents.count(contentID) != 0;
        });

        return (sceneIt == m_scenes.cend() ? sceneId_t::Invalid() : sceneIt->first);
    }

    std::vector<ContentID> DcsmContentControlImpl::findContentsAssociatingScene(sceneId_t sceneId) const
    {
        std::vector<ContentID> contents;
        const auto it = m_scenes.find(sceneId);
        if (it != m_scenes.cend())
            contents.assign(it->second.associatedContents.cbegin(), it->second.associatedContents.cend());

        return contents;
    }

    void DcsmContentControlImpl::contentEnableFocusRequest(ramses::ContentID contentID, int32_t focusRequest)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM event CONTENT ENABLE FOCUS REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not assigned, ignoring request.");
        }
        else
            m_pendingEvents.push_back({EventType::ContentEnableFocusRequest, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK, {}, {}, {}, {}, {}, {}, {}, {}, {},  focusRequest});
    }

    void DcsmContentControlImpl::contentDisableFocusRequest(ramses::ContentID contentID, int32_t focusRequest)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: received DCSM event CONTENT DISABLE FOCUS REQUEST for content " << contentID);
        auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DcsmContentControl: content " << contentID << " not assigned, ignoring request.");
        }
        else
            m_pendingEvents.push_back({EventType::ContentDisableFocusRequest, contentID, Category{0}, {}, {}, DcsmContentControlEventResult::OK, {}, {}, {}, {}, {}, {}, {}, {}, {},  focusRequest});
    }

}
