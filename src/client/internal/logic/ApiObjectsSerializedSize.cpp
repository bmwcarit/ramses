//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/logic/ApiObjects.h"
#include "internal/logic/ApiObjectsSerializedSize.h"

#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/RenderBufferBinding.h"

#include "internal/logic/flatbuffers/generated/ApiObjectsGen.h"

#include "impl/logic/AnchorPointImpl.h"
#include "impl/logic/AnimationNodeImpl.h"
#include "impl/logic/DataArrayImpl.h"
#include "impl/logic/LuaInterfaceImpl.h"
#include "impl/logic/LuaModuleImpl.h"
#include "impl/logic/LuaScriptImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"
#include "impl/logic/CameraBindingImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/RenderPassBindingImpl.h"
#include "impl/logic/RenderGroupBindingImpl.h"
#include "impl/logic/MeshNodeBindingImpl.h"
#include "impl/logic/SkinBindingImpl.h"
#include "impl/logic/TimerNodeImpl.h"
#include "impl/logic/RenderBufferBindingImpl.h"

namespace ramses::internal
{
    // Helper functions for GetSerializedSize specializations
    template<typename T, typename I>
    size_t calculateSerializedSize(const ApiObjectContainer<T>& container, ELuaSavingMode /*unused*/)
    {
        flatbuffers::FlatBufferBuilder builder{};
        SerializationMap serializationMap{};
        for (const auto& element : container)
        {
            (void)I::Serialize(static_cast<I&>(element->impl()), builder, serializationMap);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    // Since the channels (DataArrays) of an animation are not stored within the Serialize method we need this in this specialization
    template<>
    size_t calculateSerializedSize<AnimationNode, AnimationNodeImpl>(const ApiObjectContainer<AnimationNode>& container, ELuaSavingMode /*unused*/)
    {
        auto insertIds = [](const DataArray* data, std::unordered_set<sceneObjectId_t>& ids)
        {
            if (data)
            {
                ids.insert(data->getSceneObjectId());
            }
        };

        flatbuffers::FlatBufferBuilder builder{};
        for (const auto& element : container)
        {
            std::unordered_set<sceneObjectId_t> ids{};
            for (const auto& channel : element->getChannels())
            {
                insertIds(channel.timeStamps, ids);
                insertIds(channel.keyframes, ids);
                insertIds(channel.tangentsIn, ids);
                insertIds(channel.tangentsOut, ids);
            }
            SerializationMap serializationMap{};
            for (auto& id : ids)
            {
                serializationMap.storeDataArray(id, 0u);
            }
            (void)AnimationNodeImpl::Serialize(element->impl(), builder, serializationMap);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    template<>
    size_t calculateSerializedSize<LuaScript, LuaScriptImpl>(const ApiObjectContainer<LuaScript>& container, ELuaSavingMode luaSavingMode)
    {
        flatbuffers::FlatBufferBuilder builder{};
        SerializationMap serializationMap{};
        for (const auto& element : container)
        {
            (void)LuaScriptImpl::Serialize(element->impl(), builder, serializationMap, luaSavingMode);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    template<>
    size_t calculateSerializedSize<LuaModule, LuaModuleImpl>(const ApiObjectContainer<LuaModule>& container, ELuaSavingMode luaSavingMode)
    {
        flatbuffers::FlatBufferBuilder builder{};
        SerializationMap serializationMap{};
        for (const auto& element : container)
        {
            (void)LuaModuleImpl::Serialize(element->impl(), builder, serializationMap, luaSavingMode);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    size_t ApiObjectsSerializedSize::GetTotalSerializedSize(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        flatbuffers::FlatBufferBuilder builder{};
        (void)ApiObjects::Serialize(apiObjects, builder, luaSavingMode);
        return static_cast<size_t>(builder.GetSize());
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<CameraBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<CameraBinding, CameraBindingImpl>(apiObjects.getApiObjectContainer<CameraBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<NodeBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<NodeBinding, NodeBindingImpl>(apiObjects.getApiObjectContainer<NodeBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RenderPassBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RenderPassBinding, RenderPassBindingImpl>(apiObjects.getApiObjectContainer<RenderPassBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RenderGroupBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RenderGroupBinding, RenderGroupBindingImpl>(apiObjects.getApiObjectContainer<RenderGroupBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<MeshNodeBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<MeshNodeBinding, MeshNodeBindingImpl>(apiObjects.getApiObjectContainer<MeshNodeBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<SkinBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<SkinBinding, SkinBindingImpl>(apiObjects.getApiObjectContainer<SkinBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<AppearanceBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<AppearanceBinding, AppearanceBindingImpl>(apiObjects.getApiObjectContainer<AppearanceBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<TimerNode>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<TimerNode, TimerNodeImpl>(apiObjects.getApiObjectContainer<TimerNode>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<LuaModule>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<LuaModule, LuaModuleImpl>(apiObjects.getApiObjectContainer<LuaModule>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<LuaScript>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<LuaScript, LuaScriptImpl>(apiObjects.getApiObjectContainer<LuaScript>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<LuaInterface>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<LuaInterface, LuaInterfaceImpl>(apiObjects.getApiObjectContainer<LuaInterface>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<DataArray>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<DataArray, DataArrayImpl>(apiObjects.getApiObjectContainer<DataArray>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<AnimationNode>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<AnimationNode, AnimationNodeImpl>(apiObjects.getApiObjectContainer<AnimationNode>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<AnchorPoint>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<AnchorPoint, AnchorPointImpl>(apiObjects.getApiObjectContainer<AnchorPoint>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RenderBufferBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RenderBufferBinding, RenderBufferBindingImpl>(apiObjects.getApiObjectContainer<RenderBufferBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<LogicObject>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return GetTotalSerializedSize(apiObjects, luaSavingMode);
    }
}
