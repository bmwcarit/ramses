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

    status_t DcsmProviderImpl::registerRamsesContent(ContentID contentID, Category category, sceneId_t scene)
    {
        const auto contentIt = m_contents.find(contentID);
        if (contentIt != m_contents.end() && contentIt->second.status != EDcsmStatus::Unregistered)
            return addErrorEntry("DcsmProvider::registerRamsesContent failed, ContentID is already registered.");

        m_contents[contentID] = { scene, category };
        if (!m_dcsm.sendRegisterContent(ramses_internal::ContentID(contentID.getValue()), ramses_internal::Category(category.getValue())))
            return addErrorEntry("DcsmProvider::registerRamsesContent failed, failure to send sendRegisterContent message.");

        return StatusOK;
    }

    status_t DcsmProviderImpl::unregisterRamsesContent(ContentID contentID)
    {
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::unregisterRamsesContent failed, ContentID is unknown");

        if  (!m_dcsm.sendRequestUnregisterContent(ramses_internal::ContentID(contentID.getValue())))
            return addErrorEntry("DcsmProvider::unregisterRamsesContent failed, failure to send unregisterRamsesContent message.");

        auto& content = contentIt->second;
        if (content.destId.isInvalid())
        {
            m_handler->contentUnregistered(contentID, AnimationInformation());
            content.status = EDcsmStatus::Unregistered;
        }

        return StatusOK;
    }

    status_t DcsmProviderImpl::markContentReady(ContentID contentID)
    {
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::markContentReady failed, ContentID is unknown");

        auto& content = contentIt->second;
        if (!content.ready)
            content.ready = true;

        if (!content.destId.isInvalid() && content.contentRequested)
        {
            if (!m_dcsm.sendContentAvailable(content.destId, ramses_internal::ContentID(contentID.getValue()),
                                             ramses_internal::ETechnicalContentType::RamsesSceneID,
                                             ramses_internal::TechnicalContentDescriptor(content.scene)))
                return addErrorEntry("DcsmProvider::markContentReady failed, failure to send sendContentAvailable message.");
        }

        return StatusOK;
    }

    status_t DcsmProviderImpl::requestContentShown(ContentID contentID)
    {
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
            return addErrorEntry("DcsmProvider::requestContentShown failed, ContentID is unknown");

        const auto& content = contentIt->second;
        if (content.destId.isInvalid())
            return addErrorEntry("DcsmProvider::requestContentShown failed, there is no Consumer subscribed to this content");

        if (!m_dcsm.sendCategoryContentSwitchRequest(content.destId, ramses_internal::ContentID(contentID.getValue())))
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

    void DcsmProviderImpl::canvasSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation anim, const ramses_internal::Guid& consumer)
    {
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_WARN(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::canvasSizeChange: received for unknown contentID " << contentID.getValue() << ", ignoring.");
            return;
        }

        auto& content = contentIt->second;
        if (content.destId.isInvalid())
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::canvasSizeChange: received for contentID " << contentID.getValue() << " from new consumer " << consumer);
            content.destId = consumer;
        }
        else if (content.destId != consumer)
        {
            LOG_WARN(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::canvasSizeChange: received for contentID " << contentID.getValue() << " from wrong consumer " << consumer << ", ignoring.");
            return;
        }

        m_handler->canvasSizeChanged(contentID, sizeInfo, anim);
    }

    void DcsmProviderImpl::contentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation anim, const ramses_internal::Guid& consumer)
    {
        const auto contentIt = m_contents.find(contentID);
        if (contentIt == m_contents.end())
        {
            LOG_WARN(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::contentStatusChange: received for unknown contentID " << contentID.getValue() << ", ignoring.");
            return;
        }

        auto& content = contentIt->second;
        if (content.destId.isInvalid())
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::contentStatusChange: received for contentID " << contentID.getValue() << " from new consumer " << consumer);
            content.destId = consumer;
        }
        else if (!(content.destId == consumer))
        {
            LOG_WARN(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::contentStatusChange: received for contentID " << contentID.getValue() << " from wrong consumer " << consumer << ", ignoring.");
            return;
        }

        if (status == content.status)
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::contentStatusChange: received for contentID " << contentID.getValue() << " ignored because no status change");
            return;
        }

        if (status != EDcsmStatus::Shown && content.status == EDcsmStatus::Shown)
        {
            if (status == EDcsmStatus::Unregistered)
                m_handler->contentUnregistered(contentID, anim);
            else
                m_handler->contentHidden(contentID, anim);
        }

        if (status == EDcsmStatus::Shown && content.status != EDcsmStatus::Shown)
            m_handler->contentShown(contentID, anim);

        if (status == EDcsmStatus::Ready)
        {
            content.contentRequested = true;
            if (!content.ready)
                m_handler->contentReadyRequest(contentID);
            else if (content.status != EDcsmStatus::Shown)
            {
                if (!m_dcsm.sendContentAvailable(content.destId, ramses_internal::ContentID(contentID.getValue()),
                                                 ramses_internal::ETechnicalContentType::RamsesSceneID,
                                                 ramses_internal::TechnicalContentDescriptor(content.scene)))
                    LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::contentStatusChange: failed, failure to send sendContentAvailable message.");
            }

        }

        if (status == EDcsmStatus::Registered || status == EDcsmStatus::Unregistered)
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "DcsmProvider::contentStatusChange: received for contentID " << contentID.getValue() << " from consumer " << consumer << " with Registered/Unregistered. Resetting consumer.");
            content.destId = ramses_internal::Guid(false);
        }

        content.status = status;
    }
}
