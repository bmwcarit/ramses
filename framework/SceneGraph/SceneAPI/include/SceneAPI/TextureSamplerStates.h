//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESAMPLERSTATES_H
#define RAMSES_TEXTURESAMPLERSTATES_H

#include "SceneAPI/TextureEnums.h"

namespace ramses_internal
{
    struct TextureSamplerStates
    {
        TextureSamplerStates()
            : TextureSamplerStates(EWrapMethod::Clamp, EWrapMethod::Clamp, EWrapMethod::Clamp,
                                   ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Linear_MipMapLinear)
        {}

        TextureSamplerStates(EWrapMethod addressModeU,
                             EWrapMethod addressModeV,
                             EWrapMethod addressModeR,
                             ESamplingMethod minSamplingMode,
                             ESamplingMethod magSamplingMode,
                             UInt32 anisotropyLevel = 1u)
            : m_addressModeU(addressModeU)
            , m_addressModeV(addressModeV)
            , m_addressModeR(addressModeR)
            , m_minSamplingMode(minSamplingMode)
            , m_magSamplingMode(magSamplingMode)
            , m_anisotropyLevel(anisotropyLevel)
        {
        }

        EWrapMethod     m_addressModeU;
        EWrapMethod     m_addressModeV;
        EWrapMethod     m_addressModeR;
        ESamplingMethod m_minSamplingMode;
        ESamplingMethod m_magSamplingMode;
        UInt32          m_anisotropyLevel;
    };
}

#endif
