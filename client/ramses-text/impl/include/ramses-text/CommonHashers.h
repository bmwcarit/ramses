//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMONHASHERS_H
#define RAMSES_COMMONHASHERS_H

#include "ramses-framework-api/StronglyTypedValue.h"
#include "ramses-text-api/Glyph.h"
#include <functional>

namespace std
{
    template <typename _BaseType, typename _UniqueId>
    struct hash<ramses::StronglyTypedValue<_BaseType, _UniqueId>>
    {
    public:
        size_t operator()(const ramses::StronglyTypedValue<_BaseType, _UniqueId>& v) const
        {
            return static_cast<size_t>(hash<_BaseType>()(v.getValue()));
        }
    };

    template <>
    struct hash<ramses::GlyphKey>
    {
        size_t operator()(const ramses::GlyphKey& k) const
        {
            static_assert(sizeof(ramses::GlyphKey::identifier) <= sizeof(uint32_t), "Adapt hashing function!");
            static_assert(sizeof(ramses::GlyphKey::fontInstanceId) <= sizeof(uint32_t), "Adapt hashing function!");
            const uint64_t val = (uint64_t(k.identifier.getValue()) << 32) | k.fontInstanceId.getValue();
            return hash<uint64_t>()(val);
        }
    };
}

#endif
