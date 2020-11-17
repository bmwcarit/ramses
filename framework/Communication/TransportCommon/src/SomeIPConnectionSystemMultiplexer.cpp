//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SomeIPConnectionSystemMultiplexer.h"
#include "TransportCommon/RamsesConnectionSystem.h"
#include "TransportCommon/DcsmConnectionSystem.h"
#include "TransportCommon/SomeIPAdapter.h"

namespace ramses_internal
{
    SomeIPConnectionSystemMultiplexer::SomeIPConnectionSystemMultiplexer(UInt32 someipCommunicationUserID,
                                                                         const ParticipantIdentifier& participantIdentifier,
                                                                         PlatformLock& frameworkLock,
                                                                         std::unique_ptr<DcsmConnectionSystem> dcsmConnectionSystem,
                                                                         std::unique_ptr<RamsesConnectionSystem> ramsesConnectionSystem)
        : m_someipCommunicationUserID(someipCommunicationUserID)
        , m_participantIdentifier(participantIdentifier)
        , m_frameworkLock(frameworkLock)
        , m_dcsmConnectionSystem(std::move(dcsmConnectionSystem))
        , m_ramsesConnectionSystem(std::move(ramsesConnectionSystem))
    {
    }

    SomeIPConnectionSystemMultiplexer::~SomeIPConnectionSystemMultiplexer() = default;

    bool SomeIPConnectionSystemMultiplexer::connectServices()
    {
        PlatformGuard g(m_frameworkLock);

        LOG_INFO_F(CONTEXT_COMMUNICATION, [&](ramses_internal::StringOutputStream& sos) {
            String userName;
            SomeIPAdapter::GetCommunicationUserAsString(m_someipCommunicationUserID, userName);
            sos << "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::connectServices: opening connection with configuration "
                << "for communication user id " << m_someipCommunicationUserID << " (" << userName << ")"
                << ", participant " << m_participantIdentifier.getParticipantId() << " (" << m_participantIdentifier.getParticipantName() << ")"
                << ", ramses " << (m_ramsesConnectionSystem ? "enabled" : "disabled") << ", dcsm " << (m_dcsmConnectionSystem ? "enabled" : "disabled") << ", connected "
                << m_isConnected;
        });

        if (m_isConnected)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::connectServices: Already connected");
            return false;
        }

        if (!m_ramsesConnectionSystem && !m_dcsmConnectionSystem)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::connectServices: No connection system enabled for user");
            return false;
        }

        if (!doPrepareConnect())
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::connectServices: preparation failed");
            return false;
        }

        if (m_ramsesConnectionSystem && !m_ramsesConnectionSystem->connect())
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::connectServices: ramses connection system connect failed");
            return false;
        }
        if (m_dcsmConnectionSystem && !m_dcsmConnectionSystem->connect())
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::connectServices: dcsm connection system connect failed");
            if (m_ramsesConnectionSystem)
                m_ramsesConnectionSystem->disconnect();
            return false;
        }

        doFinalizeConnect();

        m_isConnected = true;

        return true;
    }

    bool SomeIPConnectionSystemMultiplexer::disconnectServices()
    {
        PlatformGuard g(m_frameworkLock);

        LOG_INFO(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::disconnectServices: connected " << m_isConnected);
        if (!m_isConnected)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::disconnectServices: Not connected");
            return false;
        }

        bool result = true;
        if (m_ramsesConnectionSystem && !m_ramsesConnectionSystem->disconnect())
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::disconnectServices: ramses disconnect failed, expect issues");
            result = false;
        }
        if (m_dcsmConnectionSystem && !m_dcsmConnectionSystem->disconnect())
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::disconnectServices: dcsm disconnect failed, expect issues");
            result = false;
        }

        doFinalizeDisconnect();

        m_isConnected = false;
        return result;
    }

    IConnectionStatusUpdateNotifier& SomeIPConnectionSystemMultiplexer::getRamsesConnectionStatusUpdateNotifier()
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->getConnectionStatusUpdateNotifier();

        // Must return a valid IConnectionStatusUpdateNotifier even when created without RamsesConnectionSystem
        // TODO(tobias) rework interface to avoid
        static std::recursive_mutex fakePlatformLock;
        static ConnectionStatusUpdateNotifier dummyNotifier("RamsesDummyNotifier", "", fakePlatformLock);
        return dummyNotifier;
    }

    IConnectionStatusUpdateNotifier& SomeIPConnectionSystemMultiplexer::getDcsmConnectionStatusUpdateNotifier()
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->getConnectionStatusUpdateNotifier();

        // Must return a valid IConnectionStatusUpdateNotifier even when created without DcsmConnectionSystem
        // TODO(tobias) rework interface to avoid
        static std::recursive_mutex fakePlatformLock;
        static ConnectionStatusUpdateNotifier dummyNotifier("DcsmDummyNotifier", "", fakePlatformLock);
        return dummyNotifier;
    }

    // log triggers
    void SomeIPConnectionSystemMultiplexer::logConnectionInfo()
    {
        if (m_ramsesConnectionSystem)
            m_ramsesConnectionSystem->logConnectionInfo();
        if (m_dcsmConnectionSystem)
            m_dcsmConnectionSystem->logConnectionInfo();
    }

    void SomeIPConnectionSystemMultiplexer::triggerLogMessageForPeriodicLog()
    {
        if (m_ramsesConnectionSystem)
            m_ramsesConnectionSystem->logPeriodicInfo();
        if (m_dcsmConnectionSystem)
            m_dcsmConnectionSystem->logPeriodicInfo();
    }


    // -----------------------------------------------------------
    // ------------------ RAMSES ---------------------------------
    // -----------------------------------------------------------
    bool SomeIPConnectionSystemMultiplexer::broadcastNewScenesAvailable(const SceneInfoVector& newScenes)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->broadcastNewScenesAvailable(newScenes);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::broadcastNewScenesAvailable: ramses not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->broadcastScenesBecameUnavailable(unavailableScenes);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::broadcastScenesBecameUnavailable: ramses not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->sendScenesAvailable(to, availableScenes);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendScenesAvailable: ramses not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendSubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->sendSubscribeScene(to, sceneId);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendSubscribeScene: ramses not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendUnsubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->sendUnsubscribeScene(to, sceneId);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendUnsubscribeScene: ramses not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendInitializeScene(const Guid& to, const SceneId& sceneId)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->sendInitializeScene(to, sceneId);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendInitializeScene: ramses not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->sendSceneUpdate(to, sceneId, serializer);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendSceneActionList: ramses not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data)
    {
        if (m_ramsesConnectionSystem)
            return m_ramsesConnectionSystem->sendRendererEvent(to, sceneId, data);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendRendererEvent: ramses not enabled");
        return false;
    }

    void SomeIPConnectionSystemMultiplexer::setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler)
    {
        if (m_ramsesConnectionSystem)
            m_ramsesConnectionSystem->setSceneProviderServiceHandler(handler);
    }

    void SomeIPConnectionSystemMultiplexer::setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler)
    {
        if (m_ramsesConnectionSystem)
            m_ramsesConnectionSystem->setSceneRendererServiceHandler(handler);
    }


    // -----------------------------------------------------------
    // ------------------ DCSM -----------------------------------
    // -----------------------------------------------------------

    bool SomeIPConnectionSystemMultiplexer::sendDcsmBroadcastOfferContent(ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendBroadcastOfferContent(content, category, technicalContentType, friendlyName, 0);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmBroadcastOfferContent: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmOfferContent(const Guid& to, ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendOfferContent(to, content, category, technicalContentType, friendlyName, 0);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmOfferContent: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmContentDescription(const Guid& to, ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendContentDescription(to, contentID, technicalContentDescriptor);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmContentDescription: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmContentReady(const Guid& to, ContentID content)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendContentReady(to, content);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmContentReady: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmContentEnableFocusRequest(const Guid& to, ContentID content, int32_t focusRequest)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendContentEnableFocusRequest(to, content, focusRequest);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmContentEnableFocusRequest: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmContentDisableFocusRequest(const Guid& to, ContentID content, int32_t focusRequest)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendContentDisableFocusRequest(to, content, focusRequest);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmContentDisableFocusRequest: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmBroadcastRequestStopOfferContent(ContentID content)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendBroadcastRequestStopOfferContent(content, false);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmBroadcastRequestStopOfferContent: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmBroadcastForceStopOfferContent(ContentID content)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendBroadcastRequestStopOfferContent(content, true);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmBroadcastForceStopOfferContent: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmUpdateContentMetadata(const Guid& to, ContentID contentID, const DcsmMetadata& metadata)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendUpdateContentMetadata(to, contentID, metadata);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmUpdateContentMetadata: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmCanvasSizeChange(const Guid& to, ContentID content, const CategoryInfo& ci, AnimationInformation ai)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendCanvasSizeChange(to, content, ci, 0, ai);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmCanvasSizeChange: dcsm not enabled");
        return false;
    }

    bool SomeIPConnectionSystemMultiplexer::sendDcsmContentStateChange(const Guid& to, ContentID content, EDcsmState state, const CategoryInfo& ci, AnimationInformation ai)
    {
        if (m_dcsmConnectionSystem)
            return m_dcsmConnectionSystem->sendContentStateChange(to, content, state, ci, ai);
        LOG_ERROR(CONTEXT_COMMUNICATION, "SomeIPConnectionSystemMultiplexer(" << m_someipCommunicationUserID << ")::sendDcsmContentStateChange: dcsm not enabled");
        return false;
    }

    void SomeIPConnectionSystemMultiplexer::setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler)
    {
        if (m_dcsmConnectionSystem)
            m_dcsmConnectionSystem->setDcsmProviderServiceHandler(handler);
    }

    void SomeIPConnectionSystemMultiplexer::setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler)
    {
        if (m_dcsmConnectionSystem)
            m_dcsmConnectionSystem->setDcsmConsumerServiceHandler(handler);
    }
}
