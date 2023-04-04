//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/ApiObjects.h"

#include "ramses-logic/EFeatureLevel.h"

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

namespace rlogic::internal
{
    // Helper functions for getSerializedSize specializations
    template<typename T, typename I>
    size_t calculateSerializedSize(const ApiObjectContainer<T>& container, EFeatureLevel featureLevel)
    {
        flatbuffers::FlatBufferBuilder builder{};
        SerializationMap serializationMap{};
        for (const auto& element : container)
        {
            (void)I::Serialize(static_cast<I&>(element->m_impl), builder, serializationMap, featureLevel);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    // Since the channels (DataArrays) of an animation are not stored within the Serialize method we need this in this specialization
    template<>
    size_t calculateSerializedSize<AnimationNode, AnimationNodeImpl>(const ApiObjectContainer<AnimationNode>& container, EFeatureLevel /*featureLevel*/)
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
    size_t calculateSerializedSize<LuaScript, LuaScriptImpl>(const ApiObjectContainer<LuaScript>& container, EFeatureLevel /*featureLevel*/)
    {
        flatbuffers::FlatBufferBuilder builder{};
        SerializationMap serializationMap{};
        for (const auto& element : container)
        {
            (void)LuaScriptImpl::Serialize(element->m_script, builder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    template<>
    size_t calculateSerializedSize<LuaModule, LuaModuleImpl>(const ApiObjectContainer<LuaModule>& container, EFeatureLevel /*featureLevel*/)
    {
        flatbuffers::FlatBufferBuilder builder{};
        SerializationMap serializationMap{};
        for (const auto& element : container)
        {
            (void)LuaModuleImpl::Serialize(element->m_impl, builder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        }
        return static_cast<size_t>(builder.GetSize());
    }

    size_t ApiObjects::getTotalSerializedSize() const
    {
        flatbuffers::FlatBufferBuilder builder{};
        (void)Serialize(*this, builder, ELuaSavingMode::ByteCodeOnly); // TODO vaclav allow specification of lua saving mode for file size estimation
        return static_cast<size_t>(builder.GetSize());
    }

    template<>
    size_t ApiObjects::getSerializedSize<RamsesCameraBinding>() const
    {
        return calculateSerializedSize<RamsesCameraBinding, RamsesCameraBindingImpl>(getApiObjectContainer<RamsesCameraBinding>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<RamsesNodeBinding>() const
    {
        return calculateSerializedSize<RamsesNodeBinding, RamsesNodeBindingImpl>(getApiObjectContainer<RamsesNodeBinding>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<RamsesRenderPassBinding>() const
    {
        return calculateSerializedSize<RamsesRenderPassBinding, RamsesRenderPassBindingImpl>(getApiObjectContainer<RamsesRenderPassBinding>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<RamsesRenderGroupBinding>() const
    {
        return calculateSerializedSize<RamsesRenderGroupBinding, RamsesRenderGroupBindingImpl>(getApiObjectContainer<RamsesRenderGroupBinding>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<RamsesMeshNodeBinding>() const
    {
        return calculateSerializedSize<RamsesMeshNodeBinding, RamsesMeshNodeBindingImpl>(getApiObjectContainer<RamsesMeshNodeBinding>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<SkinBinding>() const
    {
        return calculateSerializedSize<SkinBinding, SkinBindingImpl>(getApiObjectContainer<SkinBinding>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<RamsesAppearanceBinding>() const
    {
        return calculateSerializedSize<RamsesAppearanceBinding, RamsesAppearanceBindingImpl>(getApiObjectContainer<RamsesAppearanceBinding>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<TimerNode>() const
    {
        return calculateSerializedSize<TimerNode, TimerNodeImpl>(getApiObjectContainer<TimerNode>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<LuaModule>() const
    {
        return calculateSerializedSize<LuaModule, LuaModuleImpl>(getApiObjectContainer<LuaModule>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<LuaScript>() const
    {
        return calculateSerializedSize<LuaScript, LuaScriptImpl>(getApiObjectContainer<LuaScript>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<LuaInterface>() const
    {
        return calculateSerializedSize<LuaInterface, LuaInterfaceImpl>(getApiObjectContainer<LuaInterface>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<DataArray>() const
    {
        return calculateSerializedSize<DataArray, DataArrayImpl>(getApiObjectContainer<DataArray>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<AnimationNode>() const
    {
        return calculateSerializedSize<AnimationNode, AnimationNodeImpl>(getApiObjectContainer<AnimationNode>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<AnchorPoint>() const
    {
        return calculateSerializedSize<AnchorPoint, AnchorPointImpl>(getApiObjectContainer<AnchorPoint>(), m_featureLevel);
    }

    template<>
    size_t ApiObjects::getSerializedSize<LogicObject>() const
    {
        return getTotalSerializedSize();
    }
}
