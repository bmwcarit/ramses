//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include <functional>

namespace ramses::internal
{
    struct TextureSamplerStates
    {
        TextureSamplerStates()
            : TextureSamplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureAddressMode::Clamp,
                                   ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Linear_MipMapLinear)
        {}

        TextureSamplerStates(ETextureAddressMode addressModeU,
                             ETextureAddressMode addressModeV,
                             ETextureAddressMode addressModeR,
                             ETextureSamplingMethod minSamplingMode,
                             ETextureSamplingMethod magSamplingMode,
                             uint32_t anisotropyLevel = 1u)
            : m_addressModeU(addressModeU)
            , m_addressModeV(addressModeV)
            , m_addressModeR(addressModeR)
            , m_minSamplingMode(minSamplingMode)
            , m_magSamplingMode(magSamplingMode)
            , m_anisotropyLevel(anisotropyLevel)
        {
        }

        [[nodiscard]] uint64_t hash() const
        {
            static_assert(sizeof(ETextureAddressMode) == 1u, "Unexpected size for enum used for shift operations");
            static_assert(sizeof(ETextureSamplingMethod) == 1u, "Unexpected size for enum used for shift operations");

            return static_cast<uint64_t>(m_addressModeU)
                | (static_cast<uint64_t>(m_addressModeV)       << 8u)
                | (static_cast<uint64_t>(m_addressModeR)       << 16u)
                | (static_cast<uint64_t>(m_minSamplingMode)    << 24u)
                | (static_cast<uint64_t>(m_magSamplingMode)    << 32u)
                | (static_cast<uint64_t>(m_anisotropyLevel)    << 40u);

        }

        ETextureAddressMode     m_addressModeU;
        ETextureAddressMode     m_addressModeV;
        ETextureAddressMode     m_addressModeR;
        ETextureSamplingMethod m_minSamplingMode;
        ETextureSamplingMethod m_magSamplingMode;
        uint32_t          m_anisotropyLevel;
    };
}
