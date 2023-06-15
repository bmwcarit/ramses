//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATIONSYSTEMTESTWRAPPER_H
#define RAMSES_COMMUNICATIONSYSTEMTESTWRAPPER_H

#include "TransportCommon/ICommunicationSystem.h"
#include "MockConnectionStatusListener.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Utils/StatisticCollection.h"
#include "ServiceHandlerMocks.h"
#include "ConnectionSystemTestHelper.h"
#include "Collections/Guid.h"

#include <string_view>

namespace ramses_internal
{
    enum class ECommunicationSystemType
    {
        Tcp,
    };

    enum class EServiceType
    {
        Ramses
    };

    // better output from gtest
    void PrintTo(const ECommunicationSystemType& type, std::ostream* os);

    class CommunicationSystemTestWrapper;

    class CommunicationSystemTestState
    {
    public:
        explicit CommunicationSystemTestState(ECommunicationSystemType type, EServiceType serviceType);
        virtual ~CommunicationSystemTestState();

        void connectAll();
        void disconnectAll();
        testing::AssertionResult blockOnAllConnected(uint32_t waitTimeMsOverride = 0);

        void sendEvent();

        static std::vector<ECommunicationSystemType> GetAvailableCommunicationSystemTypes();
        ECommunicationSystemType communicationSystemType;
        EServiceType serviceType;
        AsyncEventCounter event;
        std::vector<CommunicationSystemTestWrapper*> knownCommunicationSystems;
    };

    class CommunicationSystemTestWrapper
    {
    public:
        explicit CommunicationSystemTestWrapper(CommunicationSystemTestState& state_, std::string_view name = {}, const Guid& id_ = Guid());
        virtual ~CommunicationSystemTestWrapper();

        virtual void registerAsEventReceiver() {}

        void registerForConnectionUpdates();
        void unregisterForConnectionUpdates();

        Guid id;
        PlatformLock frameworkLock;
        MockConnectionStatusListener statusUpdateListener;
        std::unique_ptr<ICommunicationSystem> commSystem;
        StatisticCollectionFramework statisticCollection;

    protected:
        CommunicationSystemTestState& state;
    };
}

#endif
