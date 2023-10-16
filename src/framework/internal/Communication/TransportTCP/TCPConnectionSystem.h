//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Communication/TransportCommon/ICommunicationSystem.h"
#include "internal/Communication/TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Communication/TransportTCP/NetworkParticipantAddress.h"
#include "internal/Communication/TransportTCP/EMessageId.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/Communication/TransportTCP/AsioWrapper.h"
#include <deque>
#include <utility>


namespace ramses::internal
{
    class StatisticCollectionFramework;
    class BinaryInputStream;

    class TCPConnectionSystem final : public Runnable, public ICommunicationSystem
    {
    public:
        TCPConnectionSystem(NetworkParticipantAddress  participantAddress, uint32_t protocolVersion, NetworkParticipantAddress  daemonAddress, bool pureDaemon,
                            PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                            std::chrono::milliseconds aliveInterval, std::chrono::milliseconds aliveTimeout);
        ~TCPConnectionSystem() override;

        static Guid GetDaemonId();

        bool connectServices() override;
        bool disconnectServices() override;

        IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() override;

        // scene
        bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes, EFeatureLevel featureLevel) override;
        bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes) override;
        bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes, EFeatureLevel featureLevel) override;

        bool sendSubscribeScene(const Guid& to, const SceneId& sceneId) override;
        bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId) override;

        bool sendInitializeScene(const Guid& to, const SceneId& sceneId) override;
        bool sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer) override;

        bool sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<std::byte>& data) override;

        // set service handlers
        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;

        // log triggers
        void logConnectionInfo() override;
        void triggerLogMessageForPeriodicLog() override;

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

        struct OutMessage
        {
            OutMessage(const Guid& to_, EMessageId messageType_)
                : OutMessage(std::vector<Guid>({to_}), messageType_)
            {
                assert(to_.isValid());
            }

            OutMessage(std::vector<Guid>  to_, EMessageId messageType_)
                : to(std::move(to_))
                , messageType(messageType_)
            {
                stream << static_cast<uint32_t>(0)  // fill in size later
                       << static_cast<uint32_t>(0)  // fill in protocol version later
                       << static_cast<uint32_t>(messageType);
            }

            // TODO(tobias) make move only in c++14
            OutMessage(OutMessage&&) noexcept = default;
            OutMessage(const OutMessage&) = default;
            OutMessage& operator=(const OutMessage&) = default;

            std::vector<Guid> to;
            EMessageId messageType;
            BinaryOutputStream stream;
        };

        struct Participant
        {
            Participant(NetworkParticipantAddress address_, asio::io_service& io_,
                        EParticipantType type_, EParticipantState state_);
            ~Participant();

            NetworkParticipantAddress address;
            asio::ip::tcp::socket socket;
            asio::steady_timer connectTimer;

            std::deque<OutMessage> outQueue;
            std::vector<std::byte> currentOutBuffer;

            uint32_t lengthReceiveBuffer;
            std::vector<std::byte> receiveBuffer;

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

        void run() override;

        void doConnect(const ParticipantPtr& pp);
        void sendConnectionDescriptionOnNewConnection(const ParticipantPtr& pp);
        void doSendQueuedMessage(const ParticipantPtr& pp);
        void doTrySendAliveMessage(const ParticipantPtr& pp);
        void doReadHeader(const ParticipantPtr& pp);
        void doReadContent(const ParticipantPtr& pp);

        bool openAcceptor();
        void doAcceptIncomingConnections();

        void sendMessageToParticipant(const ParticipantPtr& pp, OutMessage msg);
        void removeParticipant(const ParticipantPtr& pp, bool reconnectWithBackoff = false);
        void addNewParticipantByAddress(const NetworkParticipantAddress& address);
        void initializeNewlyConnectedParticipant(const ParticipantPtr& pp);
        void handleReceivedMessage(const ParticipantPtr& pp);
        bool postMessageForSending(OutMessage msg);
        void updateLastReceivedTime(const ParticipantPtr& pp);
        void sendConnectorAddressExchangeMessagesForNewParticipant(const ParticipantPtr& newPp);
        void triggerConnectionUpdateNotification(Guid participant, EConnectionStatus status);

        void handleConnectionDescriptionMessage(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleConnectorAddressExchange(const ParticipantPtr& pp, BinaryInputStream& stream);

        void handleSubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleUnsubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleCreateScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleSceneUpdate(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handlePublishScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleUnpublishScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleSceneNotAvailable(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleRendererEvent(const ParticipantPtr& pp, BinaryInputStream& stream);

        static const char* EnumToString(EParticipantState e);
        static const char* EnumToString(EParticipantType e);

        const NetworkParticipantAddress m_participantAddress;
        const uint32_t m_protocolVersion;
        const NetworkParticipantAddress m_daemonAddress;
        const bool m_actAsDaemon;
        const bool m_hasOtherDaemon;
        const EParticipantType m_participantType;
        const std::chrono::milliseconds m_aliveInterval;
        const std::chrono::milliseconds m_aliveIntervalTimeout;

        PlatformLock& m_frameworkLock;
        PlatformThread m_thread;
        StatisticCollectionFramework& m_statisticCollection;

        ConnectionStatusUpdateNotifier m_ramsesConnectionStatusUpdateNotifier;
        std::vector<Guid> m_connectedParticipantsForBroadcasts;

        ISceneProviderServiceHandler* m_sceneProviderHandler;
        ISceneRendererServiceHandler* m_sceneRendererHandler;

        std::unique_ptr<RunState>     m_runState;
        HashSet<ParticipantPtr>       m_connectingParticipants;
        HashMap<Guid, ParticipantPtr> m_establishedParticipants;
    };
}
