//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKTYPESIMPL_H
#define RAMSES_RAMSESFRAMEWORKTYPESIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "PlatformAbstraction/Hash.h"
#include "Collections/StringOutputStream.h"
#include "Common/StronglyTypedValue.h"
#include "PlatformAbstraction/FmtBase.h"

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::sceneId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::dataConsumerId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::dataProviderId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::pickableObjectId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::displayId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::displayBufferId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::resourceCacheFlag_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::binaryShaderFormatId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::waylandIviSurfaceId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::waylandIviLayerId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::streamBufferId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::externalBufferId_t);

template <>
struct fmt::formatter<ramses::resourceId_t> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::resourceId_t& res, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "0x{:016X}:{:016X}", res.highPart, res.lowPart);
    }
};

namespace std
{
    template<>
    struct hash<::ramses::resourceId_t>
    {
        size_t operator()(const ::ramses::resourceId_t& rid) const
        {
            return ramses_internal::HashValue(rid.lowPart, rid.highPart);
        }
    };
}

#endif
