//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/SemanticUniformBufferHandle.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"

namespace ramses::internal
{
    enum ESceneResourceType
    {
        ESceneResourceType_RenderBuffer_WriteOnly = 0,
        ESceneResourceType_RenderBuffer_ReadWrite,
        ESceneResourceType_DataBuffer,
        ESceneResourceType_TextureBuffer,
        ESceneResourceType_NUMBER_OF_ELEMENTS
    };

    class RendererSceneResourceRegistry
    {
    public:
        RendererSceneResourceRegistry();
        ~RendererSceneResourceRegistry();

        template<typename HandleType, typename... ParamsT>
        void add(HandleType handle, const ParamsT&... params)
        {
            auto& infoStorage = getInternalStorage<HandleType>();

            assert(!infoStorage.contains(handle));
            infoStorage.put(handle, { params... });
        }

        template<typename HandleType>
        void remove(HandleType handle)
        {
            auto& infoStorage = getInternalStorage<HandleType>();

            assert(infoStorage.contains(handle));
            infoStorage.remove(handle);
        }

        template<typename HandleType>
        [[nodiscard]] const auto& get(HandleType handle) const
        {
            const auto& internalStorage = getConstInternalStorage<HandleType>();
            assert(internalStorage.contains(handle));
            return *internalStorage.get(handle);
        }

        template<typename HandleType>
        [[nodiscard]] std::vector<HandleType> getAll() const
        {
            const auto& internalStorage = getConstInternalStorage<HandleType>();
            std::vector<HandleType> resultStorage;
            resultStorage.reserve(internalStorage.size());
            for (const auto& e : internalStorage)
            {
                resultStorage.push_back(e.key);
            }

            return resultStorage;
        }

        void                    getBlitPassDeviceHandles    (BlitPassHandle handle, DeviceResourceHandle& srcRenderTargetDeviceHandle, DeviceResourceHandle& dstRenderTargetDeviceHandle) const;
        [[nodiscard]] uint32_t  getSceneResourceMemoryUsage (ESceneResourceType resourceType) const;

    private:
        struct TextureBufferEntry
        {
            DeviceResourceHandle deviceHandle;
            EPixelStorageFormat format = EPixelStorageFormat::Invalid;
            uint32_t size = 0u;
        };

        struct RenderBufferEntry
        {
            DeviceResourceHandle deviceHandle;
            uint32_t size = 0u;
            RenderBuffer renderBufferProperties;
        };

        struct BlitPassEntry
        {
            DeviceResourceHandle sourceRenderTargetDeviceHandle;
            DeviceResourceHandle destinationRenderTargetDeviceHandle;
        };

        struct DataBufferEntry
        {
            DeviceResourceHandle deviceHandle;
            EDataBufferType dataBufferType = EDataBufferType::Invalid;
            uint32_t size = 0u;
        };

        using RenderBufferMap        = HashMap<RenderBufferHandle,   RenderBufferEntry>;
        using RenderTargetMap        = HashMap<RenderTargetHandle,   DeviceResourceHandle>;
        using BlitPassMap            = HashMap<BlitPassHandle,       BlitPassEntry>;
        using DataBufferMap          = HashMap<DataBufferHandle,     DataBufferEntry>;
        using TextureBufferMap       = HashMap<TextureBufferHandle,  TextureBufferEntry>;
        using VertexArrayMap         = HashMap<RenderableHandle,     DeviceResourceHandle>;
        using UniformBufferMap       = HashMap<UniformBufferHandle,  DeviceResourceHandle>;
        using SemanticUniformBufferMap = HashMap<SemanticUniformBufferHandle, DeviceResourceHandle>;

        template<typename T, typename = std::enable_if_t<std::is_same_v<T, RenderBufferHandle>>>
        RenderBufferMap& getInternalStorage()
        {
            return m_renderBuffers;
        }
        template<typename T, typename = std::enable_if_t<std::is_same_v<T, RenderTargetHandle>>>
        RenderTargetMap& getInternalStorage()
        {
            return m_renderTargets;
        }
        template<typename T, typename = std::enable_if_t<std::is_same_v<T, BlitPassHandle>>>
        BlitPassMap& getInternalStorage()
        {
            return m_blitPasses;
        }
        template<typename T, typename = std::enable_if_t<std::is_same_v<T, DataBufferHandle>>>
        DataBufferMap& getInternalStorage()
        {
            return m_dataBuffers;
        }
        template<typename T, typename = std::enable_if_t<std::is_same_v<T, TextureBufferHandle>>>
        TextureBufferMap& getInternalStorage()
        {
            return m_textureBuffers;
        }
        template<typename T, typename = std::enable_if_t<std::is_same_v<T, RenderableHandle>>>
        VertexArrayMap& getInternalStorage()
        {
            return m_vertexArrays;
        }
        template<typename T, typename = std::enable_if_t<std::is_same_v<T, UniformBufferHandle>>>
        UniformBufferMap& getInternalStorage()
        {
            return m_uniformBuffers;
        }
        template<typename T, typename = std::enable_if_t<std::is_same_v<T, SemanticUniformBufferHandle>>>
        SemanticUniformBufferMap& getInternalStorage()
        {
            return m_semanticUniformBuffers;
        }

        template<typename T>
        [[nodiscard]] const auto& getConstInternalStorage() const// -> const decltype(getInternalStorage<T>())&
        {
            return const_cast<RendererSceneResourceRegistry&>(*this).getInternalStorage<T>();
        }

        RenderBufferMap        m_renderBuffers;
        RenderTargetMap        m_renderTargets;
        BlitPassMap            m_blitPasses;
        DataBufferMap          m_dataBuffers;
        TextureBufferMap       m_textureBuffers;
        VertexArrayMap         m_vertexArrays;
        UniformBufferMap       m_uniformBuffers;
        SemanticUniformBufferMap m_semanticUniformBuffers;
    };
}
