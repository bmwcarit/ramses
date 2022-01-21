//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportTCP/TCPConnectionSystem.h"

#include "Scene/SceneActionCollection.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/RawBinaryOutputStream.h"
#include "Utils/StatisticCollection.h"
#include "Utils/LogMacros.h"
#include <thread>
#include "Components/CategoryInfo.h"
#include "TransportCommon/ISceneUpdateSerializer.h"

namespace ramses_internal
{
    static const constexpr uint32_t ResourceDataSize = 300000;
    static const constexpr uint32_t SceneActionDataSize = 300000;

    TCPConnectionSystem::TCPConnectionSystem(const NetworkParticipantAddress& participantAddress,
                                                     uint32_t protocolVersion,
                                                     const NetworkParticipantAddress& daemonAddress,
                                                     bool pureDaemon,
                                                     PlatformLock& frameworkLock,
                                                     StatisticCollectionFramework& statisticCollection,
                                                     std::chrono::milliseconds aliveInterval,
                                                     std::chrono::milliseconds aliveTimeout)
        : m_participantAddress(participantAddress)
        , m_protocolVersion(protocolVersion)
        , m_daemonAddress(daemonAddress)
        , m_actAsDaemon(m_participantAddress.getPort() != 0)
        , m_hasOtherDaemon(!(m_daemonAddress.getPort() == m_participantAddress.getPort() &&
                             m_daemonAddress.getIp() == m_participantAddress.getIp()) &&
                           m_daemonAddress.getPort() != 0)
        , m_participantType(m_actAsDaemon ?
                            (pureDaemon ? EParticipantType::PureDaemon
                             : EParticipantType::Daemon)
                            : EParticipantType::Client)
        , m_aliveInterval(aliveInterval)
        , m_aliveIntervalTimeout(aliveTimeout)
        , m_frameworkLock(frameworkLock)
        , m_thread("R_TCP_ConnSys")
        , m_statisticCollection(statisticCollection)
        , m_ramsesConnectionStatusUpdateNotifier(m_participantAddress.getParticipantName().stdRef(), CONTEXT_COMMUNICATION, "ramses", frameworkLock)
        , m_dcsmConnectionStatusUpdateNotifier(m_participantAddress.getParticipantName().stdRef(), CONTEXT_COMMUNICATION, "dcsm", frameworkLock) // dcsm is not separated here, no extra context
        , m_sceneProviderHandler(nullptr)
        , m_sceneRendererHandler(nullptr)
        , m_dcsmProviderHandler(nullptr)
        , m_dcsmConsumerHandler(nullptr)
    {
    }

    TCPConnectionSystem::~TCPConnectionSystem()
    {
        if (m_runState)
            disconnectServices();
    }

    Guid TCPConnectionSystem::GetDaemonId()
    {
        static Guid guid("BBBC4FDF-44E4-4EE9-8649-E0139E8AE986");
        return guid;
    }

    bool TCPConnectionSystem::connectServices()
    {
        LOG_INFO_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                               sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectServices: "
                                                   << m_participantAddress.getParticipantId() << "/" << m_participantAddress.getParticipantName()
                                                   << " at " << m_participantAddress.getIp() << ":" << m_participantAddress.getPort()
                                                   << ", type " << EnumToString(m_participantType)
                                                   << ", aliveInterval " << m_aliveInterval.count() << "ms, aliveTimeout " << m_aliveIntervalTimeout.count() << "ms";
                                               if (m_hasOtherDaemon)
                                                   sos << ", other daemon at " << m_daemonAddress.getIp() << ":" << m_daemonAddress.getPort();
                                           }));
        if (m_aliveIntervalTimeout < m_aliveInterval + std::chrono::milliseconds{100})
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << "): Alive timeout very low, expect issues " <<
                     "(alive " << m_aliveInterval.count() << ", timeout " << m_aliveIntervalTimeout.count() << ")");
        }

        PlatformGuard guard(m_frameworkLock);
        if (m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectServices: called more than once");
            return false;
        }

        m_runState = std::make_unique<RunState>();
        m_thread.start(*this);

        return true;
    }

    bool TCPConnectionSystem::disconnectServices()
    {
        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::disconnectServices");

        PlatformGuard guard(m_frameworkLock);
        if (!m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::disconnectServices: called without being connected");
            return false;
        }

        // Signal context to exit run and join thread
        m_runState->m_io.stop();
        {
            // must release lock to let things finish in thread
            m_frameworkLock.unlock();
            m_thread.join();
            m_frameworkLock.lock();
        }
        m_runState.reset();

        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::disconnectServices: done");
        return true;
    }

    IConnectionStatusUpdateNotifier& TCPConnectionSystem::getRamsesConnectionStatusUpdateNotifier()
    {
        return m_ramsesConnectionStatusUpdateNotifier;
    }

    IConnectionStatusUpdateNotifier& TCPConnectionSystem::getDcsmConnectionStatusUpdateNotifier()
    {
        return m_dcsmConnectionStatusUpdateNotifier;
    }

    void TCPConnectionSystem::setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler)
    {
        m_sceneProviderHandler = handler;
    }

    void TCPConnectionSystem::setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler)
    {
        m_sceneRendererHandler = handler;
    }

    void TCPConnectionSystem::setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler)
    {
        m_dcsmProviderHandler = handler;
    }

    void TCPConnectionSystem::setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler)
    {
        m_dcsmConsumerHandler = handler;
    }

    void TCPConnectionSystem::run()
    {
        // set up connection acceptor
        if (!openAcceptor())
            return;
        doAcceptIncomingConnections();

        // initiate connection to daemon (when not self)
        if (m_daemonAddress.getPort() != 0 && m_hasOtherDaemon)
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::run: Initiate connection to daemon at " << m_daemonAddress.getIp() << ":" << m_daemonAddress.getPort());

            auto daemonPp = std::make_shared<Participant>(m_daemonAddress, m_runState->m_io, EParticipantType::Daemon, EParticipantState::Connecting);
            m_connectingParticipants.put(daemonPp);
            doConnect(daemonPp);
        }

        m_runState->m_io.run();

        for (const auto& pp : m_establishedParticipants)
        {
            if (pp.value->type != EParticipantType::PureDaemon)
            {
                triggerConnectionUpdateNotification(pp.value->address.getParticipantId(), EConnectionStatus_NotConnected);
            }
        }

        m_runState->m_acceptor.close();
        m_runState->m_acceptorSocket.close();
        m_connectingParticipants.clear();
        m_establishedParticipants.clear();
    }

    bool TCPConnectionSystem::openAcceptor()
    {
        assert(!m_runState->m_acceptor.is_open());

        asio::error_code e;
        m_runState->m_acceptor.open(asio::ip::tcp::v4(), e);
        if (e)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::openAcceptor: open failed. " << e.message().c_str());
            return false;
        }

        m_runState->m_acceptor.set_option(asio::socket_base::reuse_address(true));

        // always accept connections from all to also work on uncommon setups (could be changed to use m_participantAddress.getIp())
        m_runState->m_acceptor.bind(asio::ip::tcp::endpoint(asio::ip::address::from_string("0.0.0.0"), m_participantAddress.getPort()), e);
        if (e)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::openAcceptor: bind failed. " << e.message().c_str());
            return false;
        }

        m_runState->m_acceptor.listen(asio::socket_base::max_connections, e);
        if (e)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::openAcceptor: listen failed. " << e.message().c_str());
            return false;
        }

        const auto endpoint = m_runState->m_acceptor.local_endpoint();
        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::openAcceptor: Listening for connections on " << endpoint.address().to_string().c_str() << ":" << endpoint.port());

        return true;
    }

    void TCPConnectionSystem::doAcceptIncomingConnections()
    {
        m_runState->m_acceptor.async_accept(m_runState->m_acceptorSocket,
                                [this](asio::error_code e) {
                                    if (e)
                                    {
                                        LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doAcceptIncomingConnections: accept failed. " << e.message().c_str());
                                        doAcceptIncomingConnections();
                                        // TODO: not sure if correct response or is really recoverable (close + reopen acceptor?)
                                    }
                                    else
                                    {
                                        auto remoteEp = m_runState->m_acceptorSocket.remote_endpoint();
                                        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doAcceptIncomingConnections: Accepted new connection from " <<
                                                 remoteEp.address().to_string().c_str() << ":" << remoteEp.port());

                                        // create new participant
                                        auto pp = std::make_shared<Participant>(NetworkParticipantAddress(), m_runState->m_io, EParticipantType::Client, EParticipantState::WaitingForHello);
                                        pp->socket = std::move(m_runState->m_acceptorSocket);
                                        m_connectingParticipants.put(pp);
                                        m_runState->m_acceptorSocket = asio::ip::tcp::socket(m_runState->m_io);

                                        initializeNewlyConnectedParticipant(pp);

                                        // accept next connection
                                        doAcceptIncomingConnections();
                                    }
                                });
    }

    void TCPConnectionSystem::doConnect(const ParticipantPtr& pp)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doConnect: Try connect to participant at " << pp->address.getIp() << ":" << pp->address.getPort());

        // convert localhost to an actual ip, no need for dns resolving here
        const char* const ipStr = (pp->address.getIp() == "localhost" ? "127.0.0.1" : pp->address.getIp().c_str());

        // parse with error checking to prevent exception when ip is invalid format
        asio::error_code err;
        const auto asioIp = asio::ip::address::from_string(ipStr, err);
        if (err)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doConnect: Failed to parse ip address '" << pp->address.getIp() << ":" << pp->address.getPort() << "'");
            return;
        }

        asio::ip::tcp::endpoint ep(asioIp, pp->address.getPort());
        std::array<asio::ip::tcp::endpoint, 1> endpointSequence = {ep};
        asio::async_connect(pp->socket, endpointSequence, [this, pp](asio::error_code e, const asio::ip::tcp::endpoint& usedEndpoint) {
                if (e)
                {
                    // connect failed, try again after timeout
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doConnect: Connect to "
                              << pp->address.getIp() << ":" << pp->address.getPort() << " failed. " << e.message().c_str());
                    pp->connectTimer.expires_after(std::chrono::milliseconds{100});
                    pp->connectTimer.async_wait([this, pp](asio::error_code ee) {
                                                    if (ee)
                                                    {
                                                        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doConnect: Connect to "
                                                                  << pp->address.getIp() << ":" << pp->address.getPort() << " failed. Timer canceled");
                                                    }
                                                    else
                                                    {
                                                        doConnect(pp);
                                                    }
                                                });
                }
                else
                {
                    // connected
                    LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doConnect: Established to " << usedEndpoint.address().to_string().c_str() << ":" << usedEndpoint.port());
                    initializeNewlyConnectedParticipant(pp);
                }
            });
    }

    void TCPConnectionSystem::initializeNewlyConnectedParticipant(const ParticipantPtr& pp)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::initializeNewlyConnectedParticipant: " << pp->address.getParticipantId());
        assert(m_connectingParticipants.contains(pp));

        // Set send buffer to resource chunk size to allow maximum one resource chunk to use up send buffer. This
        // is needed to allow high prio data to be sent as fast as possible.
        pp->socket.set_option(asio::socket_base::send_buffer_size{static_cast<int>(ResourceDataSize)});

        // Disable nagle
        pp->socket.set_option(asio::ip::tcp::no_delay{true});
        pp->state = EParticipantState::WaitingForHello;
        pp->lastReceived = std::chrono::steady_clock::now();

        doReadHeader(pp);
        sendConnectionDescriptionOnNewConnection(pp);
    }

    void TCPConnectionSystem::sendMessageToParticipant(const ParticipantPtr& pp, OutMessage msg)
    {
        assert(pp->currentOutBuffer.empty());

        pp->currentOutBuffer = msg.stream.release();
        const uint32_t fullSize = static_cast<uint32_t>(pp->currentOutBuffer.size());

        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendMessageToParticipant: To " << pp->address.getParticipantId() <<
                  ", MsgType " << msg.messageType << ", Size " << fullSize);

        RawBinaryOutputStream s(pp->currentOutBuffer.data(), pp->currentOutBuffer.size());
        const uint32_t remainingSize = fullSize - sizeof(pp->lengthReceiveBuffer);
        s << remainingSize
          << m_protocolVersion;

        asio::async_write(pp->socket, asio::const_buffer(pp->currentOutBuffer.data(), pp->currentOutBuffer.size()),
                          [this, pp](asio::error_code e, std::size_t sentBytes) {
                              if (e)
                              {
                                  LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendMessageToParticipant: Send to "
                                           << pp->address.ParticipantIdentifier::getParticipantId() << "/" << pp->address.ParticipantIdentifier::getParticipantName() <<
                                           " failed. " << e.message().c_str() << ". Remove participant");

                                  removeParticipant(pp);
                              }
                              else
                              {
                                  LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendMessageToParticipant: To " << pp->address.getParticipantId() <<
                                            ", MsgBytes " << pp->currentOutBuffer.size() << ", SentBytes " << sentBytes);

                                  pp->currentOutBuffer.clear();
                                  pp->lastSent = std::chrono::steady_clock::now();

                                  pp->sendAliveTimer.expires_after(m_aliveInterval);
                                  pp->sendAliveTimer.async_wait([this, pp](asio::error_code ee) {
                                                                    if (!ee)
                                                                    {
                                                                        // when timer not canceled
                                                                        doTrySendAliveMessage(pp);
                                                                    }
                                                                });

                                  doSendQueuedMessage(pp);
                              }
                          });
    }

    void TCPConnectionSystem::doSendQueuedMessage(const ParticipantPtr& pp)
    {
        if (pp->currentOutBuffer.empty() && !pp->outQueue.empty())
        {
            OutMessage msg = std::move(pp->outQueue.front());
            pp->outQueue.pop_front();

            sendMessageToParticipant(pp, std::move(msg));
        }
    }

    void TCPConnectionSystem::doTrySendAliveMessage(const ParticipantPtr& pp)
    {
        if (pp->currentOutBuffer.empty())
        {
            assert(pp->outQueue.empty());

            sendMessageToParticipant(pp, OutMessage(std::vector<Guid>(), EMessageId::Alive));
        }
    }

    void TCPConnectionSystem::doReadHeader(const ParticipantPtr& pp)
    {
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doReadHeader: start reading from " << pp->address.getParticipantId());

        asio::async_read(pp->socket,
                         asio::mutable_buffer(&pp->lengthReceiveBuffer, sizeof(pp->lengthReceiveBuffer)),
                         [this, pp](asio::error_code e, size_t readBytes) {
                             if (e)
                             {
                                 LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doReadHeader: read from "
                                          << pp->address.getParticipantId() << " failed (len " << readBytes << "). " << e.message().c_str());
                                 removeParticipant(pp);
                             }
                             else
                             {
                                 LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doReadHeader: done read from "
                                           << pp->address.getParticipantId() << ". Expect " << pp->lengthReceiveBuffer << " bytes");

                                 updateLastReceivedTime(pp);
                                 pp->receiveBuffer.resize(pp->lengthReceiveBuffer);
                                 doReadContent(pp);
                             }
                         });
    }

    void TCPConnectionSystem::doReadContent(const ParticipantPtr& pp)
    {
        asio::async_read(pp->socket,
                         asio::mutable_buffer(pp->receiveBuffer.data(), pp->receiveBuffer.size()),
                         [this, pp](asio::error_code e, size_t readBytes) {
                             if (e)
                             {
                                 LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doReadContent: read from "
                                          << pp->address.getParticipantId() << " failed (len " << readBytes << "). " << e.message().c_str());
                                 removeParticipant(pp);
                             }
                             else
                             {
                                 LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::doReadContent: done read from "
                                           << pp->address.getParticipantId() << ", " << readBytes << " bytes");

                                 m_statisticCollection.statMessagesReceived.incCounter(1);

                                 updateLastReceivedTime(pp);
                                 handleReceivedMessage(pp);
                                 doReadHeader(pp);
                             }
                         });
    }

    void TCPConnectionSystem::updateLastReceivedTime(const ParticipantPtr& pp)
    {
        pp->lastReceived = std::chrono::steady_clock::now();
        pp->checkReceivedAliveTimer.expires_after(m_aliveIntervalTimeout);
        pp->checkReceivedAliveTimer.async_wait([this, pp, originalLastReceived = pp->lastReceived](asio::error_code e) {
                                                   if (!e)
                                                   {
                                                       LOG_WARN_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                           const auto now = std::chrono::steady_clock::now();
                                                           const auto lastRecvMs = std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - originalLastReceived).count();
                                                           const auto expectedMs = std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - originalLastReceived - m_aliveInterval).count();
                                                           const auto expectLatest = std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - originalLastReceived - m_aliveIntervalTimeout).count();

                                                           sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::updateLastReceivedTime: alive message from " <<
                                                               pp->address.ParticipantIdentifier::getParticipantId() << " too old. lastReceived " <<
                                                               lastRecvMs << "ms ago, expected alive " << expectedMs << "ms ago, latest " << expectLatest << "ms ago";
                                                       }));
                                                       removeParticipant(pp);
                                                   }
                                               });
    }

    void TCPConnectionSystem::removeParticipant(const ParticipantPtr& pp, bool reconnectWithBackoff)
    {
        if (pp->state == EParticipantState::Invalid)
            return;

        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::removeParticipant: " << pp->address.getParticipantId() << "/" << pp->address.getParticipantName() <<
                 ", state " << EnumToString(pp->state));

        if (pp->state == EParticipantState::Established && pp->type != EParticipantType::PureDaemon)
        {
            triggerConnectionUpdateNotification(pp->address.getParticipantId(), EConnectionStatus_NotConnected);
        }

        // tear down and cancel everything
        pp->socket.close();
        pp->connectTimer.cancel();
        pp->sendAliveTimer.cancel();
        pp->checkReceivedAliveTimer.cancel();
        pp->state = EParticipantState::Invalid;

        // remove from sets
        m_connectingParticipants.remove(pp);
        if (!pp->address.getParticipantId().isInvalid())
        {
            m_establishedParticipants.remove(pp->address.getParticipantId());
        }

        // check if should be tried again
        if (reconnectWithBackoff)
        {
            const std::chrono::milliseconds backoffTime{2000};
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::removeParticipant: will delay reconnect by " << backoffTime.count() << "ms");
            pp->connectTimer.expires_after(backoffTime);
            pp->connectTimer.async_wait([this, pp](asio::error_code ee) {
                if (ee)
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::removeParticipant: Backoff timer got canceled.");
                else
                    addNewParticipantByAddress(pp->address);
            });
        }
        else
            addNewParticipantByAddress(pp->address);
    }

    const char* TCPConnectionSystem::EnumToString(EParticipantState e)
    {
        switch (e)
        {
        case EParticipantState::Invalid: return "Invalid";
        case EParticipantState::Connecting: return "Connecting";
        case EParticipantState::WaitingForHello: return "WaitingForHello";
        case EParticipantState::Established: return "Established";
        };
        return "<Unknown>";
    }

    const char* TCPConnectionSystem::EnumToString(EParticipantType e)
    {
        switch (e)
        {
        case EParticipantType::Client: return "Client";
        case EParticipantType::Daemon: return "Daemon";
        case EParticipantType::PureDaemon: return "PureDaemon";
        };
        return "<Unknown>";
    }

    void TCPConnectionSystem::addNewParticipantByAddress(const NetworkParticipantAddress& address)
    {
        // TODO: need ptype!
        // TODO: what if both daemon? guid? remember who connected/should connect (in pp)

        const bool otherIsDaemon = (m_hasOtherDaemon && address.getIp() == m_daemonAddress.getIp() && address.getPort() == m_daemonAddress.getPort());
        const bool shouldConnectbasedOnGuid = m_participantAddress.getParticipantId().get() > address.getParticipantId().get();

        if (!m_actAsDaemon && (otherIsDaemon || (!address.getParticipantId().isInvalid() && shouldConnectbasedOnGuid)))
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::addNewParticipantByAddress: attemp new connection to participant "
                     << address.getParticipantId() << " / " << address.getParticipantName()
                     << " based on: otherIsDaemon " << otherIsDaemon << ", guid valid " << !address.getParticipantId().isInvalid() << " , guid comparison " << (shouldConnectbasedOnGuid ? "will connect" : "wait for connection"));

            auto pp = std::make_shared<Participant>(address, m_runState->m_io, otherIsDaemon ? EParticipantType::Daemon : EParticipantType::Client, EParticipantState::Connecting);
            m_connectingParticipants.put(pp);
            doConnect(pp);
        }
        else
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::addNewParticipantByAddress: will wait for connection from participant "
                     << address.getParticipantId() << " / " << address.getParticipantName()
                     << " based on: otherIsDaemon " << otherIsDaemon << ", guid valid " << !address.getParticipantId().isInvalid() << " , guid comparison " << (shouldConnectbasedOnGuid ? "will connect" : "wait for connection"));
        }
    }

    bool TCPConnectionSystem::postMessageForSending(OutMessage msg)
    {
        // expect framework lock to be held
        if (!m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::postMessageForSending: called without being connected");
            return false;
        }

        m_statisticCollection.statMessagesSent.incCounter(1);

        // Skip if broadcast with no participants
        if (msg.to.empty())
            return true;

        asio::post(m_runState->m_io, [this, msg = std::move(msg)]() mutable {
                            if (msg.to.size() > 1)
                            {
                                for (auto& p : msg.to)
                                {
                                    ParticipantPtr pp;
                                    if (m_establishedParticipants.get(p, pp) != EStatus::Ok)
                                        continue; // skip invalid participant in broadcast. might happen due to disconnect race
                                    assert(pp);

                                    // cannot move here when broadcast to more than 1 participant
                                    pp->outQueue.push_back(msg);

                                    doSendQueuedMessage(pp);
                                }
                            }
                            else
                            {
                                ParticipantPtr pp;
                                if (m_establishedParticipants.get(msg.to.front(), pp) != EStatus::Ok)
                                {
                                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::postMessageForSending: post message " << msg.messageType <<
                                             " to not (fully) connected participant " << msg.to.front());
                                    return;
                                }
                                assert(pp);

                                pp->outQueue.push_back(std::move(msg));

                                doSendQueuedMessage(pp);
                            }
            });

        return true;
    }

    void TCPConnectionSystem::handleReceivedMessage(const ParticipantPtr& pp)
    {
        assert(pp->receiveBuffer.size() > 0);
        BinaryInputStream stream(pp->receiveBuffer.data());

        uint32_t recvProtocolVersion = 0;
        stream >> recvProtocolVersion;

        if (m_protocolVersion != recvProtocolVersion)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleReceivedMessage: Invalid protocol version received (expected "
                     << m_protocolVersion << ", got " << recvProtocolVersion << "). Drop connection");
            removeParticipant(pp, true);
            return;
        }

        uint32_t messageTypeTmp = 0;
        stream >> messageTypeTmp;
        EMessageId messageType = static_cast<EMessageId>(messageTypeTmp);

        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleReceivedMessage: From " <<
                 pp->address.getParticipantId() << ", type " << messageType);

        switch (messageType)
        {
        case EMessageId::Alive:
            // no-op. every message updates lastReceived
            break;
        case EMessageId::ConnectionDescriptionMessage:
            handleConnectionDescriptionMessage(pp, stream);
            break;
        case EMessageId::ConnectorAddressExchange:
            handleConnectorAddressExchange(pp, stream);
            break;
        case EMessageId::PublishScene:
            handlePublishScene(pp, stream);
            break;
        case EMessageId::UnpublishScene:
            handleUnpublishScene(pp, stream);
            break;
        case EMessageId::SubscribeScene:
            handleSubscribeScene(pp, stream);
            break;
        case EMessageId::UnsubscribeScene:
            handleUnsubscribeScene(pp, stream);
            break;
        case EMessageId::SendSceneUpdate:
            handleSceneUpdate(pp, stream);
            break;
        case EMessageId::CreateScene:
            handleCreateScene(pp, stream);
            break;
        case EMessageId::RendererEvent:
            handleRendererEvent(pp, stream);
            break;
        case EMessageId::DcsmRegisterContent:
            handleDcsmRegisterContent(pp, stream);
            break;
        case EMessageId::DcsmCanvasSizeChange:
            handleDcsmCanvasSizeChange(pp, stream);
            break;
        case EMessageId::DcsmContentStateChange:
            handleDcsmContentStateChange(pp, stream);
            break;
        case EMessageId::DcsmContentDescription:
            handleDcsmContentDescription(pp, stream);
            break;
        case EMessageId::DcsmContentAvailable:
            handleDcsmContentAvailable(pp, stream);
            break;
        case EMessageId::DcsmCategoryContentSwitchRequest:
            handleDcsmCategoryContentSwitchRequest(pp, stream);
            break;
        case EMessageId::DcsmRequestUnregisterContent:
            handleDcsmRequestUnregisterContent(pp, stream);
            break;
        case EMessageId::DcsmForceUnregisterContent:
            handleDcsmForceStopOfferContent(pp, stream);
            break;
        case EMessageId::DcsmUpdateContentMetadata:
            handleDcsmUpdateContentMetadata(pp, stream);
            break;
        case EMessageId::DcsmContentStatus:
            handleDcsmContentStatus(pp, stream);
            break;
        default:
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleReceivedMessage: Invalid messagetype " << messageType << " From " << pp->address.getParticipantId());
            removeParticipant(pp);
        }
    }

    void TCPConnectionSystem::sendConnectionDescriptionOnNewConnection(const ParticipantPtr& pp)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendConnectionDescriptionOnNewConnection: " << pp->address.getParticipantId());

        OutMessage msg(std::vector<Guid>(), EMessageId::ConnectionDescriptionMessage);
        msg.stream << m_participantAddress.getParticipantId()
                   << m_participantAddress.getParticipantName()
                   << m_participantAddress.getIp()
                   << static_cast<uint16_t>(m_runState->m_acceptor.local_endpoint().port())
                   << m_participantType;
        sendMessageToParticipant(pp, std::move(msg));
    }

    void TCPConnectionSystem::handleConnectionDescriptionMessage(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (pp->state == EParticipantState::Established)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectionDescriptionMessage: Duplicate connection description while established from " << pp->address.getParticipantId());
            return;
        }
        else if (pp->state != EParticipantState::WaitingForHello)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectionDescriptionMessage: Unexpected connection description from " << pp->address.getParticipantId() <<
                      " in state " << EnumToString(pp->state));
            removeParticipant(pp, true);
            return;
        }

        // TODO: check if information changed (?)

        Guid guid;
        String name;
        String ip;
        uint16_t port;
        EParticipantType participantType;
        stream >> guid
               >> name
               >> ip
               >> port
               >> participantType;
        pp->address = NetworkParticipantAddress(guid, name, ip, port);
        assert(!guid.isInvalid());

        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectionDescriptionMessage: Hello from " <<
                 guid << "/" << name << " type " << EnumToString(participantType) << " at " << ip << ":" << port << ". Established now");

        pp->type = participantType;
        pp->state = EParticipantState::Established;

        m_connectingParticipants.remove(pp);
        m_establishedParticipants.put(guid, pp);

        if (pp->type != EParticipantType::PureDaemon)
            triggerConnectionUpdateNotification(guid, EConnectionStatus_Connected);

        sendConnectorAddressExchangeMessagesForNewParticipant(pp);
    }

    void TCPConnectionSystem::sendConnectorAddressExchangeMessagesForNewParticipant(const ParticipantPtr& newPp)
    {
        if (m_participantType == EParticipantType::Daemon || m_participantType == EParticipantType::PureDaemon)
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendConnectorAddressExchangeMessagesForNewParticipant: Send address exchange for " <<
                     newPp->address.getParticipantId() << "/" << newPp->address.getParticipantName());

            // send all established non-daemon participants to new one
            {
                uint32_t relevantParticipants = 0;
                for (const auto& p : m_establishedParticipants)
                {
                    if (p.value->type == EParticipantType::Client)
                        ++relevantParticipants;
                }

                LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendConnectorAddressExchangeMessagesForNewParticipant: Send " << relevantParticipants << " entries to " <<
                         newPp->address.getParticipantId() << "/" << newPp->address.getParticipantName());

                OutMessage msg(newPp->address.getParticipantId(), EMessageId::ConnectorAddressExchange);
                msg.stream << relevantParticipants;
                for (const auto& p : m_establishedParticipants)
                {
                    if (p.value->type == EParticipantType::Client)
                    {
                        const NetworkParticipantAddress& addr = p.value->address;
                        msg.stream << addr.getParticipantId()
                                   << addr.getParticipantName()
                                   << addr.getIp()
                                   << addr.getPort()
                                   << p.value->type;
                    }
                }
                postMessageForSending(std::move(msg));
            }

            // send new participant to all others, inc daemons
            for (const auto& p : m_establishedParticipants)
            {
                if (newPp != p.value)
                {
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendConnectorAddressExchangeMessagesForNewParticipant: Send "
                              << newPp->address.getParticipantId() << "/" << newPp->address.getParticipantName() << " to "
                              << p.value->address.getParticipantId() << "/" << p.value->address.getParticipantName());

                    OutMessage msg(p.value->address.getParticipantId(), EMessageId::ConnectorAddressExchange);
                    const NetworkParticipantAddress& addr = newPp->address;
                    msg.stream << static_cast<uint32_t>(1)
                               << addr.getParticipantId()
                               << addr.getParticipantName()
                               << addr.getIp()
                               << addr.getPort()
                               << newPp->type;
                    postMessageForSending(std::move(msg));
                }
            }
        }
    }

    void TCPConnectionSystem::handleConnectorAddressExchange(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        uint32_t numEntries = 0;
        stream >> numEntries;

        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectorAddressExchange: from " << pp->address.getParticipantId() << ", numEntries " << numEntries);

        for (uint32_t i = 0; i < numEntries; ++i)
        {
            Guid guid;
            String name;
            String ip;
            uint16_t port;
            EParticipantType participantType;
            stream >> guid
                   >> name
                   >> ip
                   >> port
                   >> participantType;
            NetworkParticipantAddress addr(guid, name, ip, port);

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectorAddressExchange: from " << pp->address.getParticipantId() << ". " <<
                      guid << "/" << name << " at " << ip << ":" << port << ", type " << EnumToString(participantType));

            ParticipantPtr* newPpPtr = m_establishedParticipants.get(addr.getParticipantId());
            if (newPpPtr)
            {
                // already establiheds, check if matches
                const ParticipantPtr& newPp = *newPpPtr;
                if (newPp->address != addr)
                {
                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectorAddressExchange: Received mismatching participant info for "
                             << addr.getParticipantId() << ". Expect problems");
                }
                else
                {
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectorAddressExchange: Same information for existing participant "
                              << addr.getParticipantId());
                }
            }
            else if (guid == m_participantAddress.getParticipantId())
            {
                LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectorAddressExchange: Got info for self");
                // TODO: check if correct
            }
            else
            {
                // check if already connecting
                bool alreadyConnecting = false;
                for (const auto& p : m_connectingParticipants)
                {
                    if (p->address.getIp() == ip &&
                        p->address.getPort() == port)
                    {
                        alreadyConnecting = true;
                        break;
                    }
                }
                if (!alreadyConnecting)
                {
                    addNewParticipantByAddress(addr);
                }
            }
        }

        // TODO: relay on when actAsDaemon, prevent pingpong!
    }

    // --- user message handling ---
    bool TCPConnectionSystem::sendSubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendSubscribeScene: to " << to << ", sceneId " << sceneId);
        OutMessage msg(to, EMessageId::SubscribeScene);
        msg.stream << sceneId.getValue();
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleSubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneProviderHandler)
        {
            SceneId sceneId;
            stream >> sceneId.getReference();

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleSubscribeScene: from " << pp->address.getParticipantId() << ", sceneId " << sceneId);
            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleSubscribeScene(sceneId, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendUnsubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendUnsubscribeScene: to " << to << ", sceneId " << sceneId);
        OutMessage msg(to, EMessageId::UnsubscribeScene);
        msg.stream << sceneId.getValue();
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleUnsubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneProviderHandler)
        {
            SceneId sceneId;
            stream >> sceneId.getReference();

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleUnsubscribeScene: from " << pp->address.getParticipantId() << ", sceneId " << sceneId);
            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleUnsubscribeScene(sceneId, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendInitializeScene(const Guid& to, const SceneId& sceneId)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendInitializeScene: to " << to << ", sceneId " << sceneId);
        OutMessage msg(to, EMessageId::CreateScene);
        msg.stream << sceneId.getValue();
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleCreateScene(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            SceneId sceneId;
            stream >> sceneId.getReference();

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleCreateScene: from " << pp->address.getParticipantId() <<
                      ", sceneId " << sceneId);
            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleInitializeScene(sceneId, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer)
    {
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendSceneActionList: to " << to);

        static_assert(SceneActionDataSize < 1000000, "SceneActionDataSize too big");

        std::vector<Byte> buffer(SceneActionDataSize);
        return serializer.writeToPackets({buffer.data(), buffer.size()}, [&](size_t size) {

            const uint32_t usedSize = static_cast<uint32_t>(size);
            OutMessage msg(to, EMessageId::SendSceneUpdate);
            msg.stream << sceneId.getValue()
                       << usedSize;
            msg.stream.write(buffer.data(), usedSize);

            return postMessageForSending(std::move(msg));
        });
    }


    void TCPConnectionSystem::handleSceneUpdate(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            SceneId sceneId;
            stream >> sceneId.getReference();
            uint32_t dataSize = 0;
            stream >> dataSize;

            std::vector<Byte> data(dataSize);
            stream.read(data.data(), dataSize);

            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleSceneActionList: from " << pp->address.getParticipantId());

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleSceneUpdate(sceneId, std::move(data), pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::broadcastNewScenesAvailable(const SceneInfoVector& newScenes)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendScenesAvailable: to all [";
                                                for (const auto& s : newScenes)
                                                    sos << s.sceneID << "/" << s.friendlyName << "; ";
                                                sos << "]";
                                            }));

        OutMessage msg(m_connectedParticipantsForBroadcasts, EMessageId::PublishScene);
        msg.stream << static_cast<uint32_t>(newScenes.size());
        for (const auto& s : newScenes)
        {
            msg.stream << s.sceneID.getValue()
                       << s.friendlyName;
        }
        return postMessageForSending(std::move(msg));
    }

    bool TCPConnectionSystem::sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendScenesAvailable: to " << to << " [";
                                                for (const auto& s : availableScenes)
                                                    sos << s.sceneID << "/" << s.friendlyName << "; ";
                                                sos << "]";
                                            }));

        OutMessage msg(to, EMessageId::PublishScene);
        msg.stream << static_cast<uint32_t>(availableScenes.size());
        for (const auto& s : availableScenes)
        {
            msg.stream << s.sceneID.getValue()
                       << s.friendlyName;
        }
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handlePublishScene(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            uint32_t numScenes;
            stream >> numScenes;

            SceneInfoVector newScenes;
            newScenes.reserve(numScenes);

            for (uint32_t i = 0; i < numScenes; ++i)
            {
                SceneInfo sceneInfo;
                stream >> sceneInfo.sceneID.getReference()
                       >> sceneInfo.friendlyName;
                newScenes.push_back(sceneInfo);
            }

            LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                    sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handlePublishScene: from " << pp->address.getParticipantId() << " [";
                                                    for (const auto& s : newScenes)
                                                        sos << s.sceneID << "/" << s.friendlyName << "; ";
                                                    sos << "]";
                                                }));

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleNewScenesAvailable(newScenes, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::broadcastScenesBecameUnavailable: to all [";
                                                for (const auto& s : unavailableScenes)
                                                    sos << s.sceneID << "/" << s.friendlyName << "; ";
                                                sos << "]";
                                            }));

        OutMessage msg(m_connectedParticipantsForBroadcasts, EMessageId::UnpublishScene);
        msg.stream << static_cast<uint32_t>(unavailableScenes.size());
        for (const auto& s : unavailableScenes)
        {
            msg.stream << s.sceneID.getValue()
                       << s.friendlyName;
        }
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleUnpublishScene(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            uint32_t numScenes;
            stream >> numScenes;

            SceneInfoVector unavailableScenes;
            unavailableScenes.reserve(numScenes);

            for (uint32_t i = 0; i < numScenes; ++i)
            {
                SceneInfo sceneInfo;
                stream >> sceneInfo.sceneID.getReference()
                       >> sceneInfo.friendlyName;
                unavailableScenes.push_back(sceneInfo);
            }

            LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                    sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleUnpublishScene: from " << pp->address.getParticipantId() << " [";
                                                    for (const auto& s : unavailableScenes)
                                                        sos << s.sceneID << "/" << s.friendlyName << "; ";
                                                    sos << "]";
                                                }));

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleScenesBecameUnavailable(unavailableScenes,  pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendRendererEvent: to " << to << ", size " << data.size());
        if (data.size() > 32000)  // really 32768
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendRendererEvent: to " << to << " failed because size too large " << data.size());
            return false;
        }
        OutMessage msg(to, EMessageId::RendererEvent);
        msg.stream << sceneId.getValue()
                   << static_cast<uint32_t>(data.size());
        msg.stream.write(data.data(), static_cast<uint32_t>(data.size()));
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleRendererEvent(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneProviderHandler)
        {
            SceneId sceneId;
            stream >> sceneId.getReference();

            uint32_t dataSize = 0;
            stream >> dataSize;

            std::vector<Byte> data(dataSize);

            stream.read(data.data(), dataSize);

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleRendererEvent: from " << pp->address.getParticipantId() << ", size " << dataSize);
            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleRendererEvent(sceneId, std::move(data), pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmCanvasSizeChange(const Guid& to, ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation ai)
    {
        OutMessage msg(to, EMessageId::DcsmCanvasSizeChange);
        const auto blob = categoryInfo.toBinary();
        const uint64_t blobSize = blob.size();
        msg.stream << contentID.getValue()
                   << ai.startTimeStamp
                   << ai.finishedTimeStamp
                   << blobSize;
        msg.stream.write(blob.data(), static_cast<uint32_t>(blobSize));
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmCanvasSizeChange(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmProviderHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();


            AnimationInformation ai;
            stream >> ai.startTimeStamp;
            stream >> ai.finishedTimeStamp;

            uint64_t blobSize = 0;
            stream >> blobSize;

            CategoryInfo categoryInfo({stream.readPosition(), static_cast<size_t>(blobSize)});
            stream.skip(blobSize);

            PlatformGuard guard(m_frameworkLock);
            m_dcsmProviderHandler->handleCanvasSizeChange(contentID, categoryInfo, ai, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmContentStateChange(const Guid& to, ContentID contentID, EDcsmState status, const CategoryInfo& categoryInfo, AnimationInformation ai)
    {
        OutMessage msg(to, EMessageId::DcsmContentStateChange);
        const auto blob = categoryInfo.toBinary();
        const uint64_t blobSize = blob.size();
        msg.stream << contentID.getValue()
                   << status
                   << ai.startTimeStamp
                   << ai.finishedTimeStamp
                   << blobSize;
        msg.stream.write(blob.data(), static_cast<uint32_t>(blobSize));
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmContentStateChange(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmProviderHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            EDcsmState statusInfo;
            stream >> statusInfo;

            AnimationInformation ai;
            stream >> ai.startTimeStamp;
            stream >> ai.finishedTimeStamp;

            uint64_t blobSize = 0;
            stream >> blobSize;

            //TODO(Carsten): remove piggybacking for next major version
            if (static_cast<uint64_t>(statusInfo) == 27012501u && blobSize == 0)
                return handleDcsmContentStatus(pp, stream);

            CategoryInfo categoryInfo({stream.readPosition(), static_cast<size_t>(blobSize)});
            stream.skip(blobSize);

            PlatformGuard guard(m_frameworkLock);
            m_dcsmProviderHandler->handleContentStateChange(contentID, statusInfo, categoryInfo, ai, pp->address.getParticipantId());
        }
    }

    bool TCPConnectionSystem::sendDcsmContentStatus(const Guid& to, ContentID contentID, uint64_t messageID, std::vector<Byte> const& message)
    {
        // content status change piggybacking - write empty contentStateChange message first
        // TODO(Carsten): implement properly for next major version
        OutMessage msg(to, EMessageId::DcsmContentStateChange);
        msg.stream << uint64_t(0) << uint64_t(27012501) << uint64_t(0) << uint64_t(0) << uint64_t(0);

        const uint32_t usedSize = static_cast<uint32_t>(message.size());
        msg.stream << contentID.getValue() << messageID << usedSize;
        msg.stream.write(message.data(), usedSize);

        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmContentStatus(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmProviderHandler)
        {
            ContentID contentID;
            uint64_t messageID;
            uint32_t usedSize;

            stream >> contentID.getReference() >> messageID >> usedSize;
            std::vector<Byte> message(usedSize);
            stream.read(message.data(), usedSize);

            PlatformGuard guard(m_frameworkLock);
            m_dcsmProviderHandler->handleContentStatus(contentID, messageID, message, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmBroadcastOfferContent(ContentID contentID, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName)
    {
        OutMessage msg(m_connectedParticipantsForBroadcasts, EMessageId::DcsmRegisterContent);
        msg.stream << contentID.getValue()
                   << category.getValue()
                   << technicalContentType
                   << friendlyName;
        return postMessageForSending(std::move(msg));
    }

    bool TCPConnectionSystem::sendDcsmOfferContent(const Guid& to, ContentID contentID, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName)
    {
        OutMessage msg(Guid(to), EMessageId::DcsmRegisterContent);
        msg.stream << contentID.getValue()
                   << category.getValue()
                   << technicalContentType
                   << friendlyName;
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmRegisterContent(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            Category category;
            stream >> category.getReference();

            ETechnicalContentType technicalContentType;
            stream >> technicalContentType;

            std::string name;
            stream >> name;

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleOfferContent(contentID, category, technicalContentType, name, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmContentDescription(const Guid& to, ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor)
    {
        OutMessage msg(to, EMessageId::DcsmContentDescription);
        msg.stream << contentID.getValue()
                   << technicalContentDescriptor.getValue();
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmContentDescription(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            TechnicalContentDescriptor technicalContentDescriptor;
            stream >> technicalContentDescriptor.getReference();

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleContentDescription(contentID, technicalContentDescriptor, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmContentReady(const Guid& to, ContentID contentID)
    {
        OutMessage msg(to, EMessageId::DcsmContentAvailable);
        msg.stream << contentID.getValue();
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmContentAvailable(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleContentReady(contentID, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmContentEnableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest)
    {
        OutMessage msg(to, EMessageId::DcsmCategoryContentSwitchRequest);
        msg.stream << contentID.getValue();
        msg.stream << true;
        msg.stream << focusRequest;
        return postMessageForSending(std::move(msg));
    }

    bool TCPConnectionSystem::sendDcsmContentDisableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest)
    {
        OutMessage msg(to, EMessageId::DcsmCategoryContentSwitchRequest);
        msg.stream << contentID.getValue();
        msg.stream << false;
        msg.stream << focusRequest;
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmCategoryContentSwitchRequest(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            bool isEnable = false;
            int32_t focusRequest = 0;
            stream >> isEnable;
            stream >> focusRequest;
            PlatformGuard guard(m_frameworkLock);
            if (isEnable)
            {
                m_dcsmConsumerHandler->handleContentEnableFocusRequest(contentID, focusRequest, pp->address.getParticipantId());
            }
            else
            {
                m_dcsmConsumerHandler->handleContentDisableFocusRequest(contentID, focusRequest, pp->address.getParticipantId());
            }
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmBroadcastRequestStopOfferContent(ContentID contentID)
    {
        OutMessage msg(m_connectedParticipantsForBroadcasts, EMessageId::DcsmRequestUnregisterContent);
        msg.stream << contentID.getValue();
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmRequestUnregisterContent(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleRequestStopOfferContent(contentID, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmBroadcastForceStopOfferContent(ContentID contentID)
    {
        OutMessage msg(m_connectedParticipantsForBroadcasts, EMessageId::DcsmForceUnregisterContent);
        msg.stream << contentID.getValue();
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmForceStopOfferContent(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleForceStopOfferContent(contentID, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmUpdateContentMetadata(const Guid& to, ContentID contentID, const DcsmMetadata& metadata)
    {
        OutMessage msg(to, EMessageId::DcsmUpdateContentMetadata);
        const auto blob = metadata.toBinary();
        const uint64_t blobSize = blob.size();
        msg.stream << contentID.getValue()
                   << blobSize;
        msg.stream.write(blob.data(), static_cast<uint32_t>(blobSize));
        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handleDcsmUpdateContentMetadata(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            uint64_t blobSize = 0;
            stream >> blobSize;

            DcsmMetadata metadata({stream.readPosition(), static_cast<size_t>(blobSize)});
            stream.skip(blobSize);

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleUpdateContentMetadata(contentID, std::move(metadata), pp->address.getParticipantId());
        }
    }

    // --- ramsh command handling ---
    void TCPConnectionSystem::logConnectionInfo()
    {
        PlatformGuard guard(m_frameworkLock);
        auto logFunction =
            [this]() {
                LOG_INFO_F(CONTEXT_PERIODIC,
                           ([&](StringOutputStream& sos)
                            {
                                // general info
                                sos << "TCPConnectionSystem:\n";
                                sos << "  Self: " << m_participantAddress.getParticipantName() << " / " << m_participantAddress.getParticipantId() << "\n";
                                sos << "  Connected: " << (m_runState ? "Yes" : "No") << "\n";
                                sos << "  Protocol version: " << m_protocolVersion << "\n";
                                sos << "  Type: " << EnumToString(m_participantType) << "\n";
                                if (m_hasOtherDaemon)
                                    sos << "  Upstram daemon at " << m_daemonAddress.getIp() << ":" << m_daemonAddress.getPort() << "\n";

                                // established connections
                                sos << "Established connections:\n";
                                for (const auto& p : m_establishedParticipants)
                                {
                                    const auto& addr = p.value->address;
                                    sos << "  "  << addr.getParticipantId() << " / " << addr.getParticipantName() << " at " << addr.getIp() << ":" << addr.getPort();
                                    if (m_hasOtherDaemon && addr.getIp() == m_daemonAddress.getIp() && addr.getPort() == m_daemonAddress.getPort())
                                        sos << " (daemon)";
                                    sos << "\n";
                                }

                                // connection ongoing
                                sos << "Connection attempts:\n";
                                for (const auto& p : m_connectingParticipants)
                                {
                                    const auto& addr = p->address;
                                    sos << "  "  << addr.getIp() << ":" << addr.getPort() << " " << EnumToString(p->state);
                                    if (m_hasOtherDaemon && addr.getIp() == m_daemonAddress.getIp() && addr.getPort() == m_daemonAddress.getPort())
                                        sos << " (daemon)";
                                    sos << "\n";
                                }
                            }));
                    };

        if (m_runState)
            asio::post(m_runState->m_io, logFunction);
        else
            logFunction();
    }

    void TCPConnectionSystem::triggerLogMessageForPeriodicLog()
    {
        // expect framework lock to be held
        if (m_runState)
            asio::post(m_runState->m_io, [this]() {
                    LOG_INFO_F(CONTEXT_PERIODIC,
                               ([&](StringOutputStream& sos)
                                {
                                    sos << "Connected Participant(s): ";
                                    if (m_establishedParticipants.size() == 0)
                                    {
                                        sos << "None";
                                    }
                                    else
                                    {
                                        for (const auto& p : m_establishedParticipants)
                                        {
                                            sos << p.key << "; ";
                                        }
                                    }
                                }));
                });
        else
            LOG_INFO(CONTEXT_PERIODIC, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << "): Not connected");
    }

    void TCPConnectionSystem::triggerConnectionUpdateNotification(Guid participant, EConnectionStatus status)
    {
        PlatformGuard guard(m_frameworkLock);
        if (status == EConnectionStatus_Connected)
        {
            assert(std::find(m_connectedParticipantsForBroadcasts.begin(), m_connectedParticipantsForBroadcasts.end(), participant) == m_connectedParticipantsForBroadcasts.end());
            m_connectedParticipantsForBroadcasts.push_back(participant);
        }
        else
        {
            m_connectedParticipantsForBroadcasts.erase(std::remove(m_connectedParticipantsForBroadcasts.begin(),
                                                                   m_connectedParticipantsForBroadcasts.end(),
                                                                   participant),
                                                       m_connectedParticipantsForBroadcasts.end());
        }
        m_ramsesConnectionStatusUpdateNotifier.triggerNotification(participant, status);
        m_dcsmConnectionStatusUpdateNotifier.triggerNotification(participant, status);
    }


    // --- TCPConnectionSystem::Participant ---
    TCPConnectionSystem::Participant::Participant(const NetworkParticipantAddress& address_, asio::io_service& io_,
                                                  EParticipantType type_, EParticipantState state_)
        : address(address_)
        , socket(io_)
        , connectTimer(io_)
        , lengthReceiveBuffer(0)
        , sendAliveTimer(io_)
        , checkReceivedAliveTimer(io_)
        , type(type_)
        , state(state_)
    {}

    TCPConnectionSystem::Participant::~Participant()
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem::~Participant(" << address.getParticipantName() << ")");
        if (socket.is_open())
        {
            asio::error_code ec;
            socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            if (ec)
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem::~Participant(" << address.getParticipantName() << "): shutdown failed: " << ec.message().c_str());

            socket.close(ec);
            if (ec)
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem::~Participant(" << address.getParticipantName() << "): close failed: " << ec.message().c_str());
        }
    }

    // --- TCPConnectionSystem::RunState ---
    TCPConnectionSystem::RunState::RunState()
        : m_io()
        , m_acceptor(m_io)
        , m_acceptorSocket(m_io)
    {}
}
