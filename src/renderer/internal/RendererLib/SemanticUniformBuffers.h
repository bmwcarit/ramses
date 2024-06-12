//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/DataTypes.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/RendererLib/SemanticUniformBufferHandle.h"
#include "internal/RendererLib/Types.h"
#include <vector>
#include <unordered_map>

namespace ramses::internal
{
    class IRendererResourceManager;
    class SemanticUniformBufferScene;

    template <typename UBOData>
    class SemanticUniformBuffers
    {
    public:
        explicit SemanticUniformBuffers(SceneId sceneId);

        void markAsUsed(SemanticUniformBufferHandle uboHandle);
        void markDirty(SemanticUniformBufferHandle uboHandle);

        DeviceResourceHandle getDeviceHandle(SemanticUniformBufferHandle uboHandle) const;

        void update(const SemanticUniformBufferScene& scene);
        void upload(IRendererResourceManager& resourceManager);

        // number of decays to reach before deallocating a UBO from memory (in practice this is number of frames with some update where UBO was not used)
        static constexpr uint32_t DecayCountToDeallocate = 500;

    private:
        void uploadNewUBOs(IRendererResourceManager& resourceManager);
        void decayAndUnloadUnusedUBOs(IRendererResourceManager& resourceManager);
        void uploadUpdatedUBOs(IRendererResourceManager& resourceManager);

        SceneId m_sceneId;

        std::unordered_map<SemanticUniformBufferHandle, uint32_t> m_decays; // in practice this is number of frames with some update since last time UBO was used (not necessarily updated, just used)
        std::unordered_map<SemanticUniformBufferHandle, DeviceResourceHandle> m_deviceHandles;

        std::unordered_map<SemanticUniformBufferHandle, UBOData> m_uboData;
        std::vector<SemanticUniformBufferHandle> m_dirtyUBOs;
        std::vector<SemanticUniformBufferHandle> m_updatedUBOs;
        std::vector<SemanticUniformBufferHandle> m_newUBOs;

        static constexpr size_t UBOSize = sizeof(UBOData);
    };

    namespace SemanticUniformBuffersInstances
    {
        using Mat44Bytes = std::array<std::byte, 16u * sizeof(float)>;
        using Vec3Bytes = std::array<std::byte, 3u * sizeof(float)>;

        // Model instance specific data/logic
        using ModelData = Mat44Bytes;
        void CalculateUBOData(SemanticUniformBufferHandle uboHandle, const SemanticUniformBufferScene& scene, ModelData& uboData);
        const std::byte* GetUBODataPtr(const ModelData& uboData);

        // Camera instance specific data/logic
        struct CameraData
        {
            Mat44Bytes pMat;
            Mat44Bytes vMat;
            Vec3Bytes position;
        };
        void CalculateUBOData(SemanticUniformBufferHandle uboHandle, const SemanticUniformBufferScene& scene, CameraData& uboData);
        const std::byte* GetUBODataPtr(const CameraData& uboData);

        // ModelCamera instance specific data/logic
        struct ModelCameraData
        {
            Mat44Bytes mvpMat;
            Mat44Bytes mvMat;
            Mat44Bytes normalMat;
        };
        void CalculateUBOData(SemanticUniformBufferHandle uboHandle, const SemanticUniformBufferScene& scene, ModelCameraData& uboData);
        const std::byte* GetUBODataPtr(const ModelCameraData& uboData);

        glm::mat4 CalculateProjectionMatrix(CameraHandle camera, const SemanticUniformBufferScene& scene);

        template <typename UBOData>
        SemanticUniformBufferHandle::Type GetMatchingSemanticType()
        {
            if constexpr (std::is_same_v<UBOData, ModelData>)
                return SemanticUniformBufferHandle::Type::Model;
            if constexpr (std::is_same_v<UBOData, ModelCameraData>)
                return SemanticUniformBufferHandle::Type::ModelCamera;
            if constexpr (std::is_same_v<UBOData, CameraData>)
                return SemanticUniformBufferHandle::Type::Camera;
        }
    };

    using SemanticUniformBuffers_Model = SemanticUniformBuffers<SemanticUniformBuffersInstances::ModelData>;
    using SemanticUniformBuffers_Camera = SemanticUniformBuffers<SemanticUniformBuffersInstances::CameraData>;
    using SemanticUniformBuffers_ModelCamera = SemanticUniformBuffers<SemanticUniformBuffersInstances::ModelCameraData>;
}
