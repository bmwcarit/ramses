//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MOCKRESOURCEHASH_H
#define RAMSES_MOCKRESOURCEHASH_H

#include "SceneAPI/ResourceContentHash.h"
#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "Resource/TextureResource.h"

namespace ramses_internal
{
namespace MockResourceHash
{
    static constexpr ResourceContentHash EffectHash{ 120u, 0u };
    static constexpr ResourceContentHash VertArrayHash{ 123u, 0u };
    static constexpr ResourceContentHash VertArrayHash2{ 124u, 0u };
    static constexpr ResourceContentHash IndexArrayHash{ 125u, 0u };
    static constexpr ResourceContentHash IndexArrayHash2{ 126u, 0u };
    static constexpr ResourceContentHash IndexArrayHash3{ 127u, 0u };
    static constexpr ResourceContentHash TextureHash{ 128u, 0u };
    static constexpr ResourceContentHash TextureHash2{ 129u, 0u };

    static inline ManagedResource GetManagedResource(const ResourceContentHash& hash)
    {
        std::unique_ptr<ResourceBase> res;
        if (hash == EffectHash)
            res = std::make_unique<EffectResource>("", "", "", EDrawMode::NUMBER_OF_ELEMENTS, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
        else if (hash == VertArrayHash || hash == VertArrayHash2)
            res = std::make_unique<ArrayResource>(EResourceType_VertexArray, 0, EDataType::Float, nullptr, ResourceCacheFlag_DoNotCache, String());
        else if (hash == IndexArrayHash || hash == IndexArrayHash2 || hash == IndexArrayHash3)
            res = std::make_unique<ArrayResource>(EResourceType_IndexArray, 0, EDataType::UInt16, nullptr, ResourceCacheFlag_DoNotCache, String());
        else if (hash == TextureHash)
            res = std::make_unique<TextureResource>(EResourceType_Texture2D, TextureMetaInfo(1u, 1u, 1u, ETextureFormat::R8, false, {}, { 1u }), ResourceCacheFlag_DoNotCache, String());
        else if (hash == TextureHash2)
            res = std::make_unique<TextureResource>(EResourceType_Texture2D, TextureMetaInfo(2u, 2u, 1u, ETextureFormat::R8, true, {}, { 4u }), ResourceCacheFlag_DoNotCache, String());

        if (res)
            res->setResourceData(ResourceBlob{ 1 }, hash);

        return res;
    }
}
}

#endif
