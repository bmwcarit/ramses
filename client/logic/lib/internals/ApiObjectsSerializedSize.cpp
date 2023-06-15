//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/ApiObjects.h"
#include "internals/ApiObjectsSerializedSize.h"

#include "ramses-logic/AnchorPoint.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/SkinBinding.h"
#include "ramses-logic/TimerNode.h"

#include "generated/ApiObjectsGen.h"

#include "impl/AnchorPointImpl.h"
#include "impl/AnimationNodeImpl.h"
#include "impl/DataArrayImpl.h"
#include "impl/LuaInterfaceImpl.h"
#include "impl/LuaModuleImpl.h"
#include "impl/LuaScriptImpl.h"
#include "impl/RamsesAppearanceBindingImpl.h"
#include "impl/RamsesCameraBindingImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/RamsesRenderPassBindingImpl.h"
#include "impl/RamsesRenderGroupBindingImpl.h"
#include "impl/RamsesMeshNodeBindingImpl.h"
#include "impl/SkinBindingImpl.h"
#include "impl/TimerNodeImpl.h"

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
            (void)I::Serialize(static_cast<I&>(element->m_impl), builder, serializationMap);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    // Since the channels (DataArrays) of an animation are not stored within the Serialize method we need this in this specialization
    template<>
    size_t calculateSerializedSize<AnimationNode, AnimationNodeImpl>(const ApiObjectContainer<AnimationNode>& container, ELuaSavingMode /*unused*/)
    {
        auto insertIds = [](const DataArray* data, std::unordered_set<uint64_t>& ids)
        {
            if (data)
            {
                ids.insert(data->getId());
            }
        };

        flatbuffers::FlatBufferBuilder builder{};
        for (const auto& element : container)
        {
            std::unordered_set<uint64_t> ids{};
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
            (void)AnimationNodeImpl::Serialize(element->m_animationNodeImpl, builder, serializationMap);
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
            (void)LuaScriptImpl::Serialize(element->m_script, builder, serializationMap, luaSavingMode);
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
            (void)LuaModuleImpl::Serialize(element->m_impl, builder, serializationMap, luaSavingMode);
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
    size_t ApiObjectsSerializedSize::GetSerializedSize<RamsesCameraBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RamsesCameraBinding, RamsesCameraBindingImpl>(apiObjects.getApiObjectContainer<RamsesCameraBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RamsesNodeBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RamsesNodeBinding, RamsesNodeBindingImpl>(apiObjects.getApiObjectContainer<RamsesNodeBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RamsesRenderPassBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RamsesRenderPassBinding, RamsesRenderPassBindingImpl>(apiObjects.getApiObjectContainer<RamsesRenderPassBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RamsesRenderGroupBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RamsesRenderGroupBinding, RamsesRenderGroupBindingImpl>(apiObjects.getApiObjectContainer<RamsesRenderGroupBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RamsesMeshNodeBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RamsesMeshNodeBinding, RamsesMeshNodeBindingImpl>(apiObjects.getApiObjectContainer<RamsesMeshNodeBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<SkinBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<SkinBinding, SkinBindingImpl>(apiObjects.getApiObjectContainer<SkinBinding>(), luaSavingMode);
    }

    template<>
    size_t ApiObjectsSerializedSize::GetSerializedSize<RamsesAppearanceBinding>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return calculateSerializedSize<RamsesAppearanceBinding, RamsesAppearanceBindingImpl>(apiObjects.getApiObjectContainer<RamsesAppearanceBinding>(), luaSavingMode);
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
    size_t ApiObjectsSerializedSize::GetSerializedSize<LogicObject>(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode)
    {
        return GetTotalSerializedSize(apiObjects, luaSavingMode);
    }
}
