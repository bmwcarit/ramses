//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TCPCONNECTIONSYSTEM_H
#define RAMSES_TCPCONNECTIONSYSTEM_H

#include "TransportTCP/SocketManager.h"
#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "NetworkParticipantAddress.h"

#include "PlatformAbstraction/PlatformThread.h"
#include "Collections/HashSet.h"
#include "Utils/ScopedPointer.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "TransportTCP/EConnectionType.h"
#include "TransportTCP/EMessageId.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"

namespace ramses_internal
{
    class StatisticCollectionFramework;

    class TCPConnectionSystem final : public Runnable, public ICommunicationSystem
    {
    public:
        TCPConnectionSystem(const NetworkParticipantAddress& participantAddress, UInt32 protocolVersion, Bool isDaemon, const NetworkParticipantAddress& daemonAddress,
            PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection);
        ~TCPConnectionSystem() override;

        virtual bool connectServices() override;
        virtual bool disconnectServices() override;

        virtual IConnectionStatusUpdateNotifier& getConnectionStatusUpdateNotifier() override;

        static Guid GetDaemonId();

        // message limits configuration
        virtual CommunicationSendDataSizes getSendDataSizes() const override final;
        virtual void setSendDataSizes(const CommunicationSendDataSizes& sizes) override final;

        // resource
        virtual bool sendRequestResources(const Guid& to, const ResourceContentHashVector& resources) override;
        virtual bool sendResourcesNotAvailable(const Guid& to, const ResourceContentHashVector& resources) override;
        virtual bool sendResources(const Guid& to, const ManagedResourceVector& managedResources) override;

        // scene
        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes) override;
        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes) override;
        virtual bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes) override;

        virtual bool sendSubscribeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendSceneNotAvailable(const Guid& to, const SceneId& sceneId) override;

        virtual bool sendInitializeScene(const Guid& to, const SceneInfo& sceneInfo) override;
        virtual uint64_t sendSceneActionList(const Guid& to, const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& actionListCounter) override;

        // set service handlers
        void setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler) override;
        void setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler) override;
        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;

        virtual void logConnectionInfo() const override;
        virtual void triggerLogMessageForPeriodicLog() const override;

    private:
        struct OutMessage
        {
            OutMessage(EConnectionType connectionType_, EMessageId messageType_, const Guid& to_ = Guid(false))
                : to(to_)
                , connectionType(connectionType_)
                , messageType(messageType_)
                , stream(new BinaryOutputStream())
            {
                *stream << static_cast<UInt32>(0) << static_cast<UInt32>(0);
            }

            Guid to;
            EConnectionType connectionType;
            EMessageId messageType;
            std::unique_ptr<BinaryOutputStream> stream;
        };

        struct InMessage
        {
            InMessage(EMessageId type_, UInt32 dataSize)
                : sender(false)
                , type(type_)
                , data(dataSize)
                , stream(data.data())
            {}

            Guid sender;
            EMessageId type;
            HeapArray<UInt8> data;
            BinaryInputStream stream;
        };

        typedef Pair<Guid, PlatformSocket*> GuidSocketPair;

        static const UInt32 resourceDataSizeSimilarToOtherStacks = 1300000;
        static constexpr int32_t socketConnectTimeoutMs = 1500;

        bool sendMessage(OutMessage&& message);

        virtual void run() override;
        virtual void cancel() override;

        Bool setupListener();
        void cleanupListener();
        void trackServerSocket();
        void connectToDaemon();

        void acceptIncomingConnection(PlatformServerSocket& serverSocket);
        Bool sendConnectionDescriptionMessage(PlatformSocket& socket, const NetworkParticipantAddress& to, const EConnectionType& type);
        Bool sendAddressExchangeMessage(PlatformSocket& socket, const NetworkParticipantAddress& address, const Guid& to) const;
        void trackSocket(PlatformSocket& streamSocket);
        void socketHasData(PlatformSocket& socket);
        Bool connectWithType(const NetworkParticipantAddress& address, EConnectionType type, HashMap<Guid, PlatformSocket*>& socketMap);
        Bool handleMessage(const Guid& participantId, PlatformSocket& socket, InMessage& message);
        void handleConnectorAddressExchangeMessage(InMessage& message);
        Bool handleConnectionDescriptionMessage(PlatformSocket& socket, InMessage& message);
        Bool shouldTryConnectTo(const Guid& otherId) const;
        bool dispatchReceivedMessage(InMessage& message);
        void tryConnectToOthers();

        void sendAllOutMessages(const Vector<OutMessage>& messagesToSend);
        void sendUnicastMessageToSocket(const OutMessage& message, Vector<Guid>& brokenConnections);
        void sendBroadcastMessageToSockets(const OutMessage& message, Vector<Guid>& brokenConnections);
        Bool sendMessageToSocket(PlatformSocket& socket, const OutMessage& message) const;

        std::unique_ptr<InMessage> receiveMessageFromSocket(PlatformSocket& socket);
        bool readDataFromSocket(PlatformSocket& socket, char* buffer, UInt32 size);

        void removeKnownParticipant(const Guid& idRef);
        void dropConnectionToNewlyAcceptedSocket(PlatformSocket& socket);
        void addNewParticipantIfUnknown(const NetworkParticipantAddress& address);
        Bool sendAddressExchangeForNewParticipant(PlatformSocket& newSocket, const NetworkParticipantAddress& newAddress);
        void closeAndCleanupAllConnections();

        void clearMessageQueue();
        void sendAllMessagesInQueue();

        void setReadyToSendMessages(bool state);

        // receive methods
        void handleSceneSubscription(InMessage& message);
        void handleSceneUnsubscription(InMessage& message);

        void handleCreateScene(InMessage& message);
        void handleSceneActionList(InMessage& message);
        void handleScenePublication(InMessage& message);
        void handleSceneUnpublication(InMessage& message);
        void handleSceneNotAvailable(InMessage& message);

        void handleResourceRequest(InMessage& message);
        void handleIncomingResource(InMessage& message);
        void handleResourcesNotAvailable(InMessage& message);

        SocketManager m_socketManager;
        const NetworkParticipantAddress m_participantAddress;
        const UInt32 m_protocolVersion;
        const Bool m_isDaemon;
        const NetworkParticipantAddress m_daemonAddress;
        Bool m_isRunning;
        CommunicationSendDataSizes m_sendDataSizes;

        IResourceConsumerServiceHandler* m_resourceConsumerHandler;
        IResourceProviderServiceHandler* m_resourceProviderHandler;
        ISceneProviderServiceHandler* m_sceneProviderHandler;
        ISceneRendererServiceHandler* m_sceneRendererHandler;
        PlatformLock& m_frameworkLock;
        PlatformLock m_startStopLock;
        PlatformThread m_mainLoop;
        ConnectionStatusUpdateNotifier m_connectionStatusUpdateNotifier;

        PlatformLock m_mainLock;
        ScopedPointer<PlatformServerSocket> m_serverSocket;
        ScopedPointer<PlatformSocket> m_daemonSocket;
        Vector<OutMessage> m_outMessagesToSend;
        Vector<PlatformSocket*> m_newlyAcceptedSockets;

        HashSet<Guid> m_unfinishedConnections;
        HashMap<Guid, NetworkParticipantAddress> m_knownParticipantAddresses;
        HashMap<Guid, PlatformSocket*> m_controlSockets;
        HashMap<Guid, PlatformSocket*> m_dataSockets;
        HashMap<PlatformSocket*, GuidSocketPair> m_platformSocketMap;
        bool m_readyToSend;

        StatisticCollectionFramework& m_statisticCollection;
    };
}

#endif
