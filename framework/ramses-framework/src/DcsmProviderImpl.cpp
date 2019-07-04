//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmProviderImpl.h"

#include "ramses-framework-api/IDcsmProviderEventHandler.h"

#include "Utils/LogMacros.h"

namespace ramses
{
    DcsmProviderImpl::DcsmProviderImpl(ramses_internal::IDcsmComponent& dcsm)
        : StatusObjectImpl()
        , m_dcsm(dcsm)
    {
        m_dcsm.setLocalProviderAvailability(true);
    }

    DcsmProviderImpl::~DcsmProviderImpl()
    {
        m_dcsm.setLocalProviderAvailability(false);
    }

    status_t DcsmProviderImpl::offerContent(ContentID contentID, Category category, sceneId_t scene)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::requestStopOfferContent: contentID " << contentID.getValue()
            << ", category " << category.getValue() << ", scene " << scene);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt != m_contents.end() && contentIt->second.status != ramses_internal::EDcsmState::AcceptStopOffer)
            return addErrorEntry("DcsmProvider::registerRamsesContent failed, ContentID is already registered.");

        m_contents[contentID] = { scene, category };
        if (!m_dcsm.sendOfferContent(ramses_internal::ContentID(contentID.getValue()), ramses_internal::Category(category.getValue())))
            return addErrorEntry("DcsmProvider::registerRamsesContent failed, failure to send sendRegisterContent message.");

        return StatusOK;
    }

    status_t DcsmProviderImpl::requestStopOfferContent(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::requestStopOfferContent: contentID " << contentID.getValue());

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::unregisterRamsesContent failed, ContentID is unknown");

        if  (!m_dcsm.sendRequestStopOfferContent(ramses_internal::ContentID(contentID.getValue())))
            return addErrorEntry("DcsmProvider::unregisterRamsesContent failed, failure to send unregisterRamsesContent message.");

        return StatusOK;
    }

    status_t DcsmProviderImpl::markContentReady(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::markContentReady: contentID " << contentID.getValue());

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::markContentReady failed, ContentID is unknown");

        auto& content = contentIt->second;
        if (!content.ready)
            content.ready = true;

        if (content.contentRequested)
        {
            if (!m_dcsm.sendContentReady(ramses_internal::ContentID(contentID.getValue()),
                                         ramses_internal::ETechnicalContentType::RamsesSceneID,
                                         ramses_internal::TechnicalContentDescriptor(content.scene)))
                return addErrorEntry("DcsmProvider::markContentReady failed, failure to send sendContentAvailable message.");
        }

        return StatusOK;
    }

    status_t DcsmProviderImpl::requestContentFocus(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::requestContentFocus: contentID " << contentID.getValue());

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::requestContentShown failed, ContentID is unknown");

        if (!m_dcsm.sendContentFocusRequest(ramses_internal::ContentID(contentID.getValue())))
            return addErrorEntry("DcsmProvider::requestContentShown failed, failure to send sendCategoryContentSwitchRequest message.");

        return StatusOK;
    }

    status_t DcsmProviderImpl::dispatchEvents(ramses::IDcsmProviderEventHandler& handler)
    {
        m_handler = &handler;
        if (!m_dcsm.dispatchProviderEvents(*this))
            return addErrorEntry("DcsmProvider::dispatchEvents failed, failure to call dispatchEvents on component.");

        // we might want to reset the handler again here, but it makes testing way more difficult and I don't think it is necessary
        return StatusOK;
    }

    void DcsmProviderImpl::contentSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation anim)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentSizeChange: contentID " << contentID.getValue() << ", SizeInfo "
            << sizeInfo.width << "x" << sizeInfo.height << ", ai[" << anim.startTime << ", " << anim.finishTime << "]");

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_WARN(ramses_internal::CONTEXT_DCSM, "DcsmProvider::canvasSizeChange: received for unknown contentID " << contentID.getValue() << ", ignoring.");
            return;
        }

        m_handler->contentSizeChange(contentID, sizeInfo, anim);
    }

    void DcsmProviderImpl::contentStateChange(ContentID contentID, ramses_internal::EDcsmState status, SizeInfo sizeInfo, AnimationInformation anim)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: contentID " << contentID.getValue()
            << ", status " << EnumToString(status) << ", SizeInfo " << sizeInfo.width << "x" << sizeInfo.height
            << ", ai[" << anim.startTime << ", " << anim.finishTime << "]");

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_ERROR(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: received for unknown contentID " << contentID.getValue() << ", ignoring.");
            return;
        }

        auto& content = contentIt->second;
        if (status == content.status)
        {
            LOG_WARN(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: received for contentID " << contentID.getValue() << " ignored because no status change");
            return;
        }

        switch (status)
        {
        case ramses_internal::EDcsmState::AcceptStopOffer:
            m_handler->stopOfferAccepted(contentID, anim);
            m_contents.erase(contentIt);
            return;

        case ramses_internal::EDcsmState::Assigned:
            if (content.status == ramses_internal::EDcsmState::Offered)
            {
                m_handler->contentSizeChange(contentID, sizeInfo, anim);
                break;
            }
            // else fall through to Offered
        case ramses_internal::EDcsmState::Offered:
            content.ready = false;
            content.contentRequested = false;
            m_handler->contentRelease(contentID, anim);
            break;
        case ramses_internal::EDcsmState::Ready:
            content.contentRequested = true;

            if (content.status == ramses_internal::EDcsmState::Shown)
                m_handler->contentHide(contentID, anim);
            else if (!content.ready)
                m_handler->contentReadyRequested(contentID);
            else
            {
                if (!m_dcsm.sendContentReady(ramses_internal::ContentID(contentID.getValue()),
                    ramses_internal::ETechnicalContentType::RamsesSceneID,
                    ramses_internal::TechnicalContentDescriptor(content.scene)))
                    LOG_ERROR(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: failed, failure to send sendContentAvailable message.");
            }
            break;

        case ramses_internal::EDcsmState::Shown:
            m_handler->contentShow(contentID, anim);
            break;

        default:
            LOG_ERROR(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange failed with invalid EDcsmState");
        }

        content.status = status;
    }
}
