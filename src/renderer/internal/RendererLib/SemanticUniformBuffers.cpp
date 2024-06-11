//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SemanticUniformBuffers.h"
#include "internal/RendererLib/IRendererResourceManager.h"
#include "internal/RendererLib/SemanticUniformBufferScene.h"
#include "internal/Core/Math3d/CameraMatrixHelper.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    template <typename UBOData>
    SemanticUniformBuffers<UBOData>::SemanticUniformBuffers(SceneId sceneId)
        : m_sceneId{ sceneId }
    {
    }

    template <typename UBOData>
    void SemanticUniformBuffers<UBOData>::markAsUsed(SemanticUniformBufferHandle uboHandle)
    {
        auto it = m_decays.find(uboHandle);
        if (it != m_decays.cend())
        {
            it->second = 0u;
        }
        else
        {
            m_decays.emplace(uboHandle, 0u);
            m_dirtyUBOs.push_back(uboHandle); // make sure newly added UBO will be updated
        }
    }

    template <typename UBOData>
    void SemanticUniformBuffers<UBOData>::markDirty(SemanticUniformBufferHandle uboHandle)
    {
        assert(m_decays.count(uboHandle) != 0u);
        m_dirtyUBOs.push_back(uboHandle);
    }

    template <typename UBOData>
    DeviceResourceHandle SemanticUniformBuffers<UBOData>::getDeviceHandle(SemanticUniformBufferHandle uboHandle) const
    {
        const auto it = m_deviceHandles.find(uboHandle);
        assert(it != m_deviceHandles.cend());
        assert(it->second.isValid());
        return it->second;
    }

    template <typename UBOData>
    void SemanticUniformBuffers<UBOData>::update(const SemanticUniformBufferScene& scene)
    {
        // early out
        if (m_dirtyUBOs.empty())
            return;

        // remove any duplicates
        std::sort(m_dirtyUBOs.begin(), m_dirtyUBOs.end());
        m_dirtyUBOs.erase(std::unique(m_dirtyUBOs.begin(), m_dirtyUBOs.end()), m_dirtyUBOs.end());

        // update matrices for all UBOs to be updated
        assert(m_newUBOs.empty());
        for (const auto uboHandle : m_dirtyUBOs)
        {
            assert(m_decays.count(uboHandle) != 0u);
            UBOData uboData;
            SemanticUniformBuffersInstances::CalculateUBOData(uboHandle, scene, uboData);

            auto it = m_uboData.find(uboHandle);
            if (it == m_uboData.cend())
            {
                m_newUBOs.push_back(uboHandle);
                m_uboData.emplace(uboHandle, std::move(uboData));
                assert(m_deviceHandles.count(uboHandle) == 0u);
            }
            else
            {
                it->second = std::move(uboData);
            }
        }
    }

    template <typename UBOData>
    void SemanticUniformBuffers<UBOData>::uploadNewUBOs(IRendererResourceManager& resourceManager)
    {
        for (const auto uboHandle : m_newUBOs)
        {
            assert(uboHandle.getType() == SemanticUniformBuffersInstances::GetMatchingSemanticType<UBOData>());
            m_deviceHandles[uboHandle] = resourceManager.uploadUniformBuffer(uboHandle, UBOSize, m_sceneId);
        }
        m_newUBOs.clear();
    }

    template <typename UBOData>
    void SemanticUniformBuffers<UBOData>::decayAndUnloadUnusedUBOs(IRendererResourceManager& resourceManager)
    {
        auto& toDelete = m_newUBOs; // just reusing container to avoid allocating any memory
        assert(toDelete.empty());
        for (auto& decay : m_decays)
        {
            if (++decay.second > DecayCountToDeallocate)
                toDelete.push_back(decay.first);
        }
        for (const auto uboHandle : toDelete)
        {
            assert(uboHandle.getType() == SemanticUniformBuffersInstances::GetMatchingSemanticType<UBOData>());
            assert(std::find(m_dirtyUBOs.cbegin(), m_dirtyUBOs.cend(), uboHandle) == m_dirtyUBOs.cend());
            resourceManager.unloadUniformBuffer(uboHandle, m_sceneId);
            m_decays.erase(uboHandle);
            m_uboData.erase(uboHandle);
            m_deviceHandles.erase(uboHandle);
        }
        toDelete.clear();
    }

    template <typename UBOData>
    void SemanticUniformBuffers<UBOData>::uploadUpdatedUBOs(IRendererResourceManager& resourceManager)
    {
        for (const auto uboHandle : m_dirtyUBOs)
        {
            assert(uboHandle.getType() == SemanticUniformBuffersInstances::GetMatchingSemanticType<UBOData>());
            resourceManager.updateUniformBuffer(uboHandle, UBOSize, SemanticUniformBuffersInstances::GetUBODataPtr(m_uboData.find(uboHandle)->second), m_sceneId);
        }
        m_dirtyUBOs.clear();
    }

    template <typename UBOData>
    void SemanticUniformBuffers<UBOData>::upload(IRendererResourceManager& resourceManager)
    {
        // early out (decayed UBOs will be evaluated only on next change)
        if (m_dirtyUBOs.empty())
            return;

        uploadNewUBOs(resourceManager);
        decayAndUnloadUnusedUBOs(resourceManager);
        uploadUpdatedUBOs(resourceManager);
    }

    void SemanticUniformBuffersInstances::CalculateUBOData(SemanticUniformBufferHandle uboHandle, const SemanticUniformBufferScene& scene, ModelData& uboData)
    {
        assert(uboHandle.getType() == SemanticUniformBufferHandle::Type::Model);
        const auto& renderable = scene.getRenderable(uboHandle.getRenderable());
        const auto mMat = scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, renderable.node);

        std::memcpy(uboData.data(), glm::value_ptr(mMat), 16u * sizeof(float));
    }

    const std::byte* SemanticUniformBuffersInstances::GetUBODataPtr(const ModelData& uboData)
    {
        return uboData.data();
    }

    void SemanticUniformBuffersInstances::CalculateUBOData(SemanticUniformBufferHandle uboHandle, const SemanticUniformBufferScene& scene, CameraData& uboData)
    {
        assert(uboHandle.getType() == SemanticUniformBufferHandle::Type::Camera);
        const auto pMat = SemanticUniformBuffersInstances::CalculateProjectionMatrix(uboHandle.getCamera(), scene);
        const auto vMat = scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, scene.getCamera(uboHandle.getCamera()).node);
        const auto position = glm::inverse(vMat) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        std::memcpy(uboData.pMat.data(), glm::value_ptr(pMat), 16u * sizeof(float));
        std::memcpy(uboData.vMat.data(), glm::value_ptr(vMat), 16u * sizeof(float));
        std::memcpy(uboData.position.data(), glm::value_ptr(position), 3u * sizeof(float));
    }

    const std::byte* SemanticUniformBuffersInstances::GetUBODataPtr(const CameraData& uboData)
    {
        return uboData.pMat.data();
    }

    void SemanticUniformBuffersInstances::CalculateUBOData(SemanticUniformBufferHandle uboHandle, const SemanticUniformBufferScene& scene, ModelCameraData& uboData)
    {
        assert(uboHandle.getType() == SemanticUniformBufferHandle::Type::ModelCamera);
        const auto& renderable = scene.getRenderable(uboHandle.getRenderable());
        const auto mMat = scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, renderable.node);
        const auto vMat = scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, scene.getCamera(uboHandle.getCamera()).node);
        const auto pMat = SemanticUniformBuffersInstances::CalculateProjectionMatrix(uboHandle.getCamera(), scene);
        const auto mvMat = vMat * mMat;
        const auto mvpMat = pMat * mvMat;
        const auto normalMat = glm::transpose(glm::inverse(mvMat));

        std::memcpy(uboData.mvpMat.data(), glm::value_ptr(mvpMat), 16u * sizeof(float));
        std::memcpy(uboData.mvMat.data(), glm::value_ptr(mvMat), 16u * sizeof(float));
        std::memcpy(uboData.normalMat.data(), glm::value_ptr(normalMat), 16u * sizeof(float));
    }

    const std::byte* SemanticUniformBuffersInstances::GetUBODataPtr(const ModelCameraData& uboData)
    {
        return uboData.mvpMat.data();
    }

    glm::mat4 SemanticUniformBuffersInstances::CalculateProjectionMatrix(CameraHandle camera, const SemanticUniformBufferScene& scene)
    {
        const auto& projParams = scene.updateCameraProjectionParams(camera);
        return CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Frustum(
            scene.getCamera(camera).projectionType, projParams.first.x, projParams.first.y, projParams.first.z, projParams.first.w, projParams.second.x, projParams.second.y));
    }

    template class SemanticUniformBuffers<SemanticUniformBuffersInstances::ModelData>;
    template class SemanticUniformBuffers<SemanticUniformBuffersInstances::CameraData>;
    template class SemanticUniformBuffers<SemanticUniformBuffersInstances::ModelCameraData>;
}
