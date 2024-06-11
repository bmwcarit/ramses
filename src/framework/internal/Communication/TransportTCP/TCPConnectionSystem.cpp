//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Communication/TransportTCP/TCPConnectionSystem.h"

#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Core/Utils/RawBinaryOutputStream.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/Core/Utils/LogMacros.h"
#include <thread>
#include <utility>
#include "internal/Communication/TransportCommon/ISceneUpdateSerializer.h"

namespace ramses::internal
{
    static const constexpr uint32_t ResourceDataSize = 300000;
    static const constexpr uint32_t SceneActionDataSize = 300000;

    TCPConnectionSystem::TCPConnectionSystem(NetworkParticipantAddress participantAddress,
                                                     uint32_t protocolVersion,
                                                     NetworkParticipantAddress  daemonAddress,
                                                     bool pureDaemon,
                                                     PlatformLock& frameworkLock,
                                                     StatisticCollectionFramework& statisticCollection,
                                                     std::chrono::milliseconds aliveInterval,
                                                     std::chrono::milliseconds aliveTimeout)
        : m_participantAddress(std::move(participantAddress))
        , m_protocolVersion(protocolVersion)
        , m_daemonAddress(std::move(daemonAddress))
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
        , m_thread("TCP_ConnSys")
        , m_statisticCollection(statisticCollection)
        , m_ramsesConnectionStatusUpdateNotifier(m_participantAddress.getParticipantName(), CONTEXT_COMMUNICATION, "ramses", frameworkLock)
        , m_sceneProviderHandler(nullptr)
        , m_sceneRendererHandler(nullptr)
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
        LOG_INFO_F(CONTEXT_COMMUNICATION, ([&](StringOutputStream& sos) {
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
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({}): Alive timeout very low, expect issues (alive {}, timeout {})",
                m_participantAddress.getParticipantName(), m_aliveInterval.count(), m_aliveIntervalTimeout.count());
        }

        PlatformGuard guard(m_frameworkLock);
        if (m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::connectServices: called more than once", m_participantAddress.getParticipantName());
            return false;
        }

        m_runState = std::make_unique<RunState>();
        m_thread.start(*this);

        return true;
    }

    bool TCPConnectionSystem::disconnectServices()
    {
        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::disconnectServices", m_participantAddress.getParticipantName());

        PlatformGuard guard(m_frameworkLock);
        if (!m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::disconnectServices: called without being connected", m_participantAddress.getParticipantName());
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

        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::disconnectServices: done", m_participantAddress.getParticipantName());
        return true;
    }

    IConnectionStatusUpdateNotifier& TCPConnectionSystem::getRamsesConnectionStatusUpdateNotifier()
    {
        return m_ramsesConnectionStatusUpdateNotifier;
    }

    void TCPConnectionSystem::setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler)
    {
        m_sceneProviderHandler = handler;
    }

    void TCPConnectionSystem::setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler)
    {
        m_sceneRendererHandler = handler;
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
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::run: Initiate connection to daemon at {}:{}", m_participantAddress.getParticipantName(), m_daemonAddress.getIp(), m_daemonAddress.getPort());

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
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::openAcceptor: open failed. {}", m_participantAddress.getParticipantName(), e.message().c_str());
            return false;
        }

        m_runState->m_acceptor.set_option(asio::socket_base::reuse_address(true));

        // always accept connections from all to also work on uncommon setups (could be changed to use m_participantAddress.getIp())
        m_runState->m_acceptor.bind(asio::ip::tcp::endpoint(asio::ip::address::from_string("0.0.0.0"), m_participantAddress.getPort()), e);
        if (e)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::openAcceptor: bind failed. {}", m_participantAddress.getParticipantName(), e.message().c_str());
            return false;
        }

        m_runState->m_acceptor.listen(asio::socket_base::max_connections, e);
        if (e)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::openAcceptor: listen failed. {}", m_participantAddress.getParticipantName(), e.message().c_str());
            return false;
        }

        const auto endpoint = m_runState->m_acceptor.local_endpoint();
        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::openAcceptor: Listening for connections on {}:{}", m_participantAddress.getParticipantName(), endpoint.address().to_string().c_str(), endpoint.port());

        return true;
    }

    void TCPConnectionSystem::doAcceptIncomingConnections()
    {
        m_runState->m_acceptor.async_accept(m_runState->m_acceptorSocket,
                                [this](asio::error_code e) {
                                    if (e)
                                    {
                                        LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doAcceptIncomingConnections: accept failed. {}", m_participantAddress.getParticipantName(), e.message().c_str());
                                        doAcceptIncomingConnections();
                                        // TODO: not sure if correct response or is really recoverable (close + reopen acceptor?)
                                    }
                                    else
                                    {
                                        auto remoteEp = m_runState->m_acceptorSocket.remote_endpoint();
                                        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doAcceptIncomingConnections: Accepted new connection from {}:{}",
                                            m_participantAddress.getParticipantName(), remoteEp.address().to_string().c_str(), remoteEp.port());

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
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doConnect: Try connect to participant at {}:{}", m_participantAddress.getParticipantName(), pp->address.getIp(), pp->address.getPort());

        // convert localhost to an actual ip, no need for dns resolving here
        const char* const ipStr = (pp->address.getIp() == "localhost" ? "127.0.0.1" : pp->address.getIp().c_str());

        // parse with error checking to prevent exception when ip is invalid format
        asio::error_code err;
        const auto asioIp = asio::ip::address::from_string(ipStr, err);
        if (err)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doConnect: Failed to parse ip address '{}:{}'", m_participantAddress.getParticipantName(), pp->address.getIp(), pp->address.getPort());
            return;
        }

        asio::ip::tcp::endpoint ep(asioIp, pp->address.getPort());
        std::array<asio::ip::tcp::endpoint, 1> endpointSequence = {ep};
        asio::async_connect(pp->socket, endpointSequence, [this, pp](asio::error_code e, const asio::ip::tcp::endpoint& usedEndpoint) {
                if (e)
                {
                    // connect failed, try again after timeout
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doConnect: Connect to {}:{} failed. {}",
                        m_participantAddress.getParticipantName(), pp->address.getIp(), pp->address.getPort(), e.message().c_str());
                    pp->connectTimer.expires_after(std::chrono::milliseconds{100});
                    pp->connectTimer.async_wait([this, pp](asio::error_code ee) {
                                                    if (ee)
                                                    {
                                                        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doConnect: Connect to {}:{} failed. Timer canceled",
                                                            m_participantAddress.getParticipantName(), pp->address.getIp(), pp->address.getPort());
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
                    LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doConnect: Established to {}:{}", m_participantAddress.getParticipantName(), usedEndpoint.address().to_string().c_str(), usedEndpoint.port());
                    initializeNewlyConnectedParticipant(pp);
                }
            });
    }

    void TCPConnectionSystem::initializeNewlyConnectedParticipant(const ParticipantPtr& pp)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::initializeNewlyConnectedParticipant: {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId());
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
        const auto fullSize = static_cast<uint32_t>(pp->currentOutBuffer.size());

        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendMessageToParticipant: To {}, MsgType {}, Size {}",
            m_participantAddress.getParticipantName(), pp->address.getParticipantId(), msg.messageType, fullSize);

        RawBinaryOutputStream s(pp->currentOutBuffer.data(), pp->currentOutBuffer.size());
        const uint32_t remainingSize = fullSize - sizeof(pp->lengthReceiveBuffer);
        s << remainingSize
          << m_protocolVersion;

        asio::async_write(pp->socket, asio::const_buffer(pp->currentOutBuffer.data(), pp->currentOutBuffer.size()),
                          [this, pp](asio::error_code e, std::size_t sentBytes) {
                              if (e)
                              {
                                  LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendMessageToParticipant: Send to {}/{} failed. {}. Remove participant",
                                      m_participantAddress.getParticipantName(), pp->address.ParticipantIdentifier::getParticipantId(), pp->address.ParticipantIdentifier::getParticipantName(), e.message().c_str());

                                  removeParticipant(pp);
                              }
                              else
                              {
                                  LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendMessageToParticipant: To {}, MsgBytes {}, SentBytes {}",
                                      m_participantAddress.getParticipantName(), pp->address.getParticipantId(), pp->currentOutBuffer.size(), sentBytes);

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
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doReadHeader: start reading from {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId());

        asio::async_read(pp->socket,
                         asio::mutable_buffer(&pp->lengthReceiveBuffer, sizeof(pp->lengthReceiveBuffer)),
                         [this, pp](asio::error_code e, size_t readBytes) {
                             if (e)
                             {
                                 LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doReadHeader: read from {} failed (len {}). ",
                                     m_participantAddress.getParticipantName(), pp->address.getParticipantId(), readBytes, e.message().c_str());
                                 removeParticipant(pp);
                             }
                             else
                             {
                                 LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doReadHeader: done read from {}. Expect {} bytes", m_participantAddress.getParticipantName(), pp->address.getParticipantId(), pp->lengthReceiveBuffer);

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
                                 LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doReadContent: read from {} failed (len {}). {}",
                                     m_participantAddress.getParticipantName(), pp->address.getParticipantId(), readBytes, e.message().c_str());
                                 removeParticipant(pp);
                             }
                             else
                             {
                                 LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::doReadContent: done read from {}, {} bytes",
                                     m_participantAddress.getParticipantName(), pp->address.getParticipantId(), readBytes);

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
                                                       LOG_WARN_F(CONTEXT_COMMUNICATION, ([&](StringOutputStream& sos) {
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

        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::removeParticipant: {}/{}, state {}",
            m_participantAddress.getParticipantName(), pp->address.getParticipantId(), pp->address.getParticipantName(), EnumToString(pp->state));

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
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::removeParticipant: will delay reconnect by {}ms", m_participantAddress.getParticipantName(), backoffTime.count());
            pp->connectTimer.expires_after(backoffTime);
            pp->connectTimer.async_wait([this, pp](asio::error_code ee) {
                if (ee)
                {
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::removeParticipant: Backoff timer got canceled.", m_participantAddress.getParticipantName());
                }
                else
                {
                    addNewParticipantByAddress(pp->address);
                }
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
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::addNewParticipantByAddress: attempt new connection to participant {} / {} based on: otherIsDaemon {}, guid valid {}, guid comparison {}",
                m_participantAddress.getParticipantName(), address.getParticipantId(), address.getParticipantName(), otherIsDaemon, !address.getParticipantId().isInvalid(), shouldConnectbasedOnGuid ? "will connect" : "wait for connection");

            auto pp = std::make_shared<Participant>(address, m_runState->m_io, otherIsDaemon ? EParticipantType::Daemon : EParticipantType::Client, EParticipantState::Connecting);
            m_connectingParticipants.put(pp);
            doConnect(pp);
        }
        else
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::addNewParticipantByAddress: will wait for connection from participant {} / {} based on: otherIsDaemon {}, guid valid {}, guid comparison {}",
                m_participantAddress.getParticipantName(), address.getParticipantId(), address.getParticipantName(), otherIsDaemon, !address.getParticipantId().isInvalid(), shouldConnectbasedOnGuid ? "will connect" : "wait for connection");
        }
    }

    bool TCPConnectionSystem::postMessageForSending(OutMessage msg)
    {
        // expect framework lock to be held
        if (!m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::postMessageForSending: called without being connected", m_participantAddress.getParticipantName());
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
                                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::postMessageForSending: post message {} to not (fully) connected participant {}",
                                        m_participantAddress.getParticipantName(), msg.messageType, msg.to.front());
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
        assert(!pp->receiveBuffer.empty());
        BinaryInputStream stream(pp->receiveBuffer.data());

        uint32_t recvProtocolVersion = 0;
        stream >> recvProtocolVersion;

        if (m_protocolVersion != recvProtocolVersion)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleReceivedMessage: Invalid protocol version received (expected {}, got {}). Drop connection",
                m_participantAddress.getParticipantName(), m_protocolVersion, recvProtocolVersion);
            removeParticipant(pp, true);
            return;
        }

        uint32_t messageTypeTmp = 0;
        stream >> messageTypeTmp;
        auto messageType = static_cast<EMessageId>(messageTypeTmp);

        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleReceivedMessage: From {}, type {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId(), messageType);

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
        default:
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleReceivedMessage: Invalid messagetype {} From {}", m_participantAddress.getParticipantName(), messageType, pp->address.getParticipantId());
            removeParticipant(pp);
        }
    }

    void TCPConnectionSystem::sendConnectionDescriptionOnNewConnection(const ParticipantPtr& pp)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendConnectionDescriptionOnNewConnection: {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId());

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
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectionDescriptionMessage: Duplicate connection description while established from {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId());
            return;
        }
        if (pp->state != EParticipantState::WaitingForHello)
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectionDescriptionMessage: Unexpected connection description from {} in state {}",
                m_participantAddress.getParticipantName(), pp->address.getParticipantId(), EnumToString(pp->state));
            removeParticipant(pp, true);
            return;
        }

        // TODO: check if information changed (?)

        Guid guid;
        std::string name;
        std::string ip;
        uint16_t port = 0u;
        EParticipantType participantType;
        stream >> guid
               >> name
               >> ip
               >> port
               >> participantType;
        pp->address = NetworkParticipantAddress(guid, name, ip, port);
        assert(!guid.isInvalid());

        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectionDescriptionMessage: Hello from {}/{} type {} at {}:{}. Established now",
            m_participantAddress.getParticipantName(), guid, name, EnumToString(participantType), ip, port);

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
            LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendConnectorAddressExchangeMessagesForNewParticipant: Send address exchange for {}/{}",
                m_participantAddress.getParticipantName(), newPp->address.getParticipantId(), newPp->address.getParticipantName());

            // send all established non-daemon participants to new one
            {
                uint32_t relevantParticipants = 0;
                for (const auto& p : m_establishedParticipants)
                {
                    if (p.value->type == EParticipantType::Client)
                        ++relevantParticipants;
                }

                LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendConnectorAddressExchangeMessagesForNewParticipant: Send {} entries to {}/{}",
                    m_participantAddress.getParticipantName(), relevantParticipants, newPp->address.getParticipantId(), newPp->address.getParticipantName());

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
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendConnectorAddressExchangeMessagesForNewParticipant: Send {}/{} to {}/{}",
                        m_participantAddress.getParticipantName(), newPp->address.getParticipantId(), newPp->address.getParticipantName(), p.value->address.getParticipantId(), p.value->address.getParticipantName());

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

        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectorAddressExchange: from {}, numEntries {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId(), numEntries);

        for (uint32_t i = 0; i < numEntries; ++i)
        {
            Guid guid;
            std::string name;
            std::string ip;
            uint16_t port = 0u;
            EParticipantType participantType;
            stream >> guid
                   >> name
                   >> ip
                   >> port
                   >> participantType;
            NetworkParticipantAddress addr(guid, name, ip, port);

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectorAddressExchange: from {}. {}/{} at {}:{}, type {}",
                m_participantAddress.getParticipantName(), pp->address.getParticipantId(), guid, name, ip, port, EnumToString(participantType));

            ParticipantPtr* newPpPtr = m_establishedParticipants.get(addr.getParticipantId());
            if (newPpPtr)
            {
                // already establiheds, check if matches
                const ParticipantPtr& newPp = *newPpPtr;
                if (newPp->address != addr)
                {
                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectorAddressExchange: Received mismatching participant info for {}. Expect problems",
                        m_participantAddress.getParticipantName(), addr.getParticipantId());
                }
                else
                {
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectorAddressExchange: Same information for existing participant {}",
                        m_participantAddress.getParticipantName(), addr.getParticipantId());
                }
            }
            else if (guid == m_participantAddress.getParticipantId())
            {
                LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleConnectorAddressExchange: Got info for self", m_participantAddress.getParticipantName());
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
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendSubscribeScene: to {}, sceneId {}", m_participantAddress.getParticipantName(), to, sceneId);
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

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleSubscribeScene: from {}, sceneId {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId(), sceneId);
            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleSubscribeScene(sceneId, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendUnsubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendUnsubscribeScene: to {}, sceneId {}", m_participantAddress.getParticipantName(), to, sceneId);
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

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleUnsubscribeScene: from {}, sceneId {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId(), sceneId);
            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleUnsubscribeScene(sceneId, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendInitializeScene(const Guid& to, const SceneId& sceneId)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendInitializeScene: to {}, sceneId {}", m_participantAddress.getParticipantName(), to, sceneId);
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

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleCreateScene: from {}, sceneId {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId(), sceneId);
            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleInitializeScene(sceneId, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer)
    {
        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendSceneActionList: to {}", m_participantAddress.getParticipantName(), to);

        static_assert(SceneActionDataSize < 1000000, "SceneActionDataSize too big");

        std::vector<std::byte> buffer(SceneActionDataSize);
        return serializer.writeToPackets({buffer.data(), buffer.size()}, [&](size_t size) {

            const auto usedSize = static_cast<uint32_t>(size);
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

            std::vector<std::byte> data(dataSize);
            stream.read(data.data(), dataSize);

            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleSceneActionList: from {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId());

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleSceneUpdate(sceneId, data, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::broadcastNewScenesAvailable(const SceneInfoVector& newScenes, EFeatureLevel featureLevel)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](StringOutputStream& sos) {
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
        msg.stream << static_cast<uint32_t>(featureLevel);

        if (featureLevel >= EFeatureLevel_02)
        {
            for (const auto& sceneInfo : newScenes)
                msg.stream << sceneInfo.renderBackendCompatibility << sceneInfo.vulkanAPIVersion << sceneInfo.spirvVersion;
        }

        return postMessageForSending(std::move(msg));
    }

    bool TCPConnectionSystem::sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes, EFeatureLevel featureLevel)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](StringOutputStream& sos) {
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
        msg.stream << static_cast<uint32_t>(featureLevel);

        if (featureLevel >= EFeatureLevel_02)
        {
            for (const auto& sceneInfo : availableScenes)
                msg.stream << sceneInfo.renderBackendCompatibility << sceneInfo.vulkanAPIVersion << sceneInfo.spirvVersion;
        }

        return postMessageForSending(std::move(msg));
    }

    void TCPConnectionSystem::handlePublishScene(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            uint32_t numScenes = 0u;
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

            uint32_t featureLevelInt = 0u;
            stream >> featureLevelInt;
            const auto featureLevel = static_cast<EFeatureLevel>(featureLevelInt);

            if (featureLevel >= EFeatureLevel_02)
            {
                for (auto& sceneInfo : newScenes)
                    stream >> sceneInfo.renderBackendCompatibility >> sceneInfo.vulkanAPIVersion >> sceneInfo.spirvVersion;
            }

            LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](StringOutputStream& sos) {
                                                    sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handlePublishScene: from " << pp->address.getParticipantId() << " [";
                                                    for (const auto& s : newScenes)
                                                        sos << s.sceneID << "/" << s.friendlyName << "; ";
                                                    sos << "]";
                                                }));

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleNewScenesAvailable(newScenes, pp->address.getParticipantId(), featureLevel);
        }
    }

    // --
    bool TCPConnectionSystem::broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](StringOutputStream& sos) {
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
            uint32_t numScenes = 0u;
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

            LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](StringOutputStream& sos) {
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
    bool TCPConnectionSystem::sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<std::byte>& data)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendRendererEvent: to {}, size {}", m_participantAddress.getParticipantName(), to, data.size());
        if (data.size() > 32000)  // really 32768
        {
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::sendRendererEvent: to {} failed because size too large {}", m_participantAddress.getParticipantName(), to, data.size());
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

            std::vector<std::byte> data(dataSize);

            stream.read(data.data(), dataSize);

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem({})::handleRendererEvent: from {}, size {}", m_participantAddress.getParticipantName(), pp->address.getParticipantId(), dataSize);
            PlatformGuard guard(m_frameworkLock);
            m_sceneProviderHandler->handleRendererEvent(sceneId, data, pp->address.getParticipantId());
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
        {
            asio::post(m_runState->m_io, logFunction);
        }
        else
        {
            logFunction();
        }
    }

    void TCPConnectionSystem::triggerLogMessageForPeriodicLog()
    {
        // expect framework lock to be held
        if (m_runState)
        {
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
        }
        else
        {
            LOG_INFO(CONTEXT_PERIODIC, "TCPConnectionSystem({}): Not connected", m_participantAddress.getParticipantName());
        }
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
    }


    // --- TCPConnectionSystem::Participant ---
    TCPConnectionSystem::Participant::Participant(NetworkParticipantAddress address_, asio::io_service& io_,
                                                  EParticipantType type_, EParticipantState state_)
        : address(std::move(address_))
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
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem::~Participant({})", address.getParticipantName());
        if (socket.is_open())
        {
            asio::error_code ec;
            socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            if (ec)
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem::~Participant({}): shutdown failed: {}", address.getParticipantName(), ec.message().c_str());

            socket.close(ec);
            if (ec)
                LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem::~Participant({}): close failed: {}", address.getParticipantName(), ec.message().c_str());
        }
    }

    // --- TCPConnectionSystem::RunState ---
    TCPConnectionSystem::RunState::RunState()
        : m_io()
        , m_acceptor(m_io)
        , m_acceptorSocket(m_io)
    {}
}
