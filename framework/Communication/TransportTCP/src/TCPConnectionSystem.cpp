//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportTCP/TCPConnectionSystem.h"

#include "Components/ResourceStreamSerialization.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Scene/SceneActionCollection.h"
#include "TransportCommon/TransportUtilities.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/RawBinaryOutputStream.h"
#include "Utils/StatisticCollection.h"
#include <thread>

namespace ramses_internal
{
    static const constexpr uint32_t ResourceDataSize = 300000;

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
        , m_sendDataSizes(CommunicationSendDataSizes {
            std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(),
            ResourceDataSize, std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max() })
        , m_frameworkLock(frameworkLock)
        , m_thread("R_TCP_ConnSys")
        , m_statisticCollection(statisticCollection)
        , m_ramsesConnectionStatusUpdateNotifier(m_participantAddress.getParticipantName(), "ramses", frameworkLock)
        , m_dcsmConnectionStatusUpdateNotifier(m_participantAddress.getParticipantName(), "dcsm", frameworkLock)
        , m_resourceConsumerHandler(nullptr)
        , m_resourceProviderHandler(nullptr)
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
                                                   << ", aliveInterval " << static_cast<int64_t>(m_aliveInterval.count()) << "ms, aliveTimeout " << static_cast<int64_t>(m_aliveIntervalTimeout.count()) << "ms";
                                               if (m_hasOtherDaemon)
                                                   sos << ", other daemon at " << m_daemonAddress.getIp() << ":" << m_daemonAddress.getPort();
                                           }));
        if (m_aliveIntervalTimeout < m_aliveInterval + std::chrono::milliseconds{100})
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << "): Alive timeout very low, expect issues " <<
                     "(alive " << static_cast<int64_t>(m_aliveInterval.count()) << ", timeout " << static_cast<int64_t>(m_aliveIntervalTimeout.count()) << ")");
        }

        if (m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::connectServices: called more than once");
            return false;
        }

        m_runState.reset(new RunState{});
        m_thread.start(*this);

        return true;
    }

    bool TCPConnectionSystem::disconnectServices()
    {
        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::disconnectServices");
        if (!m_runState)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::disconnectServices: called without being connected");
            return false;
        }

        // Signal context to exit run and join thread
        m_runState->m_io.stop();
        m_thread.join();

        // Clear shared pointer and wait for possible concurrent users to disappear (guarantees that it is really destructed)
        std::weak_ptr<RunState> weakRunState(m_runState);
        m_runState.reset();
        for (int i = 0; i < 20; ++i)
        {
            if (!weakRunState.lock())
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds{100});
        }
        if (weakRunState.lock())
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::disconnectServices: could not properly delete asio::io_service, may cause further problems");
        }

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
                m_ramsesConnectionStatusUpdateNotifier.triggerNotification(pp.value->address.getParticipantId(), EConnectionStatus_NotConnected);
                m_dcsmConnectionStatusUpdateNotifier.triggerNotification(pp.value->address.getParticipantId(), EConnectionStatus_NotConnected);
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
        asio::async_connect(pp->socket, endpointSequence, [this, pp](asio::error_code e, asio::ip::tcp::endpoint usedEndpoint) {
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
        assert(m_connectingParticipants.hasElement(pp));

        // Set send buffer to resource chunk size to allow maximum one resource chunk to use up send buffer. This
        // is needed to allow high prio data to be sent as fast as possible.
        pp->socket.set_option(asio::socket_base::send_buffer_size{static_cast<int>(m_sendDataSizes.resourceDataArray)});
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
                  ", MsgType " << GetNameForMessageId(msg.messageType) << ", Size " << fullSize);

        RawBinaryOutputStream s(reinterpret_cast<uint8_t*>(pp->currentOutBuffer.data()), static_cast<uint32_t>(pp->currentOutBuffer.size()));
        const uint32_t remainingSize = fullSize - sizeof(pp->lengthReceiveBuffer);
        s << remainingSize;

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
        if (pp->currentOutBuffer.empty() &&
            (!pp->outQueueNormal.empty() || !pp->outQueuePrio.empty()))
        {
            auto& queue = pp->outQueuePrio.empty() ? pp->outQueueNormal : pp->outQueuePrio;
            OutMessage msg = std::move(queue.front());
            queue.pop_front();

            sendMessageToParticipant(pp, std::move(msg));
        }
    }

    void TCPConnectionSystem::doTrySendAliveMessage(const ParticipantPtr& pp)
    {
        if (pp->currentOutBuffer.empty())
        {
            assert(pp->outQueueNormal.empty());
            assert(pp->outQueuePrio.empty());

            sendMessageToParticipant(pp, OutMessage(pp->address.getParticipantId(), EMessageId_Alive));
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
        pp->checkReceivedAliveTimer.async_wait([this, pp](asio::error_code e) {
                                                   if (!e)
                                                   {
                                                       const auto now = std::chrono::steady_clock::now();
                                                       LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::updateLastReceivedTime: alive message from " <<
                                                                pp->address.ParticipantIdentifier::getParticipantId() << " too old. lastReceived " <<
                                                                (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pp->lastReceived).count()) << "ms ago, expected alive " <<
                                                                (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pp->lastReceived - m_aliveInterval).count()) << "ms ago");
                                                       removeParticipant(pp);
                                                   }
                                               });
    }

    void TCPConnectionSystem::removeParticipant(const ParticipantPtr& pp)
    {
        if (pp->state == EParticipantState::Invalid)
            return;

        LOG_INFO(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::removeParticipant: " << pp->address.getParticipantId() << "/" << pp->address.getParticipantName() <<
                 ", state " << EnumToString(pp->state));

        if (pp->state == EParticipantState::Established && pp->type != EParticipantType::PureDaemon)
        {
            m_ramsesConnectionStatusUpdateNotifier.triggerNotification(pp->address.getParticipantId(), EConnectionStatus_NotConnected);
            m_dcsmConnectionStatusUpdateNotifier.triggerNotification(pp->address.getParticipantId(), EConnectionStatus_NotConnected);
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
        const bool shouldConnectbasedOnGuid = PlatformMemory::Compare(&m_participantAddress.getParticipantId().getGuidData(), &address.getParticipantId().getGuidData(), sizeof(generic_uuid_t)) > 0;

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

    bool TCPConnectionSystem::postMessageForSending(OutMessage msg, bool hasPrio)
    {
        // Keep RunState alive between check and use
        RunStatePtr rs = m_runState;
        if (!rs)
            return false;

        m_statisticCollection.statMessagesSent.incCounter(1);

        asio::post(rs->m_io, [this, msg, hasPrio]() {
                            const bool broadcast = msg.to.isInvalid();
                            if (broadcast)
                            {
                                for (auto& p : m_establishedParticipants)
                                {
                                    ParticipantPtr& pp = p.value;
                                    assert(pp);

                                    // cannot move here when broadcast to more than 1 participant
                                    if (hasPrio)
                                        pp->outQueuePrio.push_back(msg);
                                    else
                                        pp->outQueueNormal.push_back(msg);

                                    doSendQueuedMessage(pp);
                                }
                            }
                            else
                            {
                                ParticipantPtr pp;
                                if (m_establishedParticipants.get(msg.to, pp) != EStatus_RAMSES_OK)
                                {
                                    LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::postMessageForSending: post message " << GetNameForMessageId(msg.messageType) <<
                                             " to not (fully) connected participant " << msg.to);
                                    return;
                                }
                                assert(pp);

                                if (hasPrio)
                                    pp->outQueuePrio.push_back(std::move(msg));
                                else
                                    pp->outQueueNormal.push_back(std::move(msg));

                                doSendQueuedMessage(pp);
                            }
            });

        return true;
    }

    void TCPConnectionSystem::handleReceivedMessage(const ParticipantPtr& pp)
    {
        assert(pp->receiveBuffer.size() > 0);
        BinaryInputStream stream(pp->receiveBuffer.data());

        uint32_t messageTypeTmp = 0;
        stream >> messageTypeTmp;
        EMessageId messageType = static_cast<EMessageId>(messageTypeTmp);

        LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleReceivedMessage: From " <<
                 pp->address.getParticipantId() << ", type " << GetNameForMessageId(messageType) << "/" << messageType);

        switch (messageType)
        {
        case EMessageId_Alive:
            // no-op. every message updates lastReceived
            break;
        case EMessageId_ConnectionDescriptionMessage:
            handleConnectionDescriptionMessage(pp, stream);
            break;
        case EMessageId_ConnectorAddressExchange:
            handleConnectorAddressExchange(pp, stream);
            break;
        case EMessageId_PublishScene:
            handlePublishScene(pp, stream);
            break;
        case EMessageId_UnpublishScene:
            handleUnpublishScene(pp, stream);
            break;
        case EMessageId_SubscribeScene:
            handleSubscribeScene(pp, stream);
            break;
        case EMessageId_UnsubscribeScene:
            handleUnsubscribeScene(pp, stream);
            break;
        case EMessageId_SceneNotAvailable:
            handleSceneNotAvailable(pp, stream);
            break;
        case EMessageId_SendSceneActionList:
            handleSceneActionList(pp, stream);
            break;
        case EMessageId_TransferResources:
            handleTransferResources(pp, stream);
            break;
        case EMessageId_RequestResources:
            handleRequestResources(pp, stream);
            break;
        case EMessageId_ResourcesNotAvailable:
            handleResourcesNotAvailable(pp, stream);
            break;
        case EMessageId_CreateScene:
            handleCreateScene(pp, stream);
            break;
        case EMessageId_DcsmRegisterContent:
            handleDcsmRegisterContent(pp, stream);
            break;
        case EMessageId_DcsmCanvasSizeChange:
            handleDcsmCanvasSizeChange(pp, stream);
            break;
        case EMessageId_DcsmContentStatusChange:
            handleDcsmContentStatusChange(pp, stream);
            break;
        case EMessageId_DcsmContentAvailable:
            handleDcsmContentAvailable(pp, stream);
            break;
        case EMessageId_DcsmCategoryContentSwitchRequest:
            handleDcsmCategoryContentSwitchRequest(pp, stream);
            break;
        case EMessageId_DcsmRequestUnregisterContent:
            handleDcsmRequestUnregisterContent(pp, stream);
            break;
        case EMessageId_DcsmForceUnregisterContent:
            handleDcsmForceStopOfferContent(pp, stream);
            break;
        default:
            LOG_ERROR(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleReceivedMessage: Invalid messagetype " << messageType << " From " << pp->address.getParticipantId());
            removeParticipant(pp);
        }
    }

    void TCPConnectionSystem::sendConnectionDescriptionOnNewConnection(const ParticipantPtr& pp)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendConnectionDescriptionOnNewConnection: " << pp->address.getParticipantId());

        OutMessage msg(pp->address.getParticipantId(), EMessageId_ConnectionDescriptionMessage);
        msg.stream << m_protocolVersion
                   << m_participantAddress.getParticipantId()
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
            removeParticipant(pp);
            return;
        }

        // check protocol version first before reading anything else. allows graceful protocol updates also of connection description message
        uint32_t protocolVersion = 0;
        stream >> protocolVersion;
        if (m_protocolVersion != protocolVersion)
        {
            LOG_WARN(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleConnectionDescriptionMessage: Invalid protocol version (expected "
                     << m_protocolVersion << ", got " << protocolVersion << ") on new connection. Drop connection");
            removeParticipant(pp);
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
        {
            m_ramsesConnectionStatusUpdateNotifier.triggerNotification(guid, EConnectionStatus_Connected);
            m_dcsmConnectionStatusUpdateNotifier.triggerNotification(guid, EConnectionStatus_Connected);
        }

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

                OutMessage msg(newPp->address.getParticipantId(), EMessageId_ConnectorAddressExchange);
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
                postMessageForSending(std::move(msg), true);
            }

            // send new participant to all others, inc daemons
            for (const auto p : m_establishedParticipants)
            {
                if (newPp != p.value)
                {
                    LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendConnectorAddressExchangeMessagesForNewParticipant: Send "
                              << newPp->address.getParticipantId() << "/" << newPp->address.getParticipantName() << " to "
                              << p.value->address.getParticipantId() << "/" << p.value->address.getParticipantName());

                    OutMessage msg(p.value->address.getParticipantId(), EMessageId_ConnectorAddressExchange);
                    const NetworkParticipantAddress& addr = newPp->address;
                    msg.stream << static_cast<uint32_t>(1)
                               << addr.getParticipantId()
                               << addr.getParticipantName()
                               << addr.getIp()
                               << addr.getPort()
                               << newPp->type;
                    postMessageForSending(std::move(msg), true);
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
        OutMessage msg(to, EMessageId_SubscribeScene);
        msg.stream << sceneId.getValue();
        return postMessageForSending(std::move(msg), true);
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
        OutMessage msg(to, EMessageId_UnsubscribeScene);
        msg.stream << sceneId.getValue();
        return postMessageForSending(std::move(msg), true);
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
    bool TCPConnectionSystem::sendInitializeScene(const Guid& to, const SceneInfo& sceneInfo)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendInitializeScene: to " << to << ", sceneId " << sceneInfo.sceneID << ", name " << sceneInfo.friendlyName);
        OutMessage msg(to, EMessageId_CreateScene);
        msg.stream << sceneInfo.sceneID.getValue()
                   << sceneInfo.friendlyName;
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleCreateScene(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            SceneInfo sceneInfo;
            stream >> sceneInfo.sceneID.getReference()
                   >> sceneInfo.friendlyName;

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleCreateScene: from " << pp->address.getParticipantId() <<
                      ", sceneId " << sceneInfo.sceneID << ", name " << sceneInfo.friendlyName);
            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleInitializeScene(sceneInfo, pp->address.getParticipantId());
        }
    }

    // --
    uint64_t TCPConnectionSystem::sendSceneActionList(const Guid& to, const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& counterStart)
    {
        uint64_t numberOfChunks = 0u;

        auto sendChunk =
            [&](std::pair<uint32_t, uint32_t> actionRange, std::pair<const Byte*, const Byte*> dataRange, bool isIncomplete)
        {
            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem::sendSceneActionList: to " << to <<
                ", sceneId " << sceneId.getValue() << ", actions [" << actionRange.first << ", " << actionRange.second <<
                ") from " << actions.numberOfActions());


            OutMessage msg(to, EMessageId_SendSceneActionList);
            msg.stream << static_cast<uint32_t>(actionRange.second - actionRange.first)
                       << static_cast<uint32_t>(dataRange.second - dataRange.first)
                       << sceneId.getValue();

            const uint32_t actionOffsetBase = actions[actionRange.first].offsetInCollection();
            for (uint32_t idx = actionRange.first; idx < actionRange.second; ++idx)
            {
                const SceneActionCollection::SceneActionReader reader(actions[idx]);
                ESceneActionId type = (idx == actionRange.second - 1 && isIncomplete) ? ESceneActionId_Incomplete : reader.type();
                msg.stream << static_cast<uint32_t>(type);
                msg.stream << reader.offsetInCollection() - actionOffsetBase;
            }
            msg.stream.write(dataRange.first, static_cast<uint32_t>(dataRange.second - dataRange.first));

            msg.stream << (counterStart + numberOfChunks);
            numberOfChunks++;

            return postMessageForSending(std::move(msg), true);
        };

        TransportUtilities::SplitSceneActionsToChunks(actions, m_sendDataSizes.sceneActionNumber, m_sendDataSizes.sceneActionDataArray, sendChunk);
        return numberOfChunks;
    }

    void TCPConnectionSystem::handleSceneActionList(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            uint32_t numActions = 0;
            stream >> numActions;
            uint32_t actionDataSize = 0;
            stream >> actionDataSize;

            SceneId sceneId;
            stream >> sceneId.getReference();

            SceneActionCollection actions(actionDataSize, numActions);
            for (uint32_t i = 0; i < numActions; ++i)
            {
                uint32_t type = 0;
                uint32_t offset = 0;
                stream >> type;
                stream >> offset;
                actions.addRawSceneActionInformation(static_cast<ESceneActionId>(type), offset);
            }

            std::vector<Byte>& rawActionData = actions.getRawDataForDirectWriting();
            rawActionData.resize(actionDataSize);
            stream.read(rawActionData.data(), actionDataSize);

            uint64_t sceneactionListCounter = 0;
            stream >> sceneactionListCounter;

            LOG_TRACE(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleSceneActionList: from " << pp->address.getParticipantId() <<
                      " numActions " << numActions << ", size " << actionDataSize << ", sceneactionCounter " << sceneactionListCounter);

            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleSceneActionList(sceneId, std::move(actions), sceneactionListCounter, pp->address.getParticipantId());
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

        OutMessage msg(Guid(false), EMessageId_PublishScene);
        msg.stream << static_cast<uint32_t>(newScenes.size());
        for (const auto& s : newScenes)
        {
            msg.stream << s.sceneID.getValue()
                       << s.friendlyName;
        }
        return postMessageForSending(std::move(msg), true);
    }

    bool TCPConnectionSystem::sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendScenesAvailable: to " << to << " [";
                                                for (const auto& s : availableScenes)
                                                    sos << s.sceneID << "/" << s.friendlyName << "; ";
                                                sos << "]";
                                            }));

        OutMessage msg(to, EMessageId_PublishScene);
        msg.stream << static_cast<uint32_t>(availableScenes.size());
        for (const auto& s : availableScenes)
        {
            msg.stream << s.sceneID.getValue()
                       << s.friendlyName;
        }
        return postMessageForSending(std::move(msg), true);
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
            m_sceneRendererHandler->handleNewScenesAvailable(newScenes, pp->address.getParticipantId(), EScenePublicationMode_LocalAndRemote);
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

        OutMessage msg(Guid(false), EMessageId_UnpublishScene);
        msg.stream << static_cast<uint32_t>(unavailableScenes.size());
        for (const auto& s : unavailableScenes)
        {
            msg.stream << s.sceneID.getValue()
                       << s.friendlyName;
        }
        return postMessageForSending(std::move(msg), true);
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
    bool TCPConnectionSystem::sendSceneNotAvailable(const Guid& to, const SceneId& sceneId)
    {
        LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendSceneNotAvailable: to " << to << ", sceneId " << sceneId);
        OutMessage msg(to, EMessageId_SceneNotAvailable);
        msg.stream << sceneId.getValue();
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleSceneNotAvailable(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_sceneRendererHandler)
        {
            SceneId sceneId;
            stream >> sceneId.getReference();

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleSceneNotAvailable: from " << pp->address.getParticipantId() << ", sceneId " << sceneId);
            PlatformGuard guard(m_frameworkLock);
            m_sceneRendererHandler->handleSceneNotAvailable(sceneId, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendRequestResources(const Guid& to, const ResourceContentHashVector& resources)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendRequestResources: to " << to << " [";
                                                for (const auto& r : resources)
                                                    sos << r << "; ";
                                                sos << "]";
                                            }));

        OutMessage msg(to, EMessageId_RequestResources);
        msg.stream << static_cast<uint32_t>(resources.size());
        for (const auto& r : resources)
        {
            msg.stream << r;
        }
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleRequestResources(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_resourceProviderHandler)
        {
            uint32_t numResources = 0;
            stream >> numResources;

            ResourceContentHashVector resources(numResources);
            for (uint32_t i = 0; i < numResources; ++i)
                stream >> resources[i];

            LOG_TRACE_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                    sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleRequestResources: from " << pp->address.getParticipantId() << " [";
                                                    for (const auto& r : resources)
                                                        sos << r << "; ";
                                                    sos << "]";
                                                }));

            PlatformGuard guard(m_frameworkLock);
            m_resourceProviderHandler->handleRequestResources(resources, 0u, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendResources(const Guid& to, const ManagedResourceVector& managedResources)
    {
        LOG_TRACE_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendResources: to " << to << " [";
                                                for (const auto& mr : managedResources)
                                                {
                                                    const IResource* r = mr.getResourceObject();
                                                    sos << r->getHash() << "/" << r->getName() << "; ";
                                                }
                                                sos << "]";
                                            }));

        if (!m_runState)
            return false;

        // try to compress for network sending
        for (const auto& managedResource : managedResources)
        {
            const IResource* resource = managedResource.getResourceObject();
            assert(resource != NULL);
            resource->compress(IResource::CompressionLevel::REALTIME);
        }

        std::vector<Byte> buffer;
        bool result = true;

        auto preparePacketFun = [&](UInt32 neededSize) -> std::pair<Byte*, UInt32> {
            buffer.resize(std::min(m_sendDataSizes.resourceDataArray, neededSize));
            return std::make_pair(buffer.data(), static_cast<UInt32>(buffer.size()));
        };

        auto finishedPacketFun = [&](UInt32 usedSize) {
            assert(usedSize <= buffer.size());

            OutMessage msg(to, EMessageId_TransferResources);
            msg.stream << usedSize;
            msg.stream.write(buffer.data(), usedSize);
            result &= postMessageForSending(std::move(msg), false);

            m_statisticCollection.statResourcesSentSize.incCounter(usedSize);
        };

        ResourceStreamSerializer serializer;
        serializer.serialize(preparePacketFun, finishedPacketFun, managedResources);

        return result;
    }

    void TCPConnectionSystem::handleTransferResources(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_resourceConsumerHandler)
        {
            uint32_t dataSize;
            stream >> dataSize;

            const char* receiveBufferEnd = pp->receiveBuffer.data() + pp->receiveBuffer.size();
            ByteArrayView resourceData(reinterpret_cast<const Byte*>(stream.readPosition()), static_cast<uint32_t>(receiveBufferEnd - stream.readPosition()));

            LOG_DEBUG(CONTEXT_COMMUNICATION, "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleTransferResources: from " << pp->address.getParticipantId() << ", dataSize " << dataSize);
            PlatformGuard guard(m_frameworkLock);
            m_resourceConsumerHandler->handleSendResource(resourceData, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendResourcesNotAvailable(const Guid& to, const ResourceContentHashVector& resources)
    {
        LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::sendResourcesNotAvailable: to " << to << " [";
                                                for (const auto& r : resources)
                                                    sos << r << "; ";
                                                sos << "]";
                                            }));

        OutMessage msg(Guid(false), EMessageId_ResourcesNotAvailable);
        msg.stream << static_cast<uint32_t>(resources.size());
        for (const auto& r : resources)
        {
            msg.stream << r;
        }
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleResourcesNotAvailable(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_resourceConsumerHandler)
        {
            uint32_t numResources = 0;
            stream >> numResources;

            ResourceContentHashVector resources(numResources);
            for (uint32_t i = 0; i < numResources; ++i)
                stream >> resources[i];

            LOG_DEBUG_F(CONTEXT_COMMUNICATION, ([&](ramses_internal::StringOutputStream& sos) {
                                                    sos << "TCPConnectionSystem(" << m_participantAddress.getParticipantName() << ")::handleResourcesNotAvailable: from " << pp->address.getParticipantId() << " [";
                                                    for (const auto& r : resources)
                                                        sos << r << "; ";
                                                    sos << "]";
                                                }));

            PlatformGuard guard(m_frameworkLock);
            m_resourceConsumerHandler->handleResourcesNotAvailable(resources, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmCanvasSizeChange(const Guid& to, ContentID contentID, SizeInfo sizeinfo, AnimationInformation ai)
    {
        OutMessage msg(to, EMessageId_DcsmCanvasSizeChange);
        msg.stream << contentID.getValue()
                   << sizeinfo.width
                   << sizeinfo.height
                   << ai.startTimeStamp
                   << ai.finishedTimeStamp;
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleDcsmCanvasSizeChange(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmProviderHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            SizeInfo sizeInfo;
            stream >> sizeInfo.width;
            stream >> sizeInfo.height;

            AnimationInformation ai;
            stream >> ai.startTimeStamp;
            stream >> ai.finishedTimeStamp;

            PlatformGuard guard(m_frameworkLock);
            m_dcsmProviderHandler->handleCanvasSizeChange(contentID, sizeInfo, ai, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmContentStateChange(const Guid& to, ContentID contentID, EDcsmState status, SizeInfo si, AnimationInformation ai)
    {
        OutMessage msg(to, EMessageId_DcsmContentStatusChange);
        msg.stream << contentID.getValue()
                   << status
                   << si.width
                   << si.height
                   << ai.startTimeStamp
                   << ai.finishedTimeStamp;
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleDcsmContentStatusChange(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmProviderHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            EDcsmState statusInfo;
            stream >> statusInfo;

            SizeInfo si;
            stream >> si.width;
            stream >> si.height;

            AnimationInformation ai;
            stream >> ai.startTimeStamp;
            stream >> ai.finishedTimeStamp;

            PlatformGuard guard(m_frameworkLock);
            m_dcsmProviderHandler->handleContentStateChange(contentID, statusInfo, si, ai, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmBroadcastOfferContent(ContentID contentID, Category category)
    {
        OutMessage msg(Guid(false), EMessageId_DcsmRegisterContent);
        msg.stream << contentID.getValue()
                   << category.getValue();
        return postMessageForSending(std::move(msg), true);
    }

    bool TCPConnectionSystem::sendDcsmOfferContent(const Guid& to, ContentID contentID, Category category)
    {
        OutMessage msg(Guid(to), EMessageId_DcsmRegisterContent);
        msg.stream << contentID.getValue()
                   << category.getValue();
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleDcsmRegisterContent(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            Category category;
            stream >> category.getReference();

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleOfferContent(contentID, category, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmContentReady(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor)
    {
        OutMessage msg(to, EMessageId_DcsmContentAvailable);
        msg.stream << contentID.getValue()
                   << technicalContentType
                   << technicalContentDescriptor.getValue();
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleDcsmContentAvailable(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            ETechnicalContentType technicalContentType;
            stream >> technicalContentType;

            TechnicalContentDescriptor technicalContentDescriptor;
            stream >> technicalContentDescriptor.getReference();

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleContentReady(contentID, technicalContentType, technicalContentDescriptor, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmContentFocusRequest(const Guid& to, ContentID contentID)
    {
        OutMessage msg(to, EMessageId_DcsmCategoryContentSwitchRequest);
        msg.stream << contentID.getValue();
        return postMessageForSending(std::move(msg), true);
    }

    void TCPConnectionSystem::handleDcsmCategoryContentSwitchRequest(const ParticipantPtr& pp, BinaryInputStream& stream)
    {
        if (m_dcsmConsumerHandler)
        {
            ContentID contentID;
            stream >> contentID.getReference();

            PlatformGuard guard(m_frameworkLock);
            m_dcsmConsumerHandler->handleContentFocusRequest(contentID, pp->address.getParticipantId());
        }
    }

    // --
    bool TCPConnectionSystem::sendDcsmBroadcastRequestStopOfferContent(ContentID contentID)
    {
        OutMessage msg(Guid(false), EMessageId_DcsmRequestUnregisterContent);
        msg.stream << contentID.getValue();
        return postMessageForSending(std::move(msg), true);
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
        OutMessage msg(Guid(false), EMessageId_DcsmForceUnregisterContent);
        msg.stream << contentID.getValue();
        return postMessageForSending(std::move(msg), true);
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


    // --- ramsh command handling ---
    void TCPConnectionSystem::logConnectionInfo()
    {
        // Keep RunState alive between check and use
        RunStatePtr rs = m_runState;
        if (!rs)
            return;

        asio::post(rs->m_io, [this]() {
                            LOG_INFO_F(CONTEXT_PERIODIC,
                                       ([&](StringOutputStream& sos)
                                        {
                                            // general info
                                            sos << "TCPConnectionSystem:\n";
                                            sos << "  Self: " << m_participantAddress.getParticipantName() << " / " << m_participantAddress.getParticipantId() << "\n";
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
            });
    }

    void TCPConnectionSystem::triggerLogMessageForPeriodicLog()
    {
        // Keep RunState alive between check and use
        RunStatePtr rs = m_runState;
        if (!rs)
            return;

        asio::post(rs->m_io, [this]() {
                            LOG_INFO_F(CONTEXT_PERIODIC,
                                       ([&](StringOutputStream& sos)
                                        {
                                            sos << "Connected Participant(s): ";
                                            if (m_establishedParticipants.count() == 0)
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

    // --- TCPConnectionSystem::Participant ---
    TCPConnectionSystem::Participant::Participant(const NetworkParticipantAddress& address_, asio::io_service& io_,
                                                  EParticipantType type_, EParticipantState state_)
        : address(address_)
        , socket(io_)
        , connectTimer(io_)
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
