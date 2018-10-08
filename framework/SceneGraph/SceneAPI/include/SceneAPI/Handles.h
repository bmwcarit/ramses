//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_HANDLES_H
#define RAMSES_SCENEAPI_HANDLES_H

#include "Common/MemoryHandle.h"
#include "Common/TypedMemoryHandle.h"

namespace ramses_internal
{
    // Topology handles
    struct NodeHandleTag {};
    typedef TypedMemoryHandle<NodeHandleTag> NodeHandle;

    struct RenderableHandleTag {};
    typedef TypedMemoryHandle<RenderableHandleTag> RenderableHandle;

    struct TransformHandleTag {};
    typedef TypedMemoryHandle<TransformHandleTag> TransformHandle;

    struct DataLayoutHandleTag {};
    typedef TypedMemoryHandle<DataLayoutHandleTag> DataLayoutHandle;

    struct DataInstanceHandleTag {};
    typedef TypedMemoryHandle<DataInstanceHandleTag> DataInstanceHandle;

    struct CameraHandleTag {};
    typedef TypedMemoryHandle<CameraHandleTag> CameraHandle;

    struct TextureSamplerHandleTag {};
    typedef TypedMemoryHandle<TextureSamplerHandleTag> TextureSamplerHandle;

    struct StateHandleTag {};
    typedef TypedMemoryHandle<StateHandleTag> RenderStateHandle;

    struct RenderGroupHandleTag {};
    typedef TypedMemoryHandle<RenderGroupHandleTag> RenderGroupHandle;

    struct RenderPassHandleTag {};
    typedef TypedMemoryHandle<RenderPassHandleTag> RenderPassHandle;

    struct BlitPassHandleTag {};
    typedef TypedMemoryHandle<BlitPassHandleTag> BlitPassHandle;

    struct RenderTargetHandleTag {};
    typedef TypedMemoryHandle<RenderTargetHandleTag> RenderTargetHandle;

    struct RenderBufferHandleTag {};
    typedef TypedMemoryHandle<RenderBufferHandleTag> RenderBufferHandle;

    struct DataSlotHandleTag {};
    typedef TypedMemoryHandle<DataSlotHandleTag> DataSlotHandle;

    struct StreamTextureTag {};
    typedef TypedMemoryHandle<StreamTextureTag> StreamTextureHandle;

    struct DataBufferHandleTag {};
    typedef TypedMemoryHandle<DataBufferHandleTag> DataBufferHandle;

    struct AnimationSystemHandleTag {};
    typedef TypedMemoryHandle<AnimationSystemHandleTag> AnimationSystemHandle;

    struct TextureBufferHandleTag {};
    typedef TypedMemoryHandle<TextureBufferHandleTag> TextureBufferHandle;

    //Queue handles
    struct QueueHandleTag {};
    typedef TypedMemoryHandle<QueueHandleTag> QueueHandle;

    struct DataFieldHandleTag {};
    typedef TypedMemoryHandle<DataFieldHandleTag> DataFieldHandle;
}

#endif
