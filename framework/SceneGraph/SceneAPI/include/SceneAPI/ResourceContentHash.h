//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECONTENTHASH_H
#define RAMSES_RESOURCECONTENTHASH_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Collections/StringOutputStream.h"
#include "ramses-capu/container/Hash.h"
#include <cinttypes>
#include <functional>

namespace ramses_internal
{
    struct ResourceContentHash
    {
        constexpr ResourceContentHash()
            : lowPart(0)
            , highPart(0)
        {
        }

        constexpr ResourceContentHash(UInt64 low, UInt64 high)
            : lowPart(low)
            , highPart(high)
        {
        }

        constexpr static ResourceContentHash Invalid()
        {
            return ResourceContentHash();
        }

        constexpr inline bool isValid() const
        {
            return *this != Invalid();
        }

        constexpr inline bool operator==(const ResourceContentHash& rhs) const
        {
            return (lowPart == rhs.lowPart && highPart == rhs.highPart);
        }

        constexpr inline bool operator!=(const ResourceContentHash& rhs) const
        {
            return !(*this == rhs);
        }

        UInt64 lowPart;
        UInt64 highPart;
    };

    constexpr inline bool operator<(ResourceContentHash const& lhs, ResourceContentHash const& rhs)
    {
        return lhs.highPart == rhs.highPart ? lhs.lowPart < rhs.lowPart : lhs.highPart < rhs.highPart;
    }

    inline IOutputStream& operator<<(IOutputStream& stream, const ResourceContentHash& value)
    {
        return stream << value.lowPart << value.highPart;
    }

    inline IInputStream& operator>>(IInputStream& stream, ResourceContentHash& value)
    {
        return stream >> value.lowPart >> value.highPart;
    }

    inline StringOutputStream& operator<<(StringOutputStream& sos, const ResourceContentHash& value)
    {
        sos.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        sos << "0x" << value.highPart << value.lowPart;
        sos.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_NoHex);
        return sos;
    }
}

// make hashable
namespace std
{
    template <>
    struct hash<ramses_internal::ResourceContentHash>
    {
    public:
        size_t operator()(const ramses_internal::ResourceContentHash& v) const
        {
            static_assert(sizeof(ramses_internal::ResourceContentHash) == 2*sizeof(uint64_t), "make sure resourceontenthash is just 2 64 values");
            return ramses_capu::HashMemoryRange(&v, 2 * sizeof(uint64_t));
        }
    };
}


#endif
