//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/Components/ManagedResource.h"

namespace ramses::internal
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
            {
                res = std::make_unique<EffectResource>("", "", "", SPIRVShaders{}, std::optional<EDrawMode>{}, EffectInputInformationVector(), EffectInputInformationVector(), "", EFeatureLevel_Latest);
            }
            else if (hash == VertArrayHash || hash == VertArrayHash2)
            {
                res = std::make_unique<ArrayResource>(EResourceType::VertexArray, 0, EDataType::Float, nullptr, std::string_view{});
            }
            else if (hash == IndexArrayHash || hash == IndexArrayHash2 || hash == IndexArrayHash3)
            {
                res = std::make_unique<ArrayResource>(EResourceType::IndexArray, 0, EDataType::UInt16, nullptr, std::string_view{});
            }
            else if (hash == TextureHash)
            {
                res = std::make_unique<TextureResource>(EResourceType::Texture2D, TextureMetaInfo(1u, 1u, 1u, EPixelStorageFormat::R8, false, {}, { 1u }), std::string_view{});
            }
            else if (hash == TextureHash2)
            {
                res = std::make_unique<TextureResource>(EResourceType::Texture2D, TextureMetaInfo(2u, 2u, 1u, EPixelStorageFormat::R8, true, {}, {4u}), std::string_view{});
            }

            if (res)
                res->setResourceData(ResourceBlob{ 1 }, hash);

            return res;
        }
    }
}
