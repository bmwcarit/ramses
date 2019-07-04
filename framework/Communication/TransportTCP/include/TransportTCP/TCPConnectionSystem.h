//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATION_TCPCONNECTIONSYSTEM_H
#define RAMSES_COMMUNICATION_TCPCONNECTIONSYSTEM_H

#include "TransportCommon/ICommunicationSystem.h"
#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "TransportTCP/NetworkParticipantAddress.h"
#include "TransportTCP/EMessageId.h"
#include "Utils/BinaryOutputStream.h"
#include "Collections/HashSet.h"
#include "Collections/HashMap.h"
#include "TransportTCP/AsioWrapper.h"
#include <deque>


namespace ramses_internal
{
    class StatisticCollectionFramework;
    class BinaryInputStream;

    class TCPConnectionSystem final : public Runnable, public ICommunicationSystem
    {
    public:
        TCPConnectionSystem(const NetworkParticipantAddress& participantAddress, UInt32 protocolVersion, const NetworkParticipantAddress& daemonAddress, bool pureDaemon,
                            PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                            std::chrono::milliseconds aliveInterval, std::chrono::milliseconds aliveTimeout);
        ~TCPConnectionSystem();

        static Guid GetDaemonId();

        virtual bool connectServices() override;
        virtual bool disconnectServices() override;

        virtual IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() override;
        virtual IConnectionStatusUpdateNotifier& getDcsmConnectionStatusUpdateNotifier() override;

        // message limits configuration
        virtual CommunicationSendDataSizes getSendDataSizes() const override;
        virtual void setSendDataSizes(const CommunicationSendDataSizes& sizes) override;

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

        // dcsm client -> renderer
        virtual bool sendDcsmBroadcastOfferContent(ContentID contentID, Category) override;
        virtual bool sendDcsmOfferContent(const Guid& to, ContentID contentID, Category) override;
        virtual bool sendDcsmContentReady(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual bool sendDcsmContentFocusRequest(const Guid& to, ContentID contentID) override;
        virtual bool sendDcsmBroadcastRequestStopOfferContent(ContentID contentID) override;
        virtual bool sendDcsmBroadcastForceStopOfferContent(ContentID contentID) override;

        // dcsm renderer -> client
        virtual bool sendDcsmCanvasSizeChange(const Guid& to, ContentID contentID, SizeInfo sizeinfo, AnimationInformation ai) override;
        virtual bool sendDcsmContentStateChange(const Guid& to, ContentID contentID, EDcsmState status, SizeInfo si, AnimationInformation ai) override;

        // set service handlers
        void setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler) override;
        void setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler) override;
        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;
        void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler) override;
        void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler) override;

        // log triggers
        virtual void logConnectionInfo() override;
        virtual void triggerLogMessageForPeriodicLog() override;

    private:
        enum class EParticipantState
        {
            Invalid,
            Connecting,
            WaitingForHello,
            Established,
        };

        enum class EParticipantType
        {
            Client,
            Daemon,
            PureDaemon
        };

        enum class EQueueType
        {
            Normal,
            Priority
        };

        struct OutMessage
        {
            OutMessage(const Guid& to_, EMessageId messageType_)
                : to(to_)
                , messageType(messageType_)
            {
                stream << static_cast<uint32_t>(0) << static_cast<uint32_t>(messageType);
            }

            // TODO(tobias) make move only in c++14
            OutMessage(OutMessage&&) = default;
            OutMessage(const OutMessage&) = default;
            OutMessage& operator=(const OutMessage&) = default;

            Guid to;
            EMessageId messageType;
            BinaryOutputStream stream;
        };

        struct Participant
        {
            Participant(const NetworkParticipantAddress& address_, asio::io_service& io_,
                        EParticipantType type_, EParticipantState state_);
            ~Participant();

            NetworkParticipantAddress address;
            asio::ip::tcp::socket socket;
            asio::steady_timer connectTimer;

            std::deque<OutMessage> outQueueNormal;
            std::deque<OutMessage> outQueuePrio;
            std::vector<char> currentOutBuffer;

            uint32_t lengthReceiveBuffer;
            std::vector<char> receiveBuffer;

            std::chrono::steady_clock::time_point lastSent;
            asio::steady_timer sendAliveTimer;
            std::chrono::steady_clock::time_point lastReceived;
            asio::steady_timer checkReceivedAliveTimer;

            EParticipantType type;
            EParticipantState state;
        };
        using ParticipantPtr = std::shared_ptr<Participant>;

        struct RunState
        {
            RunState();

            asio::io_service        m_io;
            asio::ip::tcp::acceptor m_acceptor;
            asio::ip::tcp::socket   m_acceptorSocket;
        };
        using RunStatePtr = std::shared_ptr<RunState>;

        virtual void run() override;

        void doConnect(const ParticipantPtr& pp);
        void sendConnectionDescriptionOnNewConnection(const ParticipantPtr& pp);
        void doSendQueuedMessage(const ParticipantPtr& pp);
        void doTrySendAliveMessage(const ParticipantPtr& pp);
        void doReadHeader(const ParticipantPtr& pp);
        void doReadContent(const ParticipantPtr& pp);

        bool openAcceptor();
        void doAcceptIncomingConnections();

        void sendMessageToParticipant(const ParticipantPtr& pp, OutMessage msg);
        void removeParticipant(const ParticipantPtr& pp);
        void addNewParticipantByAddress(const NetworkParticipantAddress& address);
        void initializeNewlyConnectedParticipant(const ParticipantPtr& pp);
        void handleReceivedMessage(const ParticipantPtr& pp);
        bool postMessageForSending(OutMessage msg, bool hasPrio);
        void updateLastReceivedTime(const ParticipantPtr& pp);
        void sendConnectorAddressExchangeMessagesForNewParticipant(const ParticipantPtr& newPp);

        void handleConnectionDescriptionMessage(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleConnectorAddressExchange(const ParticipantPtr& pp, BinaryInputStream& stream);

        void handleSubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleUnsubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleCreateScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleSceneActionList(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handlePublishScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleUnpublishScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleSceneNotAvailable(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleRequestResources(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleTransferResources(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleResourcesNotAvailable(const ParticipantPtr& pp, BinaryInputStream& stream);

        void handleDcsmCanvasSizeChange(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmContentStatusChange(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmRegisterContent(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmContentAvailable(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmCategoryContentSwitchRequest(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmRequestUnregisterContent(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmForceStopOfferContent(const ParticipantPtr& pp, BinaryInputStream& stream);

        static const char* EnumToString(EParticipantState e);
        static const char* EnumToString(EParticipantType e);

        const NetworkParticipantAddress m_participantAddress;
        const UInt32 m_protocolVersion;
        const NetworkParticipantAddress m_daemonAddress;
        const bool m_actAsDaemon;
        const bool m_hasOtherDaemon;
        const EParticipantType m_participantType;
        const std::chrono::milliseconds m_aliveInterval;
        const std::chrono::milliseconds m_aliveIntervalTimeout;

        CommunicationSendDataSizes m_sendDataSizes;

        PlatformLock& m_frameworkLock;
        PlatformThread m_thread;
        StatisticCollectionFramework& m_statisticCollection;

        ConnectionStatusUpdateNotifier m_ramsesConnectionStatusUpdateNotifier;
        ConnectionStatusUpdateNotifier m_dcsmConnectionStatusUpdateNotifier;

        IResourceConsumerServiceHandler* m_resourceConsumerHandler;
        IResourceProviderServiceHandler* m_resourceProviderHandler;
        ISceneProviderServiceHandler* m_sceneProviderHandler;
        ISceneRendererServiceHandler* m_sceneRendererHandler;
        IDcsmProviderServiceHandler* m_dcsmProviderHandler;
        IDcsmConsumerServiceHandler* m_dcsmConsumerHandler;

        RunStatePtr m_runState;
        HashSet<ParticipantPtr>       m_connectingParticipants;
        HashMap<Guid, ParticipantPtr> m_establishedParticipants;
    };
}

#endif
