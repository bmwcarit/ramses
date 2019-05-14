//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_SCENETYPES_H
#define RAMSES_SCENEAPI_SCENETYPES_H

#include "SceneAPI/Handles.h"
#include "SceneAPI/EFixedSemantics.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/EDataType.h"

#include "Collections/Vector.h"

namespace ramses_internal
{
    struct RenderGroupOrderEntry
    {
        RenderGroupHandle renderGroup;
        Int32             order;

        Bool operator<(const RenderGroupOrderEntry& other) const
        {
            return order < other.order;
        }
    };

    using RenderPassVector           =  std::vector<RenderPassHandle>;
    using RenderableVector           =  std::vector<RenderableHandle>;
    using RenderPassVector           =  std::vector<RenderPassHandle>;
    using NodeHandleVector           =  std::vector<NodeHandle>;
    using TransformHandleVector      =  std::vector<TransformHandle>;
    using RenderBufferHandleVector   =  std::vector<RenderBufferHandle>;
    using StreamTextureHandleVector  =  std::vector<StreamTextureHandle>;
    using RenderGroupOrderVector     =  std::vector<RenderGroupOrderEntry>;
    using BlitPassHandleVector       =  std::vector<BlitPassHandle>;
    using DataBufferHandleVector     =  std::vector<DataBufferHandle>;
    using TextureBufferHandleVector  =  std::vector<TextureBufferHandle>;
    using TextureSamplerHandleVector =  std::vector<TextureSamplerHandle>;
}

#endif
