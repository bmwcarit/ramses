//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once


#include "internal/Core/Common/TypedMemoryHandle.h"
#include "internal/SceneGraph/SceneAPI/DataFieldInfo.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/ECameraProjectionType.h"
#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"
#include "internal/SceneGraph/SceneAPI/EDataSlotType.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/GeometryDataBuffer.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <sys/stat.h>
#include "ActionTestScene.h"
#include "ramses/framework/TextureEnums.h"


namespace ramses::internal
{
    class AllocationHelper
    {
    public:
        static constexpr NodeHandle NODE_HANDLE{12u};
        static constexpr ECameraProjectionType CAMERA_TYPE{ECameraProjectionType::Perspective};
        static constexpr DataInstanceHandle DATA_INSTANCE_HANDLE{321u};
        static constexpr uint32_t CHILDREN_COUNT{0u};
        static constexpr ResourceContentHash EFFECT_HASH{876u, 543u};
        static constexpr ResourceContentHash TEXTURE_HASH{9876u, 543u};
        static constexpr DataLayoutHandle DATA_LAYOUT_HANDLE{3u};
        static const DataFieldInfoVector DATA_FIELD_INFOS;
        static constexpr EDataBufferType DATA_BUFFER_TYPE{EDataBufferType::IndexBuffer};
        static constexpr EDataType DATA_TYPE{EDataType::Vector3F};
        static constexpr uint32_t MAX_SIZE{12u};
        static const TextureSampler TEXTURE_SAMPLER;
        static constexpr EPixelStorageFormat TEXTURE_FORMAT{EPixelStorageFormat::RGBA8};
        static const MipMapDimensions MIP_MAP_DIMENSIONS;
        static constexpr uint32_t RENDERABLE_COUNT{7u};
        static constexpr uint32_t NESTED_GROUP_COUNT{0u};
        static constexpr RenderBufferHandle SOURCE_RENDER_BUFFER_HANDLE{234u};
        static constexpr RenderBufferHandle DESTINATION_RENDER_BUFFER_HANDLE{345u};
        static constexpr DataBufferHandle GEOMETRY_HANDLE{96u};
        static constexpr PickableObjectId PICKABLE_OBJECT_ID{66};
        static const RenderBuffer RENDER_BUFFER;
        static constexpr SceneId SCENE_ID{12345};
        static constexpr DataSlot DATA_SLOT{EDataSlotType::DataConsumer, {}, {}, {}, {}, {}};
        static constexpr uint32_t UNIFORM_BUFFER_SIZE{12u};

        static const SceneSizeInformation SCENE_SIZE_INFO;

        template <typename T>
        static TypedMemoryHandle<T> allocate([[maybe_unused]] IScene& scene, [[maybe_unused]] TypedMemoryHandle<T> handle)
        {
            assert(false);
            return TypedMemoryHandle<T>::Invalid();
        }

        template <typename T>
        static void setupPrerequisits([[maybe_unused]] IScene& scene)
        {
            // nothing to do for most types
        }
    };

    template <> TypedMemoryHandle<RenderableHandleTag> AllocationHelper::allocate<RenderableHandleTag>(IScene& scene, TypedMemoryHandle<RenderableHandleTag> handle);
    template <> TypedMemoryHandle<StateHandleTag> AllocationHelper::allocate<StateHandleTag>(IScene& scene, TypedMemoryHandle<StateHandleTag> handle);
    template <> TypedMemoryHandle<CameraHandleTag> AllocationHelper::allocate<CameraHandleTag>(IScene& scene, TypedMemoryHandle<CameraHandleTag> handle);
    template <> TypedMemoryHandle<NodeHandleTag> AllocationHelper::allocate<NodeHandleTag>(IScene& scene, TypedMemoryHandle<NodeHandleTag> handle);
    template <> TypedMemoryHandle<TransformHandleTag> AllocationHelper::allocate<TransformHandleTag>(IScene& scene, TypedMemoryHandle<TransformHandleTag> handle);
    template <> TypedMemoryHandle<DataLayoutHandleTag> AllocationHelper::allocate<DataLayoutHandleTag>(IScene& scene, TypedMemoryHandle<DataLayoutHandleTag> handle);
    template <> TypedMemoryHandle<DataInstanceHandleTag> AllocationHelper::allocate<DataInstanceHandleTag>(IScene& scene, TypedMemoryHandle<DataInstanceHandleTag> handle);
    template <> TypedMemoryHandle<UniformBufferHandleTag> AllocationHelper::allocate<UniformBufferHandleTag>(IScene& scene, TypedMemoryHandle<UniformBufferHandleTag> handle);
    template <> TypedMemoryHandle<TextureSamplerHandleTag> AllocationHelper::allocate<TextureSamplerHandleTag>(IScene& scene, TypedMemoryHandle<TextureSamplerHandleTag> handle);
    template <> TypedMemoryHandle<RenderGroupHandleTag> AllocationHelper::allocate<RenderGroupHandleTag>(IScene& scene, TypedMemoryHandle<RenderGroupHandleTag> handle);
    template <> TypedMemoryHandle<RenderPassHandleTag> AllocationHelper::allocate<RenderPassHandleTag>(IScene& scene, TypedMemoryHandle<RenderPassHandleTag> handle);
    template <> TypedMemoryHandle<BlitPassHandleTag> AllocationHelper::allocate<BlitPassHandleTag>(IScene& scene, TypedMemoryHandle<BlitPassHandleTag> handle);
    template <> TypedMemoryHandle<PickableObjectTag> AllocationHelper::allocate<PickableObjectTag>(IScene& scene, TypedMemoryHandle<PickableObjectTag> handle);
    template <> TypedMemoryHandle<RenderTargetHandleTag> AllocationHelper::allocate<RenderTargetHandleTag>(IScene& scene, TypedMemoryHandle<RenderTargetHandleTag> handle);
    template <> TypedMemoryHandle<RenderBufferHandleTag> AllocationHelper::allocate<RenderBufferHandleTag>(IScene& scene, TypedMemoryHandle<RenderBufferHandleTag> handle);
    template <> TypedMemoryHandle<DataBufferHandleTag> AllocationHelper::allocate<DataBufferHandleTag>(IScene& scene, TypedMemoryHandle<DataBufferHandleTag> handle);
    template <> TypedMemoryHandle<TextureBufferHandleTag> AllocationHelper::allocate<TextureBufferHandleTag>(IScene& scene, TypedMemoryHandle<TextureBufferHandleTag> handle);
    template <> TypedMemoryHandle<DataSlotHandleTag> AllocationHelper::allocate<DataSlotHandleTag>(IScene& scene, TypedMemoryHandle<DataSlotHandleTag> handle);
    template <> TypedMemoryHandle<SceneReferenceHandleTag> AllocationHelper::allocate<SceneReferenceHandleTag>(IScene& scene, TypedMemoryHandle<SceneReferenceHandleTag> handle);

    template <> void AllocationHelper::setupPrerequisits<TransformHandle>(IScene& scene);
    template <> void AllocationHelper::setupPrerequisits<DataInstanceHandle>(IScene& scene);
}
