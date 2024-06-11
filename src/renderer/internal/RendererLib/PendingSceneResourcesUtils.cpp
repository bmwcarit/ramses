//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PendingSceneResourcesUtils.h"
#include "internal/RendererLib/IRendererResourceManager.h"
#include "internal/RendererLib/SceneResourceUploader.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/FrameTimer.h"
#include "internal/SceneGraph/SceneAPI/GeometryDataBuffer.h"

namespace ramses::internal
{
    void PendingSceneResourcesUtils::ConsolidateSceneResourceActions(const SceneResourceActionVector& newActions, SceneResourceActionVector& currentActionsInOut)
    {
        // preallocate for maximum size - canceling out actions should be minimal in real use cases
        currentActionsInOut.reserve(currentActionsInOut.size() + newActions.size());

        for (const auto& sceneResourceAction : newActions)
        {
            bool wasCanceledOut = false;
            // Cancel out create and destroy action for the same resource.
            // This is needed because the create action cannot be executed if the scene resource
            // was removed from scene (scene actions are applied before applying scene resource actions)
            const ESceneResourceAction action = sceneResourceAction.action;
            switch (action)
            {
            case ESceneResourceAction_DestroyRenderTarget:
                wasCanceledOut = RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_CreateRenderTarget);
                break;
            case ESceneResourceAction_DestroyRenderBuffer:
                //remove all update actions first
                while (RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateRenderBufferProperties))
                {
                }
                wasCanceledOut = RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_CreateRenderBuffer);
                break;
            case ESceneResourceAction_UpdateRenderBufferProperties:
                //add update action if update action does not already exist
                wasCanceledOut = ContainsSceneResourceAction(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateRenderBufferProperties);
                break;
            case ESceneResourceAction_DestroyBlitPass:
                wasCanceledOut = RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_CreateBlitPass);
                break;
            case ESceneResourceAction_DestroyDataBuffer:
                //remove all update actions first
                while (RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateDataBuffer))
                {
                }
                wasCanceledOut = RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_CreateDataBuffer);
                break;
            case ESceneResourceAction_UpdateDataBuffer:
                //add update action iff update action does not already exist
                wasCanceledOut = ContainsSceneResourceAction(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateDataBuffer);
                break;
            case ESceneResourceAction_DestroyTextureBuffer:
                //remove all update actions first
                while (RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateTextureBuffer))
                {
                }
                wasCanceledOut = RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_CreateTextureBuffer);
                break;
            case ESceneResourceAction_UpdateTextureBuffer:
                //add update action iff update action does not already exist
                wasCanceledOut = ContainsSceneResourceAction(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateTextureBuffer);
                break;
            case ESceneResourceAction_UpdateUniformBuffer:
                //add update action iff update action does not already exist
                wasCanceledOut = ContainsSceneResourceAction(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateUniformBuffer);
                break;
            case ESceneResourceAction_DestroyUniformBuffer:
                //remove all update actions first
                while (RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_UpdateUniformBuffer))
                {
                }
                wasCanceledOut = RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_CreateUniformBuffer);
                break;

            default:
                break;
            }

            if (!wasCanceledOut)
            {
                currentActionsInOut.push_back(sceneResourceAction);
            }
        }
    }

    bool PendingSceneResourcesUtils::ApplySceneResourceActions(const SceneResourceActionVector& actions,
                                                               const RendererCachedScene&       scene,
                                                               IRendererResourceManager&        resourceManager,
                                                               const FrameTimer*                frameTimer)
    {
        constexpr size_t TimeCheckPeriod = 20u;
        constexpr size_t ThresholdForTimeChecking = 100u;
        const bool checkTimeSpent = (frameTimer != nullptr) && (actions.size() > ThresholdForTimeChecking);

        for (size_t i = 0u; i < actions.size(); ++i)
        {
            const MemoryHandle handle = actions[i].handle;
            switch (actions[i].action)
            {
            case ESceneResourceAction_CreateRenderTarget:
                SceneResourceUploader::UploadRenderTarget(scene, RenderTargetHandle(handle), resourceManager);
                break;
            case ESceneResourceAction_DestroyRenderTarget:
                resourceManager.unloadRenderTarget(RenderTargetHandle(handle), scene.getSceneId());
                break;
            case ESceneResourceAction_CreateRenderBuffer:
                SceneResourceUploader::UploadRenderBuffer(scene, RenderBufferHandle(handle), resourceManager);
                break;
            case ESceneResourceAction_DestroyRenderBuffer:
                resourceManager.unloadRenderTargetBuffer(RenderBufferHandle(handle), scene.getSceneId());
                break;
            case ESceneResourceAction_UpdateRenderBufferProperties:
                resourceManager.updateRenderTargetBufferProperties(RenderBufferHandle{ handle }, scene.getSceneId(), scene.getRenderBuffer(RenderBufferHandle{ handle }));
                break;
            case ESceneResourceAction_CreateBlitPass:
                SceneResourceUploader::UploadBlitPassRenderTargets(scene, BlitPassHandle(handle), resourceManager);
                break;
            case ESceneResourceAction_DestroyBlitPass:
                resourceManager.unloadBlitPassRenderTargets(BlitPassHandle(handle), scene.getSceneId());
                break;
            case ESceneResourceAction_CreateDataBuffer:
            {
                const GeometryDataBuffer& dataBuffer = scene.getDataBuffer(DataBufferHandle(handle));
                resourceManager.uploadDataBuffer(DataBufferHandle(handle), dataBuffer.bufferType, dataBuffer.dataType, static_cast<uint32_t>(dataBuffer.data.size()), scene.getSceneId());
            }
                break;
            case ESceneResourceAction_DestroyDataBuffer:
                resourceManager.unloadDataBuffer(DataBufferHandle(handle), scene.getSceneId());
                break;
            case ESceneResourceAction_UpdateDataBuffer:
            {
                const GeometryDataBuffer& dataBuffer = scene.getDataBuffer(DataBufferHandle(handle));
                resourceManager.updateDataBuffer(DataBufferHandle(handle), static_cast<uint32_t>(dataBuffer.data.size()), dataBuffer.data.data(), scene.getSceneId());
            }
                break;
            case ESceneResourceAction_CreateTextureBuffer:
                SceneResourceUploader::UploadTextureBuffer(scene, TextureBufferHandle(handle), resourceManager);
                break;
            case ESceneResourceAction_UpdateTextureBuffer:
                SceneResourceUploader::UpdateTextureBuffer(scene, TextureBufferHandle(handle), resourceManager);
                break;
            case ESceneResourceAction_DestroyTextureBuffer:
                resourceManager.unloadTextureBuffer(TextureBufferHandle(handle), scene.getSceneId());
                break;
            case ESceneResourceAction_CreateUniformBuffer:
            {
                const auto& uniformBuffer = scene.getUniformBuffer(UniformBufferHandle{ handle });
                resourceManager.uploadUniformBuffer(UniformBufferHandle{ handle }, uint32_t(uniformBuffer.data.size()), scene.getSceneId());
                break;
            }
            case ESceneResourceAction_DestroyUniformBuffer:
                resourceManager.unloadUniformBuffer(UniformBufferHandle{ handle }, scene.getSceneId());
                break;
            case ESceneResourceAction_UpdateUniformBuffer:
            {
                const auto& uniformBuffer = scene.getUniformBuffer(UniformBufferHandle{ handle });
                resourceManager.updateUniformBuffer(UniformBufferHandle{ handle }, uint32_t(uniformBuffer.data.size()), uniformBuffer.data.data(), scene.getSceneId());
                break;
            }
            default:
                assert(false && "unknown scene resource action");
                break;
            }

            if (checkTimeSpent && ((i + 1) % TimeCheckPeriod == 0))
            {
                if (frameTimer->isTimeBudgetExceededForSection(EFrameTimerSectionBudget::SceneResourcesUpload))
                    return false;
            }
        }

        return true;
    }

    bool PendingSceneResourcesUtils::RemoveSceneResourceActionIfContained(SceneResourceActionVector& actions, MemoryHandle handle, ESceneResourceAction action)
    {
        auto it = std::find(actions.begin(), actions.end(), SceneResourceAction{ handle, action });
        if (it != actions.end())
        {
            actions.erase(it);
            return true;
        }

        return false;
    }

    bool PendingSceneResourcesUtils::ContainsSceneResourceAction(const SceneResourceActionVector& actions, MemoryHandle handle, ESceneResourceAction action)
    {
        return std::find(actions.cbegin(), actions.cend(), SceneResourceAction{ handle, action }) != actions.cend();
    }
}
