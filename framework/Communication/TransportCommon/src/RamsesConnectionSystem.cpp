//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/RamsesConnectionSystem.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "TransportCommon/ISceneUpdateSerializer.h"
#include "Scene/SceneActionCollection.h"


namespace ramses_internal
{
    std::unique_ptr<RamsesConnectionSystem> RamsesConnectionSystem::Construct(std::shared_ptr<ISomeIPRamsesStack> stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                                                              UInt32 protocolVersion, PlatformLock& frameworkLock,
                                                                              std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                                                              std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow)
    {
        if (!CheckConstructorArguments(stack, communicationUserID, namedPid, protocolVersion, keepAliveInterval, keepAliveTimeout, CONTEXT_COMMUNICATION, "RAMSES"))
            return nullptr;
        return std::unique_ptr<RamsesConnectionSystem>(new RamsesConnectionSystem(std::move(stack), communicationUserID, namedPid, protocolVersion, frameworkLock,
                                                                                  keepAliveInterval, keepAliveTimeout, std::move(steadyClockNow)));
    }

    RamsesConnectionSystem::RamsesConnectionSystem(std::shared_ptr<ISomeIPRamsesStack> stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                               UInt32 protocolVersion, PlatformLock& frameworkLock,
                                               std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                               std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow)
        : ConnectionSystemBase(stack,
                               communicationUserID,
                               namedPid,
                               protocolVersion,
                               frameworkLock,
                               keepAliveInterval, keepAliveTimeout,
                               std::move(steadyClockNow),
                               CONTEXT_COMMUNICATION,
                               "RAMSES")
        , m_stack(std::move(stack))
    {
    }

    RamsesConnectionSystem::~RamsesConnectionSystem() = default;

    void RamsesConnectionSystem::setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler)
    {
        m_resourceProviderServiceHandler = handler;
    }

    void RamsesConnectionSystem::setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler)
    {
        m_resourceConsumerServiceHandler = handler;
    }

    void RamsesConnectionSystem::setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler)
    {
        m_sceneProviderServiceHandler = handler;
    }

    void RamsesConnectionSystem::setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler)
    {
        m_sceneRendererServiceHandler = handler;
    }

    static std::vector<SceneAvailabilityUpdate> CreateSceneAvailabilityUpdate(const SceneInfoVector& scenes, bool available)
    {
        std::vector<SceneAvailabilityUpdate> update;
        update.reserve(scenes.size());
        for (const auto& entry : scenes)
            update.push_back({entry.sceneID, entry.friendlyName, available});
        return update;
    }

    bool RamsesConnectionSystem::broadcastSceneAvailabilityChange(const char* callerMethod, const SceneInfoVector& scenes, bool available)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "RamsesConnectionSystem(" << m_communicationUserID << ")::" << callerMethod << ": [";
            for (const auto& scene : scenes)
                sos << scene.sceneID << "," << scene.friendlyName << ";";
            sos << "]";
        }));

        std::vector<SceneAvailabilityUpdate> update(CreateSceneAvailabilityUpdate(scenes, available));
        return sendBroadcast(callerMethod, [&](RamsesInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendSceneAvailabilityChange(iid, hdr, update);
        });
    }

    bool RamsesConnectionSystem::broadcastNewScenesAvailable(const SceneInfoVector& newScenes)
    {
        return broadcastSceneAvailabilityChange("broadcastNewScenesAvailable", newScenes, true);
    }

    bool RamsesConnectionSystem::broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes)
    {
        return broadcastSceneAvailabilityChange("broadcastNewScenesAvailable", unavailableScenes, false);
    }

    bool RamsesConnectionSystem::sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "RamsesConnectionSystem(" << m_communicationUserID << ")::sendScenesAvailable: to " << to << " [";
            for (const auto& scene : availableScenes)
                sos << scene.sceneID << "," << scene.friendlyName << ";";
            sos << "]";
        }));

        return sendUnicast("sendScenesAvailable", to, [&](RamsesInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendSceneAvailabilityChange(iid, hdr, CreateSceneAvailabilityUpdate(availableScenes, true));
        });
    }

    bool RamsesConnectionSystem::sendSceneSubscriptionChange(const char* callerMethod, const Guid& to, const SceneId& sceneId, bool subscribed)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "RamsesConnectionSystem(" << m_communicationUserID << ")::" << callerMethod << ": to " << to << ", sceneId " << sceneId);
        return sendUnicast(callerMethod, to, [&](RamsesInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendSceneSubscriptionChange(iid, hdr, {SceneSubscriptionUpdate{sceneId, subscribed}});
        });
    }

    bool RamsesConnectionSystem::sendSubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        return sendSceneSubscriptionChange("sendSubscribeScene", to, sceneId, true);
    }

    bool RamsesConnectionSystem::sendUnsubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        return sendSceneSubscriptionChange("sendUnsubscribeScene", to, sceneId, false);
    }

    bool RamsesConnectionSystem::sendInitializeScene(const Guid& to, const SceneId& sceneId)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "RamsesConnectionSystem(" << m_communicationUserID << ")::sendInitializeScene: to " << to << ", scene " << sceneId);
        return sendUnicast("sendInitializeScene", to, [&](RamsesInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendInitializeScene(iid, hdr, sceneId);
        });
    }

    bool RamsesConnectionSystem::sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer)
    {
        LOG_TRACE(CONTEXT_COMMUNICATION, "RamsesConnectionSystem(" << m_communicationUserID << ")::sendSceneActionList: to " << to <<
                  ", scene " << sceneId);

        const uint32_t blobSize = std::min(m_maxSendBlobSize, m_stack->getSendDataSizes().sceneActionData);
        m_blobSerializationWorkingMemory.resize(blobSize);
        return serializer.writeToPackets({m_blobSerializationWorkingMemory.data(), m_blobSerializationWorkingMemory.size()}, [&](size_t size) {
            m_blobSerializationWorkingMemory.resize(size);
            const bool result = sendUnicast("sendSceneActionList", to, [&](RamsesInstanceId iid, SomeIPMsgHeader hdr) {
                return m_stack->sendSceneUpdate(iid, hdr, sceneId, m_blobSerializationWorkingMemory);
            });
            m_blobSerializationWorkingMemory.resize(blobSize);
            return result;
        });
    }

    bool RamsesConnectionSystem::sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "RamsesConnectionSystem(" << m_communicationUserID << ")::sendRendererEvent: to " << to << ", sceneId " << sceneId << ", dataSize " << data.size());

        // TODO(tobias) also needs some chunking with serialization format here
        return sendUnicast("sendRendererEvent", to, [&](RamsesInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendRendererEvent(iid, hdr, sceneId, data);
        });
    }

    // handlers for generic ISomeIPRamsesCallbacks callbacks
    void RamsesConnectionSystem::handleRendererEvent(const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& data)
    {
        if (const Guid* pid = processReceivedMessageHeader(header, "handleRendererEvent"))
            if (m_sceneProviderServiceHandler)
                m_sceneProviderServiceHandler->handleRendererEvent(sceneId, std::move(data), *pid);
    }

    void RamsesConnectionSystem::handleSceneUpdate(const SomeIPMsgHeader& header, SceneId sceneId, absl::Span<const Byte> sceneUpdateData)
    {
        if (const Guid* pid = processReceivedMessageHeader(header, "handleSceneUpdate"))
            if (m_sceneRendererServiceHandler)
                m_sceneRendererServiceHandler->handleSceneUpdate(sceneId, std::move(sceneUpdateData), *pid);
    }

    void RamsesConnectionSystem::handleSceneAvailabilityChange(const SomeIPMsgHeader& header, const std::vector<SceneAvailabilityUpdate>& update)
    {
        if (const Guid* pid = processReceivedMessageHeader(header, "handleSceneAvailabilityChange"))
            if (m_sceneRendererServiceHandler)
            {
                SceneInfoVector sivAvailable;
                SceneInfoVector sivUnavailable;
                for (const auto& e : update)
                {
                    if (e.available)
                        sivAvailable.push_back(SceneInfo{e.sceneId, e.friendlyName, EScenePublicationMode_LocalAndRemote});
                    else
                        sivUnavailable.push_back(SceneInfo{e.sceneId, e.friendlyName, EScenePublicationMode_LocalAndRemote});
                }
                if (!sivAvailable.empty())
                    m_sceneRendererServiceHandler->handleNewScenesAvailable(sivAvailable, *pid);
                if (!sivUnavailable.empty())
                    m_sceneRendererServiceHandler->handleScenesBecameUnavailable(sivUnavailable, *pid);
            }
    }

    void RamsesConnectionSystem::handleSceneSubscriptionChange(const SomeIPMsgHeader& header, const std::vector<SceneSubscriptionUpdate>& update)
    {
        if (const Guid* pid = processReceivedMessageHeader(header, "handleSceneSubscriptionChange"))
        {
            for (const auto& e : update)
            {
                if (e.subscribed)
                    m_sceneProviderServiceHandler->handleSubscribeScene(e.sceneId, *pid);
                else
                    m_sceneProviderServiceHandler->handleUnsubscribeScene(e.sceneId, *pid);
            }
        }
    }

    void RamsesConnectionSystem::handleInitializeScene(const SomeIPMsgHeader& header, SceneId sceneId)
    {
        if (const Guid* pid = processReceivedMessageHeader(header, "handleInitializeScene"))
            if (m_sceneRendererServiceHandler)
                m_sceneRendererServiceHandler->handleInitializeScene(sceneId, *pid);
    }
}
