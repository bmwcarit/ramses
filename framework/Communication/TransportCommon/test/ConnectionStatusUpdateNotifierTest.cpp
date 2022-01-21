//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "MockConnectionStatusListener.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    using namespace ::testing;

    class AConnectionStatusUpdateNotifier : public Test
    {
    public:

        PlatformLock lock;
        StrictMock<MockConnectionStatusListener> listener;
    };

    TEST_F(AConnectionStatusUpdateNotifier, doesNotNotifyAnyoneIfNooneRegistered)
    {
        ConnectionStatusUpdateNotifier notifier(std::string(), CONTEXT_COMMUNICATION, std::string(), lock);
        notifier.triggerNotification(Guid(123), EConnectionStatus_Connected);
    }

    TEST_F(AConnectionStatusUpdateNotifier, doesNotNotifyListenerOfNooneConnected)
    {
        ConnectionStatusUpdateNotifier notifier(std::string(), CONTEXT_COMMUNICATION, std::string(), lock);
        notifier.registerForConnectionUpdates(&listener);
        notifier.unregisterForConnectionUpdates(&listener);
    }

    TEST_F(AConnectionStatusUpdateNotifier, notifiesAboutAllCurrentlyConnectedParticipants)
    {
        ConnectionStatusUpdateNotifier notifier(std::string(), CONTEXT_COMMUNICATION, std::string(), lock);
        Guid p1(1);
        Guid p2(2);
        notifier.triggerNotification(p1, EConnectionStatus_Connected);
        notifier.triggerNotification(p2, EConnectionStatus_Connected);

        EXPECT_CALL(listener, newParticipantHasConnected(p1));
        EXPECT_CALL(listener, newParticipantHasConnected(p2));
        notifier.registerForConnectionUpdates(&listener);
    }

    TEST_F(AConnectionStatusUpdateNotifier, notifiesAboutNewlyConnectedParticipants)
    {
        ConnectionStatusUpdateNotifier notifier(std::string(), CONTEXT_COMMUNICATION, std::string(), lock);
        notifier.registerForConnectionUpdates(&listener);

        Guid p1(3);
        EXPECT_CALL(listener, newParticipantHasConnected(p1));
        notifier.triggerNotification(p1, EConnectionStatus_Connected);
    }

    TEST_F(AConnectionStatusUpdateNotifier, doesNotNotifyAfterUnregister)
    {
        ConnectionStatusUpdateNotifier notifier(std::string(), CONTEXT_COMMUNICATION, std::string(), lock);
        notifier.registerForConnectionUpdates(&listener);
        notifier.unregisterForConnectionUpdates(&listener);

        EXPECT_CALL(listener, newParticipantHasConnected(_)).Times(0);
        notifier.triggerNotification(Guid(4), EConnectionStatus_Connected);
    }

    TEST_F(AConnectionStatusUpdateNotifier, notifiesAboutDisconnectingParticipants)
    {
        ConnectionStatusUpdateNotifier notifier(std::string(), CONTEXT_COMMUNICATION, std::string(), lock);

        Guid p(5);
        EXPECT_CALL(listener, newParticipantHasConnected(p));
        notifier.triggerNotification(p, EConnectionStatus_Connected);
        notifier.registerForConnectionUpdates(&listener);
        Mock::VerifyAndClearExpectations(&listener);

        EXPECT_CALL(listener, participantHasDisconnected(p));
        notifier.triggerNotification(p, EConnectionStatus_NotConnected);
    }

    TEST_F(AConnectionStatusUpdateNotifier, doesNotNotifyForAlreadyDisconnectedParticipants)
    {
        ConnectionStatusUpdateNotifier notifier(std::string(), CONTEXT_COMMUNICATION, std::string(), lock);

        Guid p(6);
        notifier.triggerNotification(p, EConnectionStatus_Connected);
        notifier.triggerNotification(p, EConnectionStatus_NotConnected);

        notifier.registerForConnectionUpdates(&listener);
    }
}
