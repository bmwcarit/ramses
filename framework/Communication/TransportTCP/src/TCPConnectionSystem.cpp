//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportTCP/TCPConnectionSystem.h"

#include "Components/ManagedResource.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/PlatformSignal.h"
#include "TransportCommon/TransportUtilities.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/StringUtils.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "Scene/SceneActionCollection.h"
#include "Collections/StringOutputStream.h"
#include "Components/ResourceStreamSerialization.h"
#include "Utils/StatisticCollection.h"
#include "Utils/RawBinaryOutputStream.h"

namespace ramses_internal
{
    static const constexpr std::chrono::milliseconds AliveInterval{100};
    static const constexpr std::chrono::milliseconds AliveIntervalTimeout{6 * AliveInterval};

    TCPConnectionSystem::TCPConnectionSystem(const NetworkParticipantAddress& participantAddress, UInt32 protocolVersion, Bool isDaemon,
        const NetworkParticipantAddress& daemonAddress, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection)
        : m_socketManager()
        , m_participantAddress(participantAddress)
        , m_protocolVersion(protocolVersion)
        , m_isDaemon(isDaemon)
        , m_daemonAddress(daemonAddress)
        , m_isRunning(false)
        , m_sendDataSizes(CommunicationSendDataSizes {
            std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max(),
            resourceDataSizeSimilarToOtherStacks, std::numeric_limits<UInt32>::max(),
            std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max() })
        , m_resourceConsumerHandler(0)
        , m_resourceProviderHandler(0)
        , m_sceneProviderHandler(0)
        , m_sceneRendererHandler(0)
        , m_frameworkLock(frameworkLock)
        , m_mainLoop("R_TCP_ConnSys")
        , m_connectionStatusUpdateNotifier(frameworkLock)
        , m_readyToSend(false)
        , m_statisticCollection(statisticCollection)
    {
        // Must disable SIGPIPE when exists
        Signal::DisableSigPipe();
    }

    TCPConnectionSystem::~TCPConnectionSystem()
    {
        disconnectServices();
        clearMessageQueue();
    }

    Guid TCPConnectionSystem::GetDaemonId()
    {
        static Guid guid("BBBC4FDF-44E4-4EE9-8649-E0139E8AE986");
        return guid;
    }

    bool TCPConnectionSystem::broadcastNewScenesAvailable(const SceneInfoVector& newScenes)
    {
        const Guid& providerID = m_participantAddress.getParticipantId();
        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_PublishScene);
        BinaryOutputStream& stream = *msg.stream;

        stream << providerID;
        stream << static_cast<UInt32>(newScenes.size());
        for(const auto& newScene : newScenes)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::broadcastNewScenesAvailable: providerID " << providerID << ", sceneId " << newScene.sceneID.getValue());
            stream << newScene.sceneID.getValue();
            stream << newScene.friendlyName;
        }

        return sendMessage(std::move(msg));
    }

    bool TCPConnectionSystem::sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes)
    {
        const Guid& providerID = m_participantAddress.getParticipantId();
        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_PublishScene, to);
        BinaryOutputStream& stream = *msg.stream;

        stream << providerID;
        stream << static_cast<UInt32>(availableScenes.size());

        for(const auto& availableScene : availableScenes)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendScenesAvailable: to " << to << ", providerID " << providerID << ", sceneId " << availableScene.sceneID.getValue());
            stream << availableScene.sceneID.getValue();
            stream << availableScene.friendlyName;
        }

        return sendMessage(std::move(msg));
    }

    bool TCPConnectionSystem::broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes)
    {
        const Guid& providerID = m_participantAddress.getParticipantId();
        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_UnpublishScene);
        BinaryOutputStream& stream = *msg.stream;

        stream << providerID;
        stream << static_cast<UInt32>(unavailableScenes.size());

        for(const auto& unavailableScene : unavailableScenes)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::broadcastScenesBecameUnavailable: sceneId " << unavailableScene.sceneID.getValue());
            stream << unavailableScene.sceneID.getValue();
            stream << unavailableScene.friendlyName;
        }

        return sendMessage(std::move(msg));
    }

    bool TCPConnectionSystem::sendInitializeScene(const Guid& to, const SceneInfo& sceneInfo)
    {
        const Guid& providerID = m_participantAddress.getParticipantId();
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendCreateScene: to " << to << ", senderId " << providerID << ", sceneName " << sceneInfo.friendlyName << ", sceneId " << sceneInfo.sceneID.getValue());

        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_CreateScene, to);
        BinaryOutputStream& stream = *msg.stream;

        stream << providerID;
        stream << sceneInfo.sceneID.getValue();
        stream << sceneInfo.friendlyName;

        return sendMessage(std::move(msg));
    }

    uint64_t TCPConnectionSystem::sendSceneActionList(const Guid& to, const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& counterStart)
    {
        const Guid& providerID = m_participantAddress.getParticipantId();
        uint64_t numberOfChunks = 0u;
        auto sendChunk =
            [&](Pair<UInt32, UInt32> actionRange, Pair<const Byte*, const Byte*> dataRange, bool isIncomplete)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendSceneActionList: to " << to <<
                ", sceneId " << sceneId.getValue() << ", actions [" << actionRange.first << ", " << actionRange.second <<
                ") from " << actions.numberOfActions());


            OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_SendSceneActionList, to);
            BinaryOutputStream& stream = *msg.stream;

            stream << providerID;
            stream << static_cast<UInt32>(actionRange.second - actionRange.first);
            stream << static_cast<UInt32>(dataRange.second - dataRange.first);
            stream << sceneId.getValue();

            const UInt32 actionOffsetBase = actions[actionRange.first].offsetInCollection();
            for (UInt32 idx = actionRange.first; idx < actionRange.second; ++idx)
            {
                const SceneActionCollection::SceneActionReader reader(actions[idx]);
                ESceneActionId type = (idx == actionRange.second - 1 && isIncomplete) ? ESceneActionId_Incomplete : reader.type();
                stream << static_cast<UInt32>(type);
                stream << reader.offsetInCollection() - actionOffsetBase;
            }
            stream.write(dataRange.first, static_cast<UInt32>(dataRange.second - dataRange.first));

            stream << counterStart + numberOfChunks;
            numberOfChunks++;
            sendMessage(std::move(msg));
        };

        TransportUtilities::SplitSceneActionsToChunks(actions, m_sendDataSizes.sceneActionNumber, m_sendDataSizes.sceneActionDataArray, sendChunk);
        return numberOfChunks;
    }

    bool TCPConnectionSystem::sendResourcesNotAvailable(const Guid& to, const ResourceContentHashVector& resources)
    {
        const Guid& providerID = m_participantAddress.getParticipantId();
        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_ResourcesNotAvailable, to);
        BinaryOutputStream& stream = *msg.stream;

        stream << static_cast<UInt32>(resources.size());
        for (const auto resourceHash : resources)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendResourcesNotAvailable: to " << to << ", resource hash " << resourceHash << ", providerID " << providerID);
            stream << resourceHash;
        }
        stream << providerID;

        return sendMessage(std::move(msg));
    }

    void TCPConnectionSystem::handleResourcesNotAvailable(InMessage& message)
    {
        if (m_resourceConsumerHandler)
        {
            UInt32 numResources;
            message.stream >> numResources;

            ResourceContentHashVector resources;
            resources.reserve(numResources);

            for (UInt32 i = 0; i < numResources; ++i)
            {
                ResourceContentHash resourceHash;
                message.stream >> resourceHash;
                resources.push_back(resourceHash);
            }

            Guid providerID;
            message.stream >> providerID;

            PlatformGuard guard(m_frameworkLock);
            m_resourceConsumerHandler->handleResourcesNotAvailable(resources, providerID);
        }
    }

    void TCPConnectionSystem::handleSceneSubscription(InMessage& message)
    {
        if (m_sceneProviderHandler)
        {
            SceneId sceneId;
            message.stream >> sceneId.getReference();

            Guid consumerID;
            message.stream >> consumerID;

            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleSubscribeScene(sceneId, consumerID);
        }
    }

    void TCPConnectionSystem::handleSceneUnsubscription(InMessage& message)
    {
        if (m_sceneProviderHandler)
        {
            SceneId sceneId;
            message.stream >> sceneId.getReference();

            Guid subscriber;
            message.stream >> subscriber;

            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleUnsubscribeScene(sceneId, subscriber);
        }
    }

    void TCPConnectionSystem::handleSceneActionList(InMessage& message)
    {
        if (m_sceneRendererHandler)
        {
            Guid providerID;
            message.stream >> providerID;

            UInt32 actionsListSize = 0;
            message.stream >> actionsListSize;
            UInt32 actionDataSize = 0;
            message.stream >> actionDataSize;

            SceneId sceneId;
            message.stream >> sceneId.getReference();

            SceneActionCollection actions(actionDataSize, actionsListSize);
            for (UInt32 i = 0; i < actionsListSize; ++i)
            {
                UInt32 type = 0;
                UInt32 offset = 0;
                message.stream >> type;
                message.stream >> offset;
                actions.addRawSceneActionInformation(static_cast<ESceneActionId>(type), offset);
            }

            Vector<Byte>& rawActionData = actions.getRawDataForDirectWriting();
            rawActionData.resize(actionDataSize);
            message.stream.read(rawActionData.data(), actionDataSize);

            UInt64 sceneactionListCounter = 0;
            message.stream >> sceneactionListCounter;
            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleSceneActionList(sceneId, std::move(actions), sceneactionListCounter, providerID);
        }
    }

    void TCPConnectionSystem::handleScenePublication(InMessage& message)
    {
        if (m_sceneRendererHandler)
        {
            Guid providerID;
            message.stream >> providerID;

            UInt32 numScenes;
            message.stream >> numScenes;

            SceneInfoVector newScenes;
            newScenes.reserve(numScenes);

            for (UInt32 i = 0; i < numScenes; ++i)
            {
                SceneInfo sceneInfo;
                message.stream >> sceneInfo.sceneID.getReference();
                message.stream >> sceneInfo.friendlyName;
                newScenes.push_back(sceneInfo);
            }

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        }
    }

    void TCPConnectionSystem::handleSceneUnpublication(InMessage& message)
    {
        if (m_sceneRendererHandler)
        {
            Guid providerID;
            message.stream >> providerID;

            UInt32 numScenes;
            message.stream >> numScenes;

            SceneInfoVector unavailableScenes;
            unavailableScenes.reserve(numScenes);

            for (UInt32 i = 0; i < numScenes; ++i)
            {
                SceneInfo sceneInfo;
                message.stream >> sceneInfo.sceneID.getReference();
                message.stream >> sceneInfo.friendlyName;
                unavailableScenes.push_back(sceneInfo);
            }

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleScenesBecameUnavailable(unavailableScenes, providerID);
        }
    }

    void TCPConnectionSystem::handleCreateScene(InMessage& message)
    {
        if (m_sceneRendererHandler)
        {
            Guid providerID;
            message.stream >> providerID;

            SceneInfo sceneInfo;
            message.stream >> sceneInfo.sceneID.getReference();
            message.stream >> sceneInfo.friendlyName;

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleInitializeScene(sceneInfo, providerID);
        }
    }

    void TCPConnectionSystem::handleIncomingResource(InMessage& message)
    {
        if (m_resourceConsumerHandler)
        {
            UInt32 bytesRead = 0;
            Guid providerID;
            message.stream >> providerID;
            bytesRead += sizeof(generic_uuid_t);

            UInt32 dataSize;
            message.stream >> dataSize;
            bytesRead += sizeof(dataSize);

            PlatformGuard guard(m_frameworkLock);

            ByteArrayView resourceData(message.data.data() + bytesRead, static_cast<UInt32>(message.data.size() - bytesRead));
            m_resourceConsumerHandler->handleSendResource(resourceData, providerID);
        }
    }

    void TCPConnectionSystem::handleResourceRequest(InMessage& message)
    {
        if (m_resourceProviderHandler)
        {
            UInt32 listLength = 0;
            message.stream >> listLength;

            ResourceContentHashVector resourceHashes;
            resourceHashes.reserve(listLength);
            for (UInt32 i = 0; i < listLength; ++i)
            {
                ResourceContentHash resourceHash;
                message.stream >> resourceHash;
                resourceHashes.push_back(resourceHash);
            }

            UInt32 chunkSize = 0;
            message.stream >> chunkSize;

            Guid requesterId;
            message.stream >> requesterId;

            PlatformGuard guard(m_frameworkLock);
            m_resourceProviderHandler->handleRequestResources(resourceHashes, chunkSize, requesterId);
        }
    }

    bool TCPConnectionSystem::sendResources(const Guid& to, const ManagedResourceVector& managedResources)
    {
        if (!m_readyToSend)
            return false;

        for (const auto& managedResource : managedResources)
        {
            const IResource* resource = managedResource.getResourceObject();
            assert(resource != NULL);
            // try to compress for network sending
            resource->compress(IResource::CompressionLevel::REALTIME);

            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendResource: to " << to << ", type " << EnumToString(resource->getTypeID()) <<
                ", resourceHash " << resource->getHash() << ", name " << resource->getName());
        }

        Vector<Byte> buffer;

        auto preparePacketFun = [&](UInt32 neededSize) -> Pair<Byte*, UInt32> {
            buffer.resize(std::min(m_sendDataSizes.resourceDataArray, neededSize));
            return MakePair(buffer.data(), static_cast<UInt32>(buffer.size()));
        };

        auto finishedPacketFun = [&](UInt32 usedSize) {
            assert(usedSize <= buffer.size());

            OutMessage msg(EConnectionType_LargeDataTransfer, EMessageId_TransferResources, to);
            BinaryOutputStream& stream = *msg.stream;

            stream << m_participantAddress.getParticipantId();
            stream << usedSize;
            stream.write(buffer.data(), usedSize);

            sendMessage(std::move(msg));

            m_statisticCollection.statResourcesSentSize.incCounter(usedSize);
        };

        ResourceStreamSerializer serializer;
        serializer.serialize(preparePacketFun, finishedPacketFun, managedResources);
        return true;
    }

    bool TCPConnectionSystem::sendRequestResources(const Guid& to, const ResourceContentHashVector& resources)
    {
        if (!m_readyToSend)
            return false;

        const Guid& requesterId = m_participantAddress.getParticipantId();
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendRequestResources: to " << to << ", numResources " << resources.size() << ", requesterId " << requesterId);

        UInt32 resourceRequestsLeft         = static_cast<UInt32>(resources.size());
        const UInt32 maxNumResourceRequests = m_sendDataSizes.resourceInfoNumber;
        auto it = resources.begin();

        while(it != resources.end())
        {
            OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_RequestResources, to);
            BinaryOutputStream& stream = *msg.stream;

            UInt32 resourceRequestsToWrite = min(resourceRequestsLeft, maxNumResourceRequests);
            resourceRequestsLeft -= resourceRequestsToWrite;

            stream << static_cast<UInt32>(resourceRequestsToWrite);
            for(;resourceRequestsToWrite > 0u; --resourceRequestsToWrite, ++it)
            {
                ResourceContentHash resourceHash = *it;
                stream << resourceHash;
            }
            stream << 0u;
            stream << requesterId;

            sendMessage(std::move(msg));
        }
        return true;
    }

    bool TCPConnectionSystem::sendSubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        const Guid& consumerID = m_participantAddress.getParticipantId();
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendSubscribeScene: to " << to << ", sceneId " << sceneId.getValue() << ", consumer " << consumerID);

        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_SubscribeScene, to);
        BinaryOutputStream& stream = *msg.stream;

        stream << sceneId.getValue();
        stream << consumerID;

        return sendMessage(std::move(msg));
    }

    bool TCPConnectionSystem::sendUnsubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        const Guid& consumerID = m_participantAddress.getParticipantId();
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendUnsubscribeScene: to " << to << ", sceneId " << sceneId.getValue() << ", consumer " << consumerID);

        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_UnsubscribeScene, to);
        BinaryOutputStream& stream = *msg.stream;

        stream << sceneId.getValue();
        stream << consumerID;

        return sendMessage(std::move(msg));
    }

    bool TCPConnectionSystem::sendSceneNotAvailable(const Guid& to, const SceneId& sceneId)
    {
        const Guid& providerID = m_participantAddress.getParticipantId();
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendSceneNotAvailable: to " << to << ", sceneId " << sceneId.getValue() << ", consumer " << providerID);

        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_SceneNotAvailable, to);
        BinaryOutputStream& stream = *msg.stream;

        stream << sceneId.getValue();
        stream << providerID;

        return sendMessage(std::move(msg));
    }

    void TCPConnectionSystem::handleSceneNotAvailable(InMessage& message)
    {
        if (m_sceneRendererHandler)
        {
            SceneId sceneId;
            message.stream >> sceneId.getReference();

            Guid providerID;
            message.stream >> providerID;

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleSceneNotAvailable(sceneId, providerID);
        }
    }

    CommunicationSendDataSizes TCPConnectionSystem::getSendDataSizes() const
    {
        return m_sendDataSizes;
    }

    void TCPConnectionSystem::setSendDataSizes(const CommunicationSendDataSizes& sizes)
    {
        m_sendDataSizes = sizes;
    }

    void TCPConnectionSystem::setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler)
    {
        m_resourceProviderHandler = handler;
    }

    void TCPConnectionSystem::setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler)
    {
        m_resourceConsumerHandler = handler;
    }

    void TCPConnectionSystem::setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler)
    {
        m_sceneProviderHandler = handler;
    }

    void TCPConnectionSystem::setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler)
    {
        m_sceneRendererHandler = handler;
    }

    bool TCPConnectionSystem::connectServices()
    {
        PlatformGuard g(m_startStopLock);
        if (!m_isRunning)
        {
            if (!setupListener())
            {
                return false;
            }
            m_isRunning = true;
            m_mainLoop.start(*this);
        }
        return true;
    }

    bool TCPConnectionSystem::disconnectServices()
    {
        PlatformGuard g(m_startStopLock);
        if (m_isRunning)
        {
            cancel();
            m_mainLoop.join();
            m_isRunning = false;
            Runnable::resetCancel();
        }
        return true;
    }

    bool TCPConnectionSystem::sendMessage(OutMessage&& message)
    {
        PlatformGuard g(m_mainLock);
        if (m_readyToSend)
        {
            m_outMessagesToSend.push_back(std::move(message));
            m_socketManager.interruptWaitCall();
        }
        m_statisticCollection.statMessagesSent.incCounter(1);
        return m_readyToSend;
    }

    IConnectionStatusUpdateNotifier& TCPConnectionSystem::getConnectionStatusUpdateNotifier()
    {
        return m_connectionStatusUpdateNotifier;
    }

    void TCPConnectionSystem::cancel()
    {
        Runnable::cancel();
        m_socketManager.interruptWaitCall();
    }

    Bool TCPConnectionSystem::setupListener()
    {
        assert(m_serverSocket.get() == 0);

        ScopedPointer<PlatformServerSocket> tmpServerSocket(new PlatformServerSocket());
        if (EStatus_RAMSES_OK != tmpServerSocket->bind(m_participantAddress.getPort()) ||
            EStatus_RAMSES_OK != tmpServerSocket->listen(4))
        {
            return false;
        }
        m_serverSocket.reset(tmpServerSocket.release());

        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::setupListener: accepting incoming connections on port " << m_serverSocket->getPort());
        return true;
    }

    void TCPConnectionSystem::cleanupListener()
    {
        m_serverSocket->close();
        m_serverSocket.reset();
    }

    void TCPConnectionSystem::run()
    {
        assert(m_serverSocket.get() != 0);

        // outer loop to retry connection to daemon when got disconnected
        while (!isCancelRequested())
        {
            if (!m_isDaemon)
            {
                m_knownParticipantAddresses.put(m_participantAddress.getParticipantId(), m_participantAddress);
                connectToDaemon();
            }

            trackServerSocket();

            setReadyToSendMessages(true);
            clearMessageQueue();

            // inner loop for normal sending and receiving messages
            while (!isCancelRequested())
            {
                // try connect
                tryConnectToOthers();

                sendAllMessagesInQueue();
                if (!sendAliveMessages())
                    break;

                handleLogRequests();

                // read from socket
                const uint32_t socketCheckTimeout_ms = std::chrono::duration_cast<std::chrono::duration<uint32_t, std::milli>>(AliveInterval).count();
                const bool canBlock = false;
                m_socketManager.checkAllSockets(canBlock, socketCheckTimeout_ms);

                if (!checkConnectionsAlive())
                    break;
            }

            setReadyToSendMessages(false);
        }

        cleanupListener();

        closeAndCleanupAllConnections();
        clearMessageQueue();
    }

    void TCPConnectionSystem::connectToDaemon()
    {
        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectToDaemon: trying to connect to daemon at "
            << m_daemonAddress.getIp() << ":" << m_daemonAddress.getPort());

        assert(m_daemonSocket.socket == nullptr);
        PlatformSocket* socket = new PlatformSocket();
        while (!isCancelRequested())
        {
            EStatus status = socket->connect(m_daemonAddress.getIp().c_str(), m_daemonAddress.getPort(), socketConnectTimeoutMs);
            if (status == EStatus_RAMSES_OK)
            {
                LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectToDaemon: connection to daemon established");
                delete m_daemonSocket.socket;
                m_daemonSocket.socket = socket;

                if (sendConnectionDescriptionMessage(m_daemonSocket, m_daemonAddress, EConnectionType_OrderedControlMessages))
                {
                    trackSocket(*m_daemonSocket.socket);
                    return;
                }
                else
                {
                    // try again with new socket
                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectToDaemon: send connection description failed");
                    delete m_daemonSocket.socket;
                    m_daemonSocket.socket = nullptr;
                    socket = new PlatformSocket();
                }
            }
            handleLogRequests();
            PlatformThread::Sleep(100);
        }
        delete socket;
    }

    void TCPConnectionSystem::tryConnectToOthers()
    {
        if (m_isDaemon)
        {
            // daemon never initiates connections
            return;
        }

        Vector<Guid> nowFinishedConnections;
        for(const auto& connecton : m_unfinishedConnections)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::tryConnectToOthers: " << connecton << " " << shouldTryConnectTo(connecton));

            if (shouldTryConnectTo(connecton))
            {
                assert(m_knownParticipantAddresses.contains(connecton));
                const NetworkParticipantAddress& address = *m_knownParticipantAddresses.get(connecton);

                if (connectWithType(address, EConnectionType_OrderedControlMessages, m_controlSockets) &&
                    connectWithType(address, EConnectionType_LargeDataTransfer, m_dataSockets))
                {
                    nowFinishedConnections.push_back(connecton);
                    m_connectionStatusUpdateNotifier.triggerNotification(connecton, EConnectionStatus_Connected);
                }
            }
        }

        // delayed cleanup, prevent modify container while iterating
        for(const auto& connecton : nowFinishedConnections)
        {
            m_unfinishedConnections.remove(connecton);
        }
    }

    Bool TCPConnectionSystem::shouldTryConnectTo(const Guid& otherId) const
    {
        // use global ordering if guids to decide who is server and who is client in a connection
        const generic_uuid_t& myUUID = m_participantAddress.getParticipantId().getGuidData();
        const generic_uuid_t& otherUUID = otherId.getGuidData();
        return PlatformMemory::Compare(&myUUID, &otherUUID, sizeof(generic_uuid_t)) > 0;
    }

    Bool TCPConnectionSystem::connectWithType(const NetworkParticipantAddress& address, EConnectionType type, HashMap<Guid, PlatformSocket*>& socketMap)
    {
        const Guid& participantId = address.getParticipantId();
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectWithType: try connect to " << address.getParticipantName() << ":" << participantId << " (type " << type << ")");

        if (socketMap.contains(participantId))
        {
            return true;
        }

        PlatformSocket* socket = new PlatformSocket();
        EStatus status = socket->connect(address.getIp().c_str(), address.getPort());
        if (status == EStatus_RAMSES_OK)
        {
            SocketInfo socketInfo = {participantId, socket};
            Bool writeSuccessful = sendConnectionDescriptionMessage(socketInfo, address, type);
            trackSocket(*socket);
            socketMap.put(participantId, socket);
            m_platformSocketMap.put(socket, socketInfo);

            if (writeSuccessful)
            {
                LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectWithType: connected");
                return true;
            }
            else
            {
                removeKnownParticipant(address.getParticipantId(), true);
                return false;
            }
        }
        else
        {
            delete socket;
            return false;
        }
    }

    void TCPConnectionSystem::sendAllOutMessages(const Vector<OutMessage>& messagesToSend)
    {
        for (const auto& message : messagesToSend)
        {
            Vector<Guid> brokenConnections;

            if (message.to.isInvalid())
            {
                assert(message.connectionType == EConnectionType_OrderedControlMessages);
                sendBroadcastMessageToSockets(message, brokenConnections);
            }
            else
            {
                assert(message.connectionType == EConnectionType_OrderedControlMessages || message.connectionType == EConnectionType_LargeDataTransfer);
                sendUnicastMessageToSocket(message, brokenConnections);
            }

            // remove participants we cannot write to anymore
            for(const auto& connecton : brokenConnections)
            {
                removeKnownParticipant(connecton, true);
            }
        }
    }

    void TCPConnectionSystem::sendUnicastMessageToSocket(const OutMessage& message, Vector<Guid>& brokenConnections)
    {
        PlatformSocket* controlSocket = 0;
        PlatformSocket* dataSocket = 0;

        if (m_controlSockets.get(message.to, controlSocket) == EStatus_RAMSES_OK &&
            m_dataSockets.get(message.to, dataSocket) == EStatus_RAMSES_OK)
        {
            Bool writeSuccessful = true;
            if (message.connectionType == EConnectionType_OrderedControlMessages)
            {
                assert(m_platformSocketMap.contains(controlSocket));
                writeSuccessful = sendMessageToSocket(*m_platformSocketMap.get(controlSocket), message);
            }
            else
            {
                assert(m_platformSocketMap.contains(dataSocket));
                writeSuccessful = sendMessageToSocket(*m_platformSocketMap.get(dataSocket), message);
            }

            if (!writeSuccessful)
            {
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendUnicastMessageToSocket: write to " << message.to << " failed");
                brokenConnections.push_back(message.to);
            }
        }
        else
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendUnicastMessageToSocket: drop message '" << GetNameForMessageId(message.messageType) << "' to unknown location " << message.to);
        }
    }

    void TCPConnectionSystem::sendBroadcastMessageToSockets(const OutMessage& message, Vector<Guid>& brokenConnections)
    {
        // send to all valid connections
        for(const auto& controlSocket : m_controlSockets)
        {
            if (m_dataSockets.contains(controlSocket.key))
            {
                assert(m_platformSocketMap.contains(controlSocket.value));
                if (!sendMessageToSocket(*m_platformSocketMap.get(controlSocket.value), message))
                {
                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendBroadcastMessageToSockets: write to " << controlSocket.key << " failed");
                    brokenConnections.push_back(controlSocket.key);
                }
            }
        }
    }

    Bool TCPConnectionSystem::sendMessageToSocket(SocketInfo& si, const OutMessage& message) const
    {
        const char* data = message.stream->getData();
        const UInt32 size = message.stream->getSize();

        RawBinaryOutputStream rawStream(reinterpret_cast<UInt8*>(const_cast<char*>(data)), size);
        const UInt32 messageTypeConv = htonl(static_cast<UInt32>(message.messageType));
        const UInt32 payloadLengthConv = htonl(static_cast<UInt32>(size - 2 * sizeof(UInt32)));
        rawStream << messageTypeConv << payloadLengthConv;

        uint32_t sentBytes = 0;
        do
        {
            int32_t numBytes = 0;
            if (si.socket->send(data + sentBytes, size - sentBytes, numBytes) != EStatus_RAMSES_OK)
            {
                return false;
            }
            sentBytes += numBytes;
        }
        while (sentBytes != size);

        si.lastSent = std::chrono::steady_clock::now();

        return true;
    }

    void TCPConnectionSystem::trackSocket(PlatformSocket& socket)
    {
        m_socketManager.trackSocket(&socket, TcpSocketDelegate::Create<TCPConnectionSystem, &TCPConnectionSystem::socketHasData>(*this));
    }

    void TCPConnectionSystem::trackServerSocket()
    {
        m_socketManager.trackSocket(m_serverSocket.get(), ServerSocketDelegate::Create<TCPConnectionSystem, &TCPConnectionSystem::acceptIncomingConnection>(*this));
    }

    void TCPConnectionSystem::acceptIncomingConnection(PlatformServerSocket& serverSocket)
    {
        trackServerSocket();

        PlatformSocket* clientSocket = serverSocket.accept(0);
        if (0 != clientSocket)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::acceptIncomingConnection:");

            m_newlyAcceptedSockets.push_back(clientSocket);
            m_platformSocketMap.put(clientSocket, {Guid(false), clientSocket});
            trackSocket(*clientSocket);
        }
        else
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::acceptIncomingConnection: accept failed");
        }
    }

    Bool TCPConnectionSystem::sendConnectionDescriptionMessage(SocketInfo& si, const NetworkParticipantAddress& to, const EConnectionType& type)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendConnectionDescriptionMessage: to " << to.getParticipantName() << ":" << to.getParticipantId());

        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_ConnectionDescriptionMessage);
        BinaryOutputStream& stream = *msg.stream;

        stream << m_protocolVersion;
        stream << m_participantAddress.getParticipantId();
        stream << m_participantAddress.getParticipantName();
        stream << m_participantAddress.getIp();
        stream << m_serverSocket->getPort();
        stream << static_cast<UInt32>(type);

        return sendMessageToSocket(si, msg);
    }

    Bool TCPConnectionSystem::sendAddressExchangeMessage(SocketInfo& si, const NetworkParticipantAddress& address, const Guid& to) const
    {
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendAddressExchange: send " << address.getParticipantName() << ":" << address.getParticipantId() << " to " << to);

        OutMessage msg(EConnectionType_OrderedControlMessages, EMessageId_ConnectorAddressExchange);
        BinaryOutputStream& stream = *msg.stream;

        stream << address.getParticipantId();
        stream << address.getParticipantName();
        stream << address.getIp();
        stream << address.getPort();

        return sendMessageToSocket(si, msg);
    }

    void TCPConnectionSystem::socketHasData(PlatformSocket& socket)
    {
        SocketInfo* socketInfo = nullptr;

        // check if from daemon
        if (m_daemonSocket.socket && &socket == m_daemonSocket.socket)
        {
            socketInfo = &m_daemonSocket;
        }
        // check if has socket (newly or known participant)
        else if ((socketInfo = m_platformSocketMap.get(&socket)) != nullptr)
        {
            assert(socketInfo->socket != nullptr);
        }
        // completely unknown socket, should never happen
        else
        {
            LOG_FATAL(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::socketHasData: data on unknown socket");
            assert(false);
            return;
        }
        assert(socketInfo != nullptr);

        const Guid participantId = socketInfo->participant;
        std::unique_ptr<InMessage> message(receiveMessageFromSocket(socket));
        Bool handledSuccessfully = false;
        if (nullptr != message)
        {
            message->sender = participantId;
            handledSuccessfully = handleMessage(participantId, socket, *message);

            // update lastReceived timestamp
            socketInfo->lastReceived = std::chrono::steady_clock::now();
        }
        else
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::socketHasData: readMessage failed on stream socket");
        }

        if (handledSuccessfully)
        {
            trackSocket(socket);
        }
        else
        {
            // we don't want to talk to someone who has failed us anymore
            if (participantId.isInvalid())
            {
                dropConnectionToNewlyAcceptedSocket(socket);
            }
            else
            {
                removeKnownParticipant(participantId, true);
            }
        }
    }

    std::unique_ptr<TCPConnectionSystem::InMessage> TCPConnectionSystem::receiveMessageFromSocket(PlatformSocket& socket)
    {
        UInt32 header[2];
        if (!readDataFromSocket(socket, reinterpret_cast<char*>(header), sizeof(header)))
        {
            return nullptr;
        }

        const EMessageId type = static_cast<EMessageId>(ntohl(header[0]));
        const UInt32 payloadSize = ntohl(header[1]);
        std::unique_ptr<InMessage> message(new InMessage(type, payloadSize));

        if (!readDataFromSocket(socket, reinterpret_cast<char*>(message->data.data()), static_cast<UInt32>(message->data.size())))        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::socketHasData: readMessage(Message) failed on stream socket");
            return nullptr;
        }

        return message;
    }

    bool TCPConnectionSystem::readDataFromSocket(PlatformSocket& socket, char* buffer, UInt32 size)
    {
        UInt32 receivedBytes = 0;
        while (receivedBytes < size)
        {
            Int32 length = 0;
            const EStatus state = socket.receive(buffer + receivedBytes, size - receivedBytes, length);
            if (state != EStatus_RAMSES_OK || length == 0)
            {
                return false;
            }
            receivedBytes += length;
        }
        return true;
    }

    Bool TCPConnectionSystem::handleMessage(const Guid& participantId, PlatformSocket& socket, InMessage& message)
    {
        Bool handledSuccessfully = false;

        m_statisticCollection.statMessagesReceived.incCounter(1);

        // check for messages only important for us
        if (message.type == EMessageId_ConnectionDescriptionMessage)
        {
            // connection description may only come on new connections to still unknown participants
            if (participantId.isInvalid())
            {
                handledSuccessfully = handleConnectionDescriptionMessage(socket, message);
            }
            else
            {
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleMessage: EMessageId_ConnectionDescriptionMessage from known participant, drop connection");
            }
        }
        else if (message.type == EMessageId_ConnectorAddressExchange)
        {
            handleConnectorAddressExchangeMessage(message);
            handledSuccessfully = true;
        }
        else if (message.type == EMessageId_Alive)
        {
            // valid but handled outside
            handledSuccessfully = true;
        }
        else
        {
            if (participantId.isInvalid())
            {
                // now allowed from unknown
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleMessage: unexpected message "
                         << GetNameForMessageId(message.type) << " (" << message.type << ") from unknown participant");
            }
            else
            {
                handledSuccessfully = dispatchReceivedMessage(message);
            }
        }
        return handledSuccessfully;
    }

    bool TCPConnectionSystem::dispatchReceivedMessage(InMessage& message)
    {
        switch (message.type)
        {
        case EMessageId_RequestResources:
            handleResourceRequest(message);
            break;
        case EMessageId_TransferResources:
            handleIncomingResource(message);
            break;
        case EMessageId_ResourcesNotAvailable:
            handleResourcesNotAvailable(message);
            break;
        case EMessageId_SubscribeScene:
            handleSceneSubscription(message);
            break;
        case EMessageId_UnsubscribeScene:
            handleSceneUnsubscription(message);
            break;
        case EMessageId_CreateScene:
            handleCreateScene(message);
            break;
        case EMessageId_SendSceneActionList:
            handleSceneActionList(message);
            break;
        case EMessageId_PublishScene:
            handleScenePublication(message);
            break;
        case EMessageId_SceneNotAvailable:
            handleSceneNotAvailable(message);
            break;
        case EMessageId_UnpublishScene:
            handleSceneUnpublication(message);
            break;
        default:
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::dispatchReceivedMessage: unknown message type "
                << GetNameForMessageId(message.type) << " from " << message.sender);
            return false;
        }
        return true;
    }

    void TCPConnectionSystem::handleConnectorAddressExchangeMessage(InMessage& message)
    {
        Guid id;
        message.stream >> id;

        String name;
        message.stream >> name;

        String ip;
        message.stream >> ip;

        UInt16 port;
        message.stream >> port;

        const NetworkParticipantAddress newAddress(id, name, ip, port);
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectorAddressExchange: received new address " << name << ":" << id << " on " << ip << ":" << port);

        addNewParticipantIfUnknown(newAddress, "ConnectorAddressExchange message");
    }

    Bool TCPConnectionSystem::handleConnectionDescriptionMessage(PlatformSocket& socket, InMessage& message)
    {
        UInt32 protocolVersion = 0;
        message.stream >> protocolVersion;
        if (protocolVersion != m_protocolVersion)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectionDescriptionMessage: protocol version of daemon (" << m_protocolVersion << ") does not match with new participant (" << protocolVersion << ")");

            //the daemon will check the version of every new participant
            //-> mismatch can only happen between new participant and daemon
            assert(m_isDaemon);

            //this is the first message of the new participant, the platform map
            //must not contain this socket yet
            assert(!m_platformSocketMap.contains(&socket));

            //do not remove socket from newly accepted connections, will be done
            //by socketHasData() by calling dropConnectionToNewlyAcceptedSocket()
            return false;
        }

        Guid id;
        message.stream >> id;

        String name;
        message.stream >> name;

        String ip;
        message.stream >> ip;

        UInt16 port;
        message.stream >> port;

        UInt32 connectionType;
        message.stream >> connectionType;

        const NetworkParticipantAddress newAddress(id, name, ip, port);
        addNewParticipantIfUnknown(newAddress, "ConnectionDescription message");

        // remove from newly accepted connections
        const auto newlyAcceptedIt = m_newlyAcceptedSockets.find(&socket);
        assert(newlyAcceptedIt != m_newlyAcceptedSockets.end());
        m_newlyAcceptedSockets.erase(newlyAcceptedIt);
        m_platformSocketMap.remove(&socket);

        // handle depending on connection type
        assert(connectionType == EConnectionType_OrderedControlMessages || connectionType == EConnectionType_LargeDataTransfer);

        if (connectionType == EConnectionType_OrderedControlMessages)
        {
            if (m_controlSockets.contains(id))
            {
                LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectionDescriptionMessage: got ConnectionDescriptionMessage for OrderedControlMessages twice on same connection");
                return false;
            }
            else
            {
                SocketInfo socketInfo = {id, &socket};
                Bool writeSuccessful = sendAddressExchangeForNewParticipant(socketInfo, newAddress);
                m_controlSockets.put(id, &socket);
                m_platformSocketMap.put(&socket, socketInfo);

                if (!writeSuccessful)
                {
                    removeKnownParticipant(newAddress.getParticipantId(), true);
                    return false;
                }
            }
        }
        else
        {
            if (m_dataSockets.contains(id))
            {
                LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectionDescriptionMessage: got ConnectionDescriptionMessage for LargeDataTransfer twice on same connection");
                return false;
            }
            else
            {
                m_dataSockets.put(id, &socket);
                m_platformSocketMap.put(&socket, {id, &socket});
            }
        }

        // check if now fully connected to this participant
        if (m_controlSockets.contains(id) && m_dataSockets.contains(id))
        {
            m_unfinishedConnections.remove(id);
            m_connectionStatusUpdateNotifier.triggerNotification(id, EConnectionStatus_Connected);
        }

        return true;
    }

    Bool TCPConnectionSystem::sendAddressExchangeForNewParticipant(SocketInfo& si, const NetworkParticipantAddress& newAddress)
    {
        // send all known addresses to new connection
        for (const auto& p : m_knownParticipantAddresses)
        {
            if (p.key != m_participantAddress.getParticipantId())
            {
                if (!sendAddressExchangeMessage(si, p.value, newAddress.getParticipantId()))
                {
                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendAddressExchangeForNewParticipant: send to new participant failed");
                    return false;
                }
            }
        }

        // send new address to already known connections
        for (const auto& p : m_controlSockets)
        {
            // only log send error here for now
            assert(m_platformSocketMap.contains(p.value));
            if (!sendAddressExchangeMessage(*m_platformSocketMap.get(p.value), newAddress, p.key))
            {
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendAddressExchangeForNewParticipant: send to existing participant failed");
            }
        }
        return true;
    }

    void TCPConnectionSystem::addNewParticipantIfUnknown(const NetworkParticipantAddress& address, const char* reason)
    {
        const Guid participantId = address.getParticipantId();

        // not self
        if (participantId != m_participantAddress.getParticipantId())
        {
            // already known?
            NetworkParticipantAddress* oldAddress = m_knownParticipantAddresses.get(participantId);
            if (oldAddress && *oldAddress != address)
            {
                // address changed: update participant info
                const bool wasConnected =  m_controlSockets.contains(participantId) || m_dataSockets.contains(participantId);

                LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::addNewParticipantIfUnknown: update existing participant " <<
                         address.getParticipantName() << ":" << participantId << "; reason " << reason << " (" << address.getIp() << ":" << address.getPort() << "); wasConnected " << wasConnected);

                if (wasConnected)
                {
                    // disconnect first (no retry to avoid recursion)
                    removeKnownParticipant(participantId, false);
                }

                // update information
                m_knownParticipantAddresses.put(participantId, address);
                m_unfinishedConnections.put(participantId);
            }
            else if (!oldAddress)
            {
                // add new / not yet known participant
                LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::addNewParticipantIfUnknown: add participant " <<
                         address.getParticipantName() << ":" << participantId << "; reason " << reason);

                m_knownParticipantAddresses.put(participantId, address);
                m_unfinishedConnections.put(participantId);
            }
        }
    }

    void TCPConnectionSystem::removeKnownParticipant(const Guid& idRef, bool addForNewAttempt)
    {
        assert(m_knownParticipantAddresses.contains(idRef));
        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::removeKnownParticipant: remove participant " << idRef << ", addForRetry: " << addForNewAttempt);

        // prevent holding reference to entry that max be removed here
        const Guid id = idRef;
        NetworkParticipantAddress address = *m_knownParticipantAddresses.get(idRef);

        m_unfinishedConnections.remove(id);
        m_knownParticipantAddresses.remove(id);

        // remove streamsockets
        PlatformSocket* controlSocket = nullptr;
        bool hadControlSocket = false;
        if (EStatus_RAMSES_OK == m_controlSockets.remove(id, &controlSocket))
        {
            assert(controlSocket);
            hadControlSocket = true;
            m_platformSocketMap.remove(controlSocket);
            m_socketManager.untrackSocket(controlSocket);
            delete controlSocket;
        }

        PlatformSocket* dataSocket = nullptr;
        bool hadDataSocket = false;
        if (EStatus_RAMSES_OK == m_dataSockets.remove(id, &dataSocket))
        {
            assert(dataSocket);
            hadDataSocket = true;
            m_platformSocketMap.remove(dataSocket);
            m_socketManager.untrackSocket(dataSocket);
            delete dataSocket;
        }

        if (hadControlSocket && hadDataSocket)
        {
            // was fully connected, send disconnect notification
            m_connectionStatusUpdateNotifier.triggerNotification(id, EConnectionStatus_NotConnected);
        }

        // TODO(tobias) ignore request for new attempt until non-blocking connect on windows works
        // if (addForNewAttempt)
        //     addNewParticipantIfUnknown(address, "retry after connection error");
    }

    void TCPConnectionSystem::dropConnectionToNewlyAcceptedSocket(PlatformSocket& socket)
    {
        const auto it = m_newlyAcceptedSockets.find(&socket);
        if (it != m_newlyAcceptedSockets.end())
        {
            assert(m_platformSocketMap.contains(&socket));
            m_platformSocketMap.remove(&socket);
            m_newlyAcceptedSockets.erase(it);
            m_socketManager.untrackSocket(&socket);
            delete &socket;
        }
    }

    void TCPConnectionSystem::closeAndCleanupAllConnections()
    {
        // closed and deleted outside thread because also created there
        m_socketManager.untrackSocket(m_serverSocket.get());

        if (m_daemonSocket.socket)
        {
            m_socketManager.untrackSocket(m_daemonSocket.socket);
            delete m_daemonSocket.socket;
            m_daemonSocket.socket = nullptr;
        }

        while (!m_newlyAcceptedSockets.empty())
        {
            assert(m_newlyAcceptedSockets.front() != nullptr);
            dropConnectionToNewlyAcceptedSocket(*m_newlyAcceptedSockets.front());
        }

        while (m_knownParticipantAddresses.count() > 0)
        {
            auto it = m_knownParticipantAddresses.begin();
            removeKnownParticipant(it->key, false);
        }
        assert(m_controlSockets.count() == 0 && m_dataSockets.count() == 0);

        m_unfinishedConnections.clear();
        m_platformSocketMap.clear();
    }

    void TCPConnectionSystem::clearMessageQueue()
    {
        PlatformGuard g(m_mainLock);
        m_outMessagesToSend.clear();
    }

    void TCPConnectionSystem::sendAllMessagesInQueue()
    {
        // write queued messages
        Vector<OutMessage> localOutMessagesToSend;
        {
            // swap message queue into local var
            PlatformGuard g(m_mainLock);
            localOutMessagesToSend.swap(m_outMessagesToSend);
        }

        sendAllOutMessages(localOutMessagesToSend);

        localOutMessagesToSend.clear();
    }

    bool TCPConnectionSystem::sendAliveMessages()
    {
        const auto now = std::chrono::steady_clock::now();
        OutMessage aliveMsg(EConnectionType_OrderedControlMessages, EMessageId_Alive);
        Vector<SocketInfo> brokenConnections;

        auto checkSendAlive = [&](SocketInfo& si) {
                                  if (si.lastSent + AliveInterval < now)
                                  {
                                      if (!sendMessageToSocket(si, aliveMsg))
                                      {
                                          LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendAliveMessages: failed to send alive message to " << si.participant);
                                          brokenConnections.push_back(si);
                                      }
                                  }
                              };

        for (auto& p : m_platformSocketMap)
            checkSendAlive(p.value);
        if (!m_isDaemon)
            checkSendAlive(m_daemonSocket);

        return removeConnectionsBySocket(brokenConnections);
    }

    bool TCPConnectionSystem::checkConnectionsAlive()
    {
        const auto now = std::chrono::steady_clock::now();
        Vector<SocketInfo> brokenConnections;

        auto checkAlive = [&](SocketInfo& si) {
                                if (si.lastReceived + AliveIntervalTimeout < now)
                                {
                                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::checkConnectionsAlive: alive message from " <<
                                             si.participant.toString() << " too old. lastReceived " <<
                                             (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - si.lastReceived).count()) << "ms ago, expected alive " <<
                                             (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - si.lastReceived - AliveInterval).count()) << "ms ago");
                                    brokenConnections.push_back(si);
                                }
                            };

        for (auto& p : m_platformSocketMap)
            checkAlive(p.value);
        if (!m_isDaemon)
            checkAlive(m_daemonSocket);

        return removeConnectionsBySocket(brokenConnections);
    }

    bool TCPConnectionSystem::removeConnectionsBySocket(const Vector<SocketInfo>& infos)
    {
        // close all if connection to daemon gone
        if (std::any_of(infos.begin(), infos.end(), [&](const SocketInfo& si) { return si.socket == m_daemonSocket.socket; }))
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::removeConnectionsBySocket: connection to daemon lost; disconnect all");
            closeAndCleanupAllConnections();
            return false;
        }

        for (const auto& si : infos)
        {
            if (si.participant.isInvalid())
            {
                // must be new connection before first hello
                assert(m_newlyAcceptedSockets.contains(si.socket));
                dropConnectionToNewlyAcceptedSocket(*si.socket);
            }
            else
            {
                // check if still known, otherwise might try to remove it twice when alive timeout on control and data socket at the same time
                if (m_knownParticipantAddresses.contains(si.participant))
                {
                    removeKnownParticipant(si.participant, true);
                }
            }
        }

        return true;
    }

    void TCPConnectionSystem::setReadyToSendMessages(bool state)
    {
        PlatformGuard g(m_mainLock);
        m_readyToSend = state;
    }

    void TCPConnectionSystem::handleLogRequests()
    {
        if (m_requestLogConnectionInfo)
        {
            m_requestLogConnectionInfo = false;
            LOG_INFO_F(CONTEXT_COMMUNICATION,
                       ([&](StringOutputStream& sos) {
                            sos << "TCPConnectionSystem:\n";
                            sos << "  Self: " << m_participantAddress.getParticipantName() << " / " << m_participantAddress.getParticipantId() << "\n";
                            sos << "  Protocol version: " << m_protocolVersion << "\n";
                            if (m_isDaemon)
                            {
                                sos << "Participants known to daemon:\n";
                                for (const auto& p : m_knownParticipantAddresses)
                                {
                                    sos << "  " << p.key << " / " << p.value.getParticipantName() << " at " << p.value.getIp() << ":" << p.value.getPort() << " : ";
                                    if (m_controlSockets.contains(p.key))
                                        sos << "connected";
                                    else
                                        sos << "not connected";
                                    sos << "\n";
                                }
                            }
                            else
                            {
                                sos << "  Connected to Daemon: " << (m_daemonSocket.socket != nullptr) << "\n";
                                sos << "Known participants:\n";
                                for (const auto& p : m_knownParticipantAddresses)
                                {
                                    if (p.key != m_participantAddress.getParticipantId())
                                    {
                                        sos << "  " << p.key << " / " << p.value.getParticipantName() << " at " << p.value.getIp() << ":" << p.value.getPort() << " : ";
                                        if (m_controlSockets.contains(p.key) && m_dataSockets.contains(p.key))
                                            sos << "connected";
                                        else
                                            sos << "not connected";
                                        sos << "\n";
                                    }
                                }
                            }
                        }));
        }

        if (m_requestLogMessageForPeriodicLog)
        {
            m_requestLogMessageForPeriodicLog = false;
            LOG_INFO_F(CONTEXT_PERIODIC,
                       ([&](StringOutputStream& sos) {
                            sos << "Connected Participant(s):";

                            bool first = true;
                            for (const auto& socketMapIt : m_controlSockets)
                            {
                                if (m_dataSockets.contains(socketMapIt.key))
                                {
                                    if (first)
                                    {
                                        first = false;
                                    }
                                    else
                                    {
                                        sos << ",";
                                    }
                                    sos << " " << socketMapIt.key;
                                }
                            }
                        }));
        }
    }

    void TCPConnectionSystem::logConnectionInfo()
    {
        m_requestLogConnectionInfo = true;
        m_socketManager.interruptWaitCall();
    }

    void TCPConnectionSystem::triggerLogMessageForPeriodicLog()
    {
        m_requestLogMessageForPeriodicLog = true;
        m_socketManager.interruptWaitCall();
    }
}
