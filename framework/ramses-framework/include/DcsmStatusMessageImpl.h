//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMSTATUSMESSAGEIMPL_H
#define RAMSES_DCSMSTATUSMESSAGEIMPL_H

#include "ramses-framework-api/DcsmStatusMessage.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/RawBinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "PlatformAbstraction/FmtBase.h"

#include <vector>
#include "absl/types/span.h"

namespace ramses
{
    class DcsmStatusMessageImpl
    {
    public:
        enum class Type : uint64_t
        {
            StreamStatus,
            ActiveLayout,
            WidgetFocusStatus,

            NumElements
        };

        DcsmStatusMessageImpl(Type type, size_t size);
        DcsmStatusMessageImpl(Type type, absl::Span<const ramses_internal::Byte> message);

        static std::unique_ptr<DcsmStatusMessage> CreateMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl);

        static bool isValidType(uint64_t messageId);

        Type getType() const;
        std::vector<ramses_internal::Byte> const& getData() const;

        ramses_internal::RawBinaryOutputStream getOStream();
        ramses_internal::BinaryInputStream getIStream();

    private:
        Type m_type;
        std::vector<ramses_internal::Byte> m_data;
    };

    inline bool DcsmStatusMessageImpl::isValidType(uint64_t messageId)
    {
        return (messageId < static_cast<uint64_t>(Type::NumElements));
    }
}

template <> struct fmt::formatter<ramses::DcsmStatusMessageImpl> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const ramses::DcsmStatusMessageImpl& msg, FormatContext& ctx)
    {
        fmt::format_to(ctx.out(), "[");
        switch (msg.getType())
        {
        case ramses::DcsmStatusMessageImpl::Type::StreamStatus:
            fmt::format_to(ctx.out(), "StreamStatus: ");
            break;
        case ramses::DcsmStatusMessageImpl::Type::ActiveLayout:
            fmt::format_to(ctx.out(), "ActiveLayout: ");
            break;
        case ramses::DcsmStatusMessageImpl::Type::WidgetFocusStatus:
            fmt::format_to(ctx.out(), "WidgetFocusStatus: ");
            break;
        case ramses::DcsmStatusMessageImpl::Type::NumElements:
            assert(false);
            break;
        }

        if (msg.getData().size() <= 16)
        {
            fmt::format_to(ctx.out(), "{}", fmt::join(msg.getData(), ","));
        }
        else
        {
            fmt::format_to(ctx.out(), "<size:{}>", msg.getData().size());
        }
        return fmt::format_to(ctx.out(), "]");
    }
};

#endif
