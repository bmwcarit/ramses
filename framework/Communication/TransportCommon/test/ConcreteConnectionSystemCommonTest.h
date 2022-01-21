//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "Collections/Guid.h"
#include "MockConnectionStatusListener.h"
#include "TransportCommon/SomeIPStackCommon.h"
#include "ConnectionSystemTestCommon.h"
#include "gmock/gmock.h"
#include <vector>
#include <cstdint>

namespace ramses_internal
{
    using namespace ::testing;

    class ConcreteConnectionSystemCommonTest
    {
    public:
        struct Config
        {
            uint16_t serviceIid;
            uint16_t remoteIid;
            uint16_t remoteMinorProtocolVersion;
            const char* desc;

            friend std::ostream& operator<<(std::ostream& os, const Config& tc) { return os << tc.desc; } // avoid valgrind googletest byte print issue due to padding
        };

        static std::vector<Config> ConfigValues()
        {
            return {{5, 6, 0, "old"},
                    {5, 6, 1, "responder"},
                    {5, 4, 1, "initiator"}};
        };

        static auto ConfigPrinter()
        {
            return [](const auto& info){ return info.param.desc; };
        }

        template <typename InstanceIdType, typename StackMock>
        uint64_t connectRemoteHelper(const Config& config,
                                     StackMock& stack,  ISomeIPStackCallbacksCommon<InstanceIdType>& fromStack,
                                     InstanceIdType remoteIidArg, const Guid& remotePid)
        {
            uint64_t activeSessionId = 123;
            if (config.remoteMinorProtocolVersion == 0)
            {
                EXPECT_CALL(stack, sendParticipantInfo(remoteIidArg, ValidHdr(pid, 1u), 99, _, InstanceIdType{config.serviceIid}, _, _, _)).WillOnce(Return(true));
                fromStack.handleServiceAvailable(remoteIidArg);
                EXPECT_CALL(connections, newParticipantHasConnected(remotePid));
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid.get(), 123, 1}, 99, config.remoteMinorProtocolVersion, remoteIidArg, 0, 0, 0);
            }
            else if (config.serviceIid < remoteIidArg.getValue())
            {
                // service is responder
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid.get(), 123, 1}, 99, config.remoteMinorProtocolVersion, remoteIidArg, 0, 0, 0);
                EXPECT_CALL(stack, sendParticipantInfo(remoteIidArg, ValidHdr(pid, 1u), 99, _, InstanceIdType{config.serviceIid}, _, _, _)).WillOnce(Return(true));
                EXPECT_CALL(connections, newParticipantHasConnected(remotePid));
                fromStack.handleServiceAvailable(remoteIidArg);
            }
            else
            {
                // service is initiator
                EXPECT_CALL(stack, sendParticipantInfo(remoteIidArg, ValidHdr(pid, 1u), 99, _, InstanceIdType{config.serviceIid}, _, _, _)).WillOnce(
                    Invoke([&](const auto&, const auto& header, const auto&, const auto&, const auto&, const auto&, const auto&, const auto&) {
                        activeSessionId = header.sessionId;
                        return true;
                    }));
                fromStack.handleServiceAvailable(remoteIidArg);
                EXPECT_CALL(connections, newParticipantHasConnected(remotePid));
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid.get(), activeSessionId, 1}, 99, config.remoteMinorProtocolVersion, remoteIidArg, 0, 0, 0);
            }
            return activeSessionId;
        }

        template <typename StackMock>
        void expectRemoteDisconnects(StackMock& stack, std::initializer_list<uint64_t> guids)
        {
            EXPECT_CALL(stack, sendKeepAlive(_, _, _, _)).Times(AnyNumber()); // responders will send error
            for (auto g : guids)
                EXPECT_CALL(connections, participantHasDisconnected(Guid(g)));
        }

        StrictMock<MockConnectionStatusListener> connections;
        uint64_t pid{4};
    };
}
