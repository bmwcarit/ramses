//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmProviderImpl.h"
#include "DcsmMetadataCreatorImpl.h"

#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/DcsmMetadataCreator.h"

#include "Utils/LogMacros.h"
#include "RamsesFrameworkTypesImpl.h"
#include "PlatformAbstraction/Macros.h"

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

    status_t DcsmProviderImpl::offerContent(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::offerContent: contentID " << contentID
            << ", category " << category << ", scene " << scene << ", mode " << static_cast<int>(mode));
        return commonOfferContent("offerContent", contentID, category, scene, mode);
    }

    status_t DcsmProviderImpl::offerContentWithMetadata(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode, const DcsmMetadataCreator& metadata)
    {
        const ramses_internal::DcsmMetadata riMetadata = metadata.impl.getMetadata();
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::offerContentWithMetadata: contentID " << contentID
                 << ", category " << category << ", scene " << scene << ", mode " << static_cast<int>(mode) << ", metadata " << riMetadata);

        auto res = commonOfferContent("offerContentWithMetadata", contentID, category, scene, mode);
        if (res != StatusOK)
            return res;

        if (!m_dcsm.sendUpdateContentMetadata(ramses_internal::ContentID(contentID.getValue()), std::move(riMetadata)))
        {
            m_dcsm.sendRequestStopOfferContent(ramses_internal::ContentID(contentID.getValue()));
            return addErrorEntry("DcsmProvider::offerContentWithMetadata failed, failure to send sendUpdateContentMetadata message.");
        }

        return StatusOK;
    }

    status_t DcsmProviderImpl::commonOfferContent(const char* callerMethod, ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode)
    {
        if (!category.isValid())
            return addErrorEntry((ramses_internal::StringOutputStream() << "DcsmProvider::" << callerMethod << " failed, category is invalid.").c_str());

        const auto contentIt = m_contents.find(contentID);
        if (contentIt != m_contents.end() && contentIt->second.status != ramses_internal::EDcsmState::AcceptStopOffer)
            return addErrorEntry((ramses_internal::StringOutputStream() << "DcsmProvider::" << callerMethod << " failed, ContentID is already registered.").c_str());

        m_contents[contentID] = { scene, category };
        if (!m_dcsm.sendOfferContent(ramses_internal::ContentID(contentID.getValue()), ramses_internal::Category(category.getValue()), mode == EDcsmOfferingMode::LocalOnly))
            return addErrorEntry((ramses_internal::StringOutputStream() << "DcsmProvider::" << callerMethod << " failed, failure to send sendOfferContent message.").c_str());
        else if (!m_dcsm.sendContentDescription(ramses_internal::ContentID(contentID.getValue()),
                                                ramses_internal::ETechnicalContentType::RamsesSceneID, // SceneID is the TechnicalContentDescriptor when using type RamsesSceneID
                                                ramses_internal::TechnicalContentDescriptor(scene.getValue())))
        {
            m_dcsm.sendRequestStopOfferContent(ramses_internal::ContentID(contentID.getValue()));
            return addErrorEntry((ramses_internal::StringOutputStream() << "DcsmProvider::" << callerMethod << " failed, failure to send sendContentDescription message.").c_str());
        }

        return StatusOK;
    }

    status_t DcsmProviderImpl::updateContentMetadata(ContentID contentID, const DcsmMetadataCreator& metadata)
    {
        const ramses_internal::DcsmMetadata riMetadata = metadata.impl.getMetadata();
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::updateContentMetadata: contentID " << contentID << ", metadata " << riMetadata);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::updateContentMetadata failed, ContentID is unknown");

        if (!m_dcsm.sendUpdateContentMetadata(ramses_internal::ContentID(contentID.getValue()), riMetadata))
            return addErrorEntry("DcsmProvider::updateContentMetadata failed, failure to send sendUpdateContentMetadata message.");

        return StatusOK;
    }

    status_t DcsmProviderImpl::requestStopOfferContent(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::requestStopOfferContent: contentID " << contentID);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::requestStopOfferContent failed, ContentID is unknown");

        if  (!m_dcsm.sendRequestStopOfferContent(ramses_internal::ContentID(contentID.getValue())))
            return addErrorEntry("DcsmProvider::requestStopOfferContent failed, failure to send unregisterRamsesContent message.");

        return StatusOK;
    }

    status_t DcsmProviderImpl::markContentReady(ContentID contentID)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::markContentReady: contentID " << contentID);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::markContentReady failed, ContentID is unknown");

        auto& content = contentIt->second;
        if (!content.ready)
            content.ready = true;

        if (content.contentRequested)
        {
            if (!m_dcsm.sendContentReady(ramses_internal::ContentID(contentID.getValue())))
                return addErrorEntry("DcsmProvider::markContentReady failed, failure to send sendContentAvailable message.");
        }

        return StatusOK;
    }

    ramses::status_t DcsmProviderImpl::enableFocusRequest(ContentID contentID, int32_t focusRequest)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::enableFocusRequest: contentID " << contentID << " request:" << focusRequest);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::enableFocusRequest failed, ContentID is unknown");

        if (!m_dcsm.sendContentEnableFocusRequest(ramses_internal::ContentID(contentID.getValue()), focusRequest))
            return addErrorEntry("DcsmProvider::enableFocusRequest failed, failure to send enableFocusRequest message.");

        return StatusOK;
    }

    ramses::status_t DcsmProviderImpl::disableFocusRequest(ContentID contentID, int32_t focusRequest)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::disableFocusRequest: contentID " << contentID << " request:" << focusRequest);

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::disableFocusRequest failed, ContentID is unknown");

        if (!m_dcsm.sendContentDisableFocusRequest(ramses_internal::ContentID(contentID.getValue()), focusRequest))
            return addErrorEntry("DcsmProvider::disableFocusRequest failed, failure to send disableFocusRequest message.");

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

    void DcsmProviderImpl::contentSizeChange(ContentID contentID, const CategoryInfoUpdate& categoryInfo, AnimationInformation anim)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentSizeChange: contentID " << contentID << ", "
            << categoryInfo << ", ai[" << anim.startTime << ", " << anim.finishTime << "]");

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_WARN(ramses_internal::CONTEXT_DCSM, "DcsmProvider::canvasSizeChange: received for unknown contentID " << contentID << ", ignoring.");
            return;
        }

        m_handler->contentSizeChange(contentID, categoryInfo, anim);
    }

    void DcsmProviderImpl::contentStateChange(ContentID contentID, ramses_internal::EDcsmState state, const CategoryInfoUpdate& categoryInfo, AnimationInformation anim)
    {
        LOG_INFO(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: contentID " << contentID
            << ", status " << EnumToString(state) << ", " << categoryInfo << ", ai[" << anim.startTime << ", " << anim.finishTime << "]");

        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_ERROR(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: received for unknown contentID " << contentID << ", ignoring.");
            return;
        }

        auto& content = contentIt->second;
        if (state == content.status)
        {
            LOG_WARN(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: received for contentID " << contentID << " ignored because no status change");
            return;
        }

        switch (state)
        {
        case ramses_internal::EDcsmState::AcceptStopOffer:
            m_handler->stopOfferAccepted(contentID, anim);
            m_contents.erase(contentIt);
            return;

        case ramses_internal::EDcsmState::Assigned:
            if (content.status == ramses_internal::EDcsmState::Offered)
            {
                m_handler->contentSizeChange(contentID, categoryInfo, anim);
                break;
            }
            RFALLTHROUGH;
        case ramses_internal::EDcsmState::Offered:
            content.ready = false;
            content.contentRequested = false;
            if (content.status != ramses_internal::EDcsmState::Assigned)
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
                if (!m_dcsm.sendContentReady(ramses_internal::ContentID(contentID.getValue())))
                    LOG_ERROR(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange: failed, failure to send sendContentAvailable message.");
            }
            break;

        case ramses_internal::EDcsmState::Shown:
            m_handler->contentShow(contentID, anim);
            break;

        default:
            LOG_ERROR(ramses_internal::CONTEXT_DCSM, "DcsmProvider::contentStateChange failed with invalid EDcsmState");
        }

        content.status = state;
    }
}
