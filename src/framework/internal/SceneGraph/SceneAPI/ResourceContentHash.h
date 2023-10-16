//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Hash.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/PlatformAbstraction/FmtBase.h"
#include "internal/SceneGraph/Resource/ResourceTypes.h"

#include <cstdint>
#include <cinttypes>
#include <functional>
#include <vector>

namespace ramses::internal
{
    struct ResourceContentHash
    {
        constexpr ResourceContentHash() = default;

        constexpr ResourceContentHash(uint64_t low, uint64_t high)
            : lowPart(low)
            , highPart(high)
        {
        }

        static constexpr ResourceContentHash Invalid()
        {
            return ResourceContentHash();
        }

        [[nodiscard]] constexpr inline bool isValid() const
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

        uint64_t lowPart{0u};
        uint64_t highPart{0u};
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

    using ResourceContentHashVector = std::vector<ResourceContentHash>;
}

template <>
struct fmt::formatter<ramses::internal::ResourceContentHash> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::internal::ResourceContentHash& res, FormatContext& ctx)
    {
        const char* typeShortString = nullptr;
        const auto type = static_cast<uint32_t>((res.highPart >> 60LU) & 0xFu);
        switch (type)
        {
        case static_cast<uint32_t>(ramses::internal::EResourceType::VertexArray):
            typeShortString = "vtx";
            break;
        case static_cast<uint32_t>(ramses::internal::EResourceType::IndexArray):
            typeShortString = "idx";
            break;
        case static_cast<uint32_t>(ramses::internal::EResourceType::Texture2D):
            typeShortString = "tx2";
            break;
        case static_cast<uint32_t>(ramses::internal::EResourceType::Texture3D):
            typeShortString = "tx3";
            break;
        case static_cast<uint32_t>(ramses::internal::EResourceType::TextureCube):
            typeShortString = "txc";
            break;
        case static_cast<uint32_t>(ramses::internal::EResourceType::Effect):
            typeShortString = "eff";
            break;
        default:
            typeShortString = "inv";
        }
        return fmt::format_to(ctx.out(), "{}_{:016X}{:016X}", typeShortString, res.highPart, res.lowPart);
    }
};

template <>
struct fmt::formatter<ramses::internal::ResourceContentHashVector> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::internal::ResourceContentHashVector& hashes, FormatContext& ctx)
    {
        fmt::format_to(ctx.out(), "[{} resources:", hashes.size());
        for (auto const& hash : hashes)
            fmt::format_to(ctx.out(), " {}", hash);

        return fmt::format_to(ctx.out(), "]");
    }
};

// make hashable
namespace std
{
    template <>
    struct hash<ramses::internal::ResourceContentHash>
    {
    public:
        size_t operator()(const ramses::internal::ResourceContentHash& v) const
        {
            static_assert(sizeof(ramses::internal::ResourceContentHash) == 2*sizeof(uint64_t), "make sure resourceontenthash is just 2 64 values");
            return ramses::internal::HashMemoryRange(&v, 2 * sizeof(uint64_t));
        }
    };
}

