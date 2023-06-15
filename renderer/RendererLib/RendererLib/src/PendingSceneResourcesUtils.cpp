//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/PendingSceneResourcesUtils.h"
#include "RendererLib/IRendererResourceManager.h"
#include "RendererLib/SceneResourceUploader.h"
#include "RendererLib/FrameTimer.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/GeometryDataBuffer.h"

namespace ramses_internal
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
                wasCanceledOut = RemoveSceneResourceActionIfContained(currentActionsInOut, sceneResourceAction.handle, ESceneResourceAction_CreateRenderBuffer);
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
            default:
                break;
            }

            if (!wasCanceledOut)
            {
                currentActionsInOut.push_back(sceneResourceAction);
            }
        }
    }

    bool PendingSceneResourcesUtils::ApplySceneResourceActions(const SceneResourceActionVector& actions, const IScene& scene, IRendererResourceManager& resourceManager, const FrameTimer* frameTimer)
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
        for (auto actionIter = actions.begin(); actionIter != actions.end(); ++actionIter)
        {
            if (actionIter->handle == handle &&
                actionIter->action == action)
            {
                actions.erase(actionIter);
                return true;
            }
        }

        return false;
    }

    bool PendingSceneResourcesUtils::ContainsSceneResourceAction(const SceneResourceActionVector& actions, MemoryHandle handle, ESceneResourceAction action)
    {
        for (const auto& a : actions)
        {
            if (a.handle == handle &&
                a.action == action)
            {
                return true;
            }
        }

        return false;
    }
}
