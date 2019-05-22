//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESAMPLERGPURESOURCE_H
#define RAMSES_TEXTURESAMPLERGPURESOURCE_H

#include "Platform_Base/GpuResource.h"
#include "SceneAPI/TextureEnums.h"

namespace ramses_internal
{
    class TextureSamplerGPUResource : public GPUResource
    {
    public:
        TextureSamplerGPUResource(EWrapMethod wrapMethodU, EWrapMethod wrapMethodV, EWrapMethod wrapMethodR, ESamplingMethod minSamplingMethod, ESamplingMethod magSamplingMethod, UInt32 anisotropyLvl, UInt32 gpuAddress, UInt32 dataSizeInBytes)
            : GPUResource(gpuAddress, dataSizeInBytes)
            , wrapU(wrapMethodU)
            , wrapV(wrapMethodV)
            , wrapR(wrapMethodR)
            , minSampling(minSamplingMethod)
            , magSampling(magSamplingMethod)
            , anisotropyLevel(anisotropyLvl)
        {
        }

        const EWrapMethod wrapU;
        const EWrapMethod wrapV;
        const EWrapMethod wrapR;
        const ESamplingMethod minSampling;
        const ESamplingMethod magSampling;
        const UInt32 anisotropyLevel;
    };
}

#endif
