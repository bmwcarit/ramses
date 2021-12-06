//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SOMEIPSTACKCOMMON_H
#define RAMSES_SOMEIPSTACKCOMMON_H

#include "Common/StronglyTypedValue.h"
#include "PlatformAbstraction/FmtBase.h"
#include <cstdint>

namespace ramses_internal
{
    class StringOutputStream;

    namespace SomeIPConstants
    {
        static constexpr uint32_t FallbackMinorProtocolVersion = 0;

        static constexpr const char* const ParticipantInfoExpectedReceiverPidKey = "%expectedReceiverPid";
        static constexpr const char* const ParticipantInfoMinorProtocolVersionKey = "%minorProtocolVersion";
        static constexpr const char* const KeepAliveUsingPreviousMessageIdKey = "%usingPreviousMessageId";
    }

    struct SomeIPMsgHeader
    {
        uint64_t participantId;
        uint64_t sessionId;
        uint64_t messageId;

        friend bool operator==(const SomeIPMsgHeader& a, const SomeIPMsgHeader& b)
        {
            return a.participantId == b.participantId && a.sessionId == b.sessionId && a.messageId == b.messageId;
        }
    };

    template <typename InstanceIdT>
    class ISomeIPStackCallbacksCommon
    {
    public:
        using InstanceIdType = InstanceIdT;

        virtual ~ISomeIPStackCallbacksCommon() = default;

        virtual void handleServiceAvailable(InstanceIdType iid) = 0;
        virtual void handleServiceUnavailable(InstanceIdType iid) = 0;

        virtual void handleParticipantInfo(const SomeIPMsgHeader& header, uint16_t protocolVersion, uint32_t minorProtocolVersion, InstanceIdType senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow) = 0;
        virtual void handleKeepAlive(const SomeIPMsgHeader& header, uint64_t timestampNow, bool usingPreviousMessageId) = 0;
    };

    template <typename InstanceIdT>
    class ISomeIPStackCommon
    {
    public:
        using InstanceIdType = InstanceIdT;

        virtual ~ISomeIPStackCommon() = default;

        virtual bool connect() = 0;
        virtual bool disconnect() = 0;
        virtual void logConnectionState(StringOutputStream& sos) = 0;

        virtual InstanceIdType getServiceInstanceId() const = 0;

        virtual bool sendParticipantInfo(InstanceIdType to, const SomeIPMsgHeader& header, uint16_t protocolVersion, uint32_t minorProtocolVersion, InstanceIdType senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow) = 0;
        virtual bool sendKeepAlive(InstanceIdType to, const SomeIPMsgHeader& header, uint64_t timestampNow, bool usingPreviousMessageId) = 0;
    };
}

template <>
struct fmt::formatter<ramses_internal::SomeIPMsgHeader> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::SomeIPMsgHeader& hdr, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "Hdr(pid:{} sid:{} mid:{})",
                              hdr.participantId, hdr.sessionId, hdr.messageId);
    }
};

#endif
