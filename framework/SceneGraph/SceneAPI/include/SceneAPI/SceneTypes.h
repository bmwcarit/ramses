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

    using RenderPassVector           =  Vector<RenderPassHandle>;
    using RenderableVector           =  Vector<RenderableHandle>;
    using RenderPassVector           =  Vector<RenderPassHandle>;
    using NodeHandleVector           =  Vector<NodeHandle>;
    using TransformHandleVector      =  Vector<TransformHandle>;
    using RenderBufferHandleVector   =  Vector<RenderBufferHandle>;
    using StreamTextureHandleVector  =  Vector<StreamTextureHandle>;
    using RenderGroupOrderVector     =  Vector<RenderGroupOrderEntry>;
    using BlitPassHandleVector       =  Vector<BlitPassHandle>;
    using DataBufferHandleVector     =  Vector<DataBufferHandle>;
    using TextureBufferHandleVector  =  Vector<TextureBufferHandle>;
    using TextureSamplerHandleVector =  Vector<TextureSamplerHandle>;
}

#endif
