//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AllocationHelper.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/SceneGraph/SceneAPI/DataFieldInfo.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/MipMapSize.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"

namespace ramses::internal
{
    const DataFieldInfoVector AllocationHelper::DATA_FIELD_INFOS{
        DataFieldInfo{EDataType::Vector2F, 1}};
    const TextureSampler AllocationHelper::TEXTURE_SAMPLER{{}, TEXTURE_HASH};
    const RenderBuffer AllocationHelper::RENDER_BUFFER{};
    const MipMapDimensions AllocationHelper::MIP_MAP_DIMENSIONS{};

    const SceneSizeInformation AllocationHelper::SCENE_SIZE_INFO{
        42u, //nodes,
        42u, //cameras,
        42u, //transforms,
        42u, //renderables,
        42u, //states,
        42u, //datalayouts,
        42u, //datainstances,
        42u, //uniformBuffers,
        42u, //renderGroups,
        42u, //renderPasses,
        42u, //blitPasses,
        42u, //renderTargets,
        42u, //renderBuffers,
        42u, //textureSamplers,
        42u, //dataSlots,
        42u, //dataBuffers,
        42u, //textureBuffers,
        42u, //pickableObjects,
        42u, //sceneReferences
        };

    template <> TypedMemoryHandle<RenderableHandleTag> AllocationHelper::allocate<RenderableHandleTag>(IScene& scene, TypedMemoryHandle<RenderableHandleTag> handle)
    {
        return scene.allocateRenderable(NODE_HANDLE, handle);
    }

    template <> TypedMemoryHandle<StateHandleTag> AllocationHelper::allocate<StateHandleTag>(IScene& scene, TypedMemoryHandle<StateHandleTag> handle)
    {
        return scene.allocateRenderState(handle);
    }

    template <> TypedMemoryHandle<CameraHandleTag> AllocationHelper::allocate<CameraHandleTag>(IScene& scene, TypedMemoryHandle<CameraHandleTag> handle)
    {
        return scene.allocateCamera(CAMERA_TYPE, NODE_HANDLE, DATA_INSTANCE_HANDLE, handle);
    }

    template <> TypedMemoryHandle<NodeHandleTag> AllocationHelper::allocate<NodeHandleTag>(IScene& scene, TypedMemoryHandle<NodeHandleTag> handle)
    {
        return scene.allocateNode(CHILDREN_COUNT, handle);
    }

    template <> TypedMemoryHandle<TransformHandleTag> AllocationHelper::allocate<TransformHandleTag>(IScene& scene, TypedMemoryHandle<TransformHandleTag> handle)
    {
        return scene.allocateTransform(NODE_HANDLE, handle);
    }

    template <> TypedMemoryHandle<DataLayoutHandleTag> AllocationHelper::allocate<DataLayoutHandleTag>(IScene& scene, TypedMemoryHandle<DataLayoutHandleTag> handle)
    {
        return scene.allocateDataLayout(DATA_FIELD_INFOS, EFFECT_HASH, handle);
    }

    template <> TypedMemoryHandle<DataInstanceHandleTag> AllocationHelper::allocate<DataInstanceHandleTag>(IScene& scene, TypedMemoryHandle<DataInstanceHandleTag> handle)
    {
        return scene.allocateDataInstance(DATA_LAYOUT_HANDLE, handle);
    }

    template <> TypedMemoryHandle<UniformBufferHandleTag> AllocationHelper::allocate<UniformBufferHandleTag>(IScene& scene, TypedMemoryHandle<UniformBufferHandleTag> handle)
    {
        return scene.allocateUniformBuffer(UNIFORM_BUFFER_SIZE, handle);
    }

    template <> TypedMemoryHandle<TextureSamplerHandleTag> AllocationHelper::allocate<TextureSamplerHandleTag>(IScene& scene, TypedMemoryHandle<TextureSamplerHandleTag> handle)
    {
        return scene.allocateTextureSampler(TEXTURE_SAMPLER, handle);
    }

    template <> TypedMemoryHandle<RenderGroupHandleTag> AllocationHelper::allocate<RenderGroupHandleTag>(IScene& scene, TypedMemoryHandle<RenderGroupHandleTag> handle)
    {
        return scene.allocateRenderGroup(RENDERABLE_COUNT, NESTED_GROUP_COUNT, handle);
    }

    template <> TypedMemoryHandle<RenderPassHandleTag> AllocationHelper::allocate<RenderPassHandleTag>(IScene& scene, TypedMemoryHandle<RenderPassHandleTag> handle)
    {
        return scene.allocateRenderPass(RENDERABLE_COUNT, handle);
    }

    template <> TypedMemoryHandle<BlitPassHandleTag> AllocationHelper::allocate<BlitPassHandleTag>(IScene& scene, TypedMemoryHandle<BlitPassHandleTag> handle)
    {
        return scene.allocateBlitPass(SOURCE_RENDER_BUFFER_HANDLE, DESTINATION_RENDER_BUFFER_HANDLE, handle);
    }

    template <> TypedMemoryHandle<PickableObjectTag> AllocationHelper::allocate<PickableObjectTag>(IScene& scene, TypedMemoryHandle<PickableObjectTag> handle)
    {
        return scene.allocatePickableObject(GEOMETRY_HANDLE, NODE_HANDLE, PICKABLE_OBJECT_ID, handle);
    }

    template <> TypedMemoryHandle<RenderTargetHandleTag> AllocationHelper::allocate<RenderTargetHandleTag>(IScene& scene, TypedMemoryHandle<RenderTargetHandleTag> handle)
    {
        return scene.allocateRenderTarget(handle);
    }

    template <> TypedMemoryHandle<RenderBufferHandleTag> AllocationHelper::allocate<RenderBufferHandleTag>(IScene& scene, TypedMemoryHandle<RenderBufferHandleTag> handle)
    {
        return scene.allocateRenderBuffer(RENDER_BUFFER, handle);
    }

    template <> TypedMemoryHandle<DataBufferHandleTag> AllocationHelper::allocate<DataBufferHandleTag>(IScene& scene, TypedMemoryHandle<DataBufferHandleTag> handle)
    {
        return scene.allocateDataBuffer(DATA_BUFFER_TYPE, DATA_TYPE,MAX_SIZE, handle);
    }

    template <> TypedMemoryHandle<TextureBufferHandleTag> AllocationHelper::allocate<TextureBufferHandleTag>(IScene& scene, TypedMemoryHandle<TextureBufferHandleTag> handle)
    {
        return scene.allocateTextureBuffer(TEXTURE_FORMAT, MIP_MAP_DIMENSIONS, handle);
    }

    template <> TypedMemoryHandle<DataSlotHandleTag> AllocationHelper::allocate<DataSlotHandleTag>(IScene& scene, TypedMemoryHandle<DataSlotHandleTag> handle)
    {
        return scene.allocateDataSlot(DATA_SLOT, handle);
    }

    template <> TypedMemoryHandle<SceneReferenceHandleTag> AllocationHelper::allocate<SceneReferenceHandleTag>(IScene& scene, TypedMemoryHandle<SceneReferenceHandleTag> handle)
    {
        return scene.allocateSceneReference(SCENE_ID, handle);
    }

    template <> void AllocationHelper::setupPrerequisits<TransformHandle>(IScene& scene)
    {
        scene.allocateNode(0u, AllocationHelper::NODE_HANDLE);
    }

    template <> void AllocationHelper::setupPrerequisits<DataInstanceHandle>(IScene& scene)
    {
        scene.allocateDataLayout(AllocationHelper::DATA_FIELD_INFOS, AllocationHelper::EFFECT_HASH, AllocationHelper::DATA_LAYOUT_HANDLE);
    }
}
