//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/PlatformAbstraction/Hash.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/Core/Common/StronglyTypedValue.h"
#include "internal/PlatformAbstraction/FmtBase.h"

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::sceneId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::dataConsumerId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::dataProviderId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::pickableObjectId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::displayId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::displayBufferId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::binaryShaderFormatId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::waylandIviSurfaceId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::waylandIviLayerId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::streamBufferId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::externalBufferId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::sceneObjectId_t);

template <>
struct fmt::formatter<ramses::resourceId_t> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::resourceId_t& res, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "0x{:016X}:{:016X}", res.highPart, res.lowPart);
    }
};

template <typename E>
struct fmt::formatter<ramses::Flags<E>> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::Flags<E>& value, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", value.value());
    }
};

namespace std
{
    template<>
    struct hash<::ramses::resourceId_t>
    {
        size_t operator()(const ::ramses::resourceId_t& rid) const
        {
            return ramses::internal::HashValue(rid.lowPart, rid.highPart);
        }
    };
}
