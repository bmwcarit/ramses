//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_HANDLES_H
#define RAMSES_SCENEAPI_HANDLES_H

#include "Common/TypedMemoryHandle.h"

namespace ramses_internal
{
    // Topology handles
    struct NodeHandleTag {};
    using NodeHandle = TypedMemoryHandle<NodeHandleTag>;

    struct RenderableHandleTag {};
    using RenderableHandle = TypedMemoryHandle<RenderableHandleTag>;

    struct TransformHandleTag {};
    using TransformHandle = TypedMemoryHandle<TransformHandleTag>;

    struct DataLayoutHandleTag {};
    using DataLayoutHandle = TypedMemoryHandle<DataLayoutHandleTag>;

    struct DataInstanceHandleTag {};
    using DataInstanceHandle = TypedMemoryHandle<DataInstanceHandleTag>;

    struct CameraHandleTag {};
    using CameraHandle = TypedMemoryHandle<CameraHandleTag>;

    struct TextureSamplerHandleTag {};
    using TextureSamplerHandle = TypedMemoryHandle<TextureSamplerHandleTag>;

    struct StateHandleTag {};
    using RenderStateHandle = TypedMemoryHandle<StateHandleTag>;

    struct RenderGroupHandleTag {};
    using RenderGroupHandle = TypedMemoryHandle<RenderGroupHandleTag>;

    struct RenderPassHandleTag {};
    using RenderPassHandle = TypedMemoryHandle<RenderPassHandleTag>;

    struct BlitPassHandleTag {};
    using BlitPassHandle = TypedMemoryHandle<BlitPassHandleTag>;

    struct PickableObjectTag {};
    using PickableObjectHandle = TypedMemoryHandle<PickableObjectTag>;

    struct RenderTargetHandleTag {};
    using RenderTargetHandle = TypedMemoryHandle<RenderTargetHandleTag>;

    struct RenderBufferHandleTag {};
    using RenderBufferHandle = TypedMemoryHandle<RenderBufferHandleTag>;

    struct DataSlotHandleTag {};
    using DataSlotHandle = TypedMemoryHandle<DataSlotHandleTag>;

    struct StreamTextureTag {};
    using StreamTextureHandle = TypedMemoryHandle<StreamTextureTag>;

    struct DataBufferHandleTag {};
    using DataBufferHandle = TypedMemoryHandle<DataBufferHandleTag>;

    struct TextureBufferHandleTag {};
    using TextureBufferHandle = TypedMemoryHandle<TextureBufferHandleTag>;

    struct DataFieldHandleTag {};
    using DataFieldHandle = TypedMemoryHandle<DataFieldHandleTag>;

    struct SceneReferenceHandleTag {};
    using SceneReferenceHandle = TypedMemoryHandle<SceneReferenceHandleTag>;
}

#endif
