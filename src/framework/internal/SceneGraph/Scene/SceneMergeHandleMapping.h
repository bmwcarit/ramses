//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

#include <unordered_map>

namespace ramses::internal
{
    class SceneMergeHandleMapping
    {
    public:
        SceneMergeHandleMapping() = default;
        ~SceneMergeHandleMapping() = default;

        template<typename T>
        void addMapping(TypedMemoryHandle<T> handle, TypedMemoryHandle<T> newHandle);
        void addMapping(sceneObjectId_t handle, sceneObjectId_t newHandle);

        template<typename T>
        [[nodiscard]] TypedMemoryHandle<T> getMapping(TypedMemoryHandle<T> handle) const;
        [[nodiscard]] sceneObjectId_t getMapping(sceneObjectId_t handle) const;

        template<typename T>
        [[nodiscard]] bool hasMapping(TypedMemoryHandle<T> handle) const;
        [[nodiscard]] bool hasMapping(sceneObjectId_t handle) const;

    private:
        template<typename T>
        using container_ll_t = std::unordered_map<TypedMemoryHandle<T>, TypedMemoryHandle<T>>;

        template<typename T>
        using container_hl_t = std::unordered_map<T, T>;

        template<typename T>
        container_ll_t<T>& getMapping();

        template<typename T>
        const container_ll_t<T>& getMapping() const;

        container_ll_t<NodeHandleTag> m_nodeHandleMapping;
        container_ll_t<TransformHandleTag> m_transformHandleMapping;
        container_ll_t<RenderableHandleTag> m_renderableHandleMapping;
        container_ll_t<DataInstanceHandleTag> m_dataInstanceHandleMapping;
        container_ll_t<TextureSamplerHandleTag> m_textureSamplerHandleMapping;
        container_ll_t<RenderBufferHandleTag> m_renderBufferHandleMapping;
        container_ll_t<DataSlotHandleTag> m_dataSlotHandleMapping;
        container_ll_t<RenderGroupHandleTag> m_renderGroupHandleMapping;
        container_ll_t<StateHandleTag> m_stateHandleMapping;
        container_ll_t<CameraHandleTag> m_cameraHandleMapping;
        container_ll_t<RenderPassHandleTag> m_renderPassHandleMapping;
        container_ll_t<BlitPassHandleTag> m_blitPassHandleMapping;
        container_ll_t<PickableObjectTag> m_pickableObjectHandleMapping;
        container_ll_t<RenderTargetHandleTag> m_renderTargetHandleMapping;
        container_ll_t<DataBufferHandleTag> m_dataBufferHandleMapping;
        container_ll_t<TextureBufferHandleTag> m_textureBufferHandleMapping;
        container_ll_t<SceneReferenceHandleTag> m_sceneReferenceHandleMapping;
        container_ll_t<DataLayoutHandleTag> m_dataLayoutHandleMapping;
        container_ll_t<UniformBufferHandleTag> m_uniformBufferHandleMapping;

        container_hl_t<sceneObjectId_t> m_sceneObjectIdMapping;
    };

    template<typename T>
    inline void SceneMergeHandleMapping::addMapping(TypedMemoryHandle<T> handle, TypedMemoryHandle<T> newHandle)
    {
        assert(handle.isValid());
        assert(newHandle.isValid());
        auto& mapping = getMapping<T>();
        assert(mapping.count(handle) == 0);
        mapping[handle] = newHandle;
    }

    inline void SceneMergeHandleMapping::addMapping(sceneObjectId_t handle, sceneObjectId_t newHandle)
    {
        assert(handle.isValid());
        assert(newHandle.isValid());
        assert(m_sceneObjectIdMapping.count(handle) == 0);
        m_sceneObjectIdMapping[handle] = newHandle;
    }

    template<typename T>
    inline TypedMemoryHandle<T> SceneMergeHandleMapping::getMapping(TypedMemoryHandle<T> handle) const
    {
        assert(handle.isValid());
        const auto& mapping = getMapping<T>();
        auto findIt = mapping.find(handle);
        if (findIt != mapping.end())
        {
            return findIt->second;
        }
        return TypedMemoryHandle<T>::Invalid();
    }

    inline sceneObjectId_t SceneMergeHandleMapping::getMapping(sceneObjectId_t handle) const
    {
        assert(handle.isValid());
        auto findIt = m_sceneObjectIdMapping.find(handle);
        if (findIt != m_sceneObjectIdMapping.end())
        {
            return findIt->second;
        }
        return sceneObjectId_t::Invalid();
    }

    template<typename T>
    inline bool SceneMergeHandleMapping::hasMapping(TypedMemoryHandle<T> handle) const
    {
        if (!handle.isValid())
        {
            return false;
        }

        return getMapping<T>().count(handle) > 0u;
    }

    inline bool SceneMergeHandleMapping::hasMapping(sceneObjectId_t handle) const
    {
        if (!handle.isValid())
        {
            return false;
        }

        return m_sceneObjectIdMapping.count(handle) > 0u;
    }

    template<typename T>
    inline const SceneMergeHandleMapping::container_ll_t<T>& SceneMergeHandleMapping::getMapping() const
    {
        auto* nonConstThis = const_cast<SceneMergeHandleMapping*>(this);
        return nonConstThis->getMapping<T>();
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<NodeHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_nodeHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<TransformHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_transformHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<RenderableHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_renderableHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<DataInstanceHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_dataInstanceHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<RenderBufferHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_renderBufferHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<DataSlotHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_dataSlotHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<RenderGroupHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_renderGroupHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<StateHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_stateHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<CameraHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_cameraHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<RenderPassHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_renderPassHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<BlitPassHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_blitPassHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<PickableObjectTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_pickableObjectHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<RenderTargetHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_renderTargetHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<DataBufferHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_dataBufferHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<TextureBufferHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_textureBufferHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<SceneReferenceHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_sceneReferenceHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<TextureSamplerHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_textureSamplerHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<DataLayoutHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_dataLayoutHandleMapping;
    }

    template<>
    inline SceneMergeHandleMapping::container_ll_t<UniformBufferHandleTag>& SceneMergeHandleMapping::getMapping()
    {
        return m_uniformBufferHandleMapping;
    }
}
