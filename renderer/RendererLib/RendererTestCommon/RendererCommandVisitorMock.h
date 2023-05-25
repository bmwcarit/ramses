//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDVISITORMOCK_H
#define RAMSES_RENDERERCOMMANDVISITORMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/DisplayConfig.h"
#include "SceneAPI/SceneId.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class RendererCommandBuffer;
    class IBinaryShaderCache;

    class RendererCommandVisitorMock
    {
    public:
        RendererCommandVisitorMock();
        ~RendererCommandVisitorMock();

        void visit(const RendererCommands& cmds);
        void visit(RendererCommandBuffer& buffer);

        void operator()(const RendererCommand::ScenePublished& cmd)
        {
            handleScenePublished(cmd.scene, cmd.publicationMode);
        }

        void operator()(const RendererCommand::SceneUnpublished& cmd)
        {
            handleSceneUnpublished(cmd.scene);
        }

        void operator()(const RendererCommand::ReceiveScene& cmd)
        {
            handleSceneReceived(cmd.info);
        }

        void operator()(const RendererCommand::UpdateScene& cmd)
        {
            handleSceneUpdate(cmd.scene, cmd.updateData);
        }

        void operator()(const RendererCommand::SetSceneState& cmd)
        {
            setSceneState(cmd.scene, cmd.state);
        }

        void operator()(const RendererCommand::SetSceneMapping& cmd)
        {
            setSceneMapping(cmd.scene, cmd.display);
        }

        void operator()(const RendererCommand::SetSceneDisplayBufferAssignment& cmd)
        {
            setSceneDisplayBufferAssignment(cmd.scene, cmd.buffer, cmd.renderOrder);
        }

        void operator()(const RendererCommand::LinkData& cmd)
        {
            handleSceneDataLinkRequest(cmd.providerScene, cmd.providerData, cmd.consumerScene, cmd.consumerData);
        }

        void operator()(const RendererCommand::LinkOffscreenBuffer& cmd)
        {
            handleBufferToSceneDataLinkRequest(cmd.providerBuffer, cmd.consumerScene, cmd.consumerData);
        }

        void operator()(const RendererCommand::LinkStreamBuffer& cmd)
        {
            handleBufferToSceneDataLinkRequest(cmd.providerBuffer, cmd.consumerScene, cmd.consumerData);
        }

        void operator()(const RendererCommand::LinkExternalBuffer& cmd)
        {
            handleBufferToSceneDataLinkRequest(cmd.providerBuffer, cmd.consumerScene, cmd.consumerData);
        }

        void operator()(const RendererCommand::UnlinkData& cmd)
        {
            handleDataUnlinkRequest(cmd.consumerScene, cmd.consumerData);
        }

        void operator()(const RendererCommand::CreateDisplay& cmd)
        {
            createDisplayContext(cmd.config, cmd.display, cmd.binaryShaderCache);
        }

        void operator()(const RendererCommand::DestroyDisplay& cmd)
        {
            destroyDisplayContext(cmd.display);
        }

        void operator()(const RendererCommand::CreateOffscreenBuffer& cmd)
        {
            handleBufferCreateRequest(cmd.offscreenBuffer, cmd.display, cmd.width, cmd.height, cmd.sampleCount, cmd.interruptible, cmd.depthStencilBufferType);
        }

        void operator()(const RendererCommand::CreateDmaOffscreenBuffer& cmd)
        {
            handleDmaBufferCreateRequest(cmd.offscreenBuffer, cmd.display, cmd.width, cmd.height, cmd.dmaBufferFourccFormat, cmd.dmaBufferUsageFlags, cmd.dmaBufferModifiers);
        }

        void operator()(const RendererCommand::DestroyOffscreenBuffer& cmd)
        {
            handleBufferDestroyRequest(cmd.offscreenBuffer, cmd.display);
        }

        void operator()(const RendererCommand::CreateStreamBuffer& cmd)
        {
            handleBufferCreateRequest(cmd.streamBuffer, cmd.display, cmd.source);
        }

        void operator()(const RendererCommand::DestroyStreamBuffer& cmd)
        {
            handleBufferDestroyRequest(cmd.streamBuffer, cmd.display);
        }

        void operator()(const RendererCommand::CreateExternalBuffer& cmd)
        {
            handleExternalBufferCreateRequest(cmd.externalBuffer, cmd.display);
        }

        void operator()(const RendererCommand::DestroyExternalBuffer& cmd)
        {
            handleExternalBufferDestroyRequest(cmd.externalBuffer, cmd.display);
        }

        void operator()(const RendererCommand::SetClearFlags& cmd)
        {
            handleSetClearFlags(cmd.display, cmd.offscreenBuffer, cmd.clearFlags);
        }

        void operator()(const RendererCommand::SetClearColor& cmd)
        {
            handleSetClearColor(cmd.display, cmd.offscreenBuffer, cmd.clearColor);
        }

        void operator()(const RendererCommand::SetExterallyOwnedWindowSize& cmd)
        {
            handleSetExternallyOwnedWindowSize(cmd.display, cmd.width, cmd.height);
        }

        void operator()(const RendererCommand::PickEvent& cmd)
        {
            handlePick(cmd.scene, cmd.coordsNormalizedToBufferSize);
        }

        void operator()(const RendererCommand::ReadPixels& cmd)
        {
            handleReadPixels(cmd.display, cmd.offscreenBuffer, cmd.offsetX, cmd.offsetY, cmd.width, cmd.height, cmd.fullScreen, cmd.filename);
        }

        void operator()(const RendererCommand::SCListIviSurfaces&)
        {
            systemCompositorListIviSurfaces();
        }

        void operator()(const RendererCommand::SCSetIviSurfaceVisibility& cmd)
        {
            systemCompositorSetIviSurfaceVisibility(cmd.surface, cmd.visibility);
        }

        void operator()(const RendererCommand::SCSetIviSurfaceOpacity& cmd)
        {
            systemCompositorSetIviSurfaceOpacity(cmd.surface, cmd.opacity);
        }

        void operator()(const RendererCommand::SCAddIviSurfaceToIviLayer& cmd)
        {
            systemCompositorAddIviSurfaceToIviLayer(cmd.surface, cmd.layer);
        }

        void operator()(const RendererCommand::SCSetIviLayerVisibility& cmd)
        {
            systemCompositorSetIviLayerVisibility(cmd.layer, cmd.visibility);
        }

        void operator()(const RendererCommand::SCRemoveIviSurfaceFromIviLayer& cmd)
        {
            systemCompositorRemoveIviSurfaceFromIviLayer(cmd.surface, cmd.layer);
        }

        void operator()(const RendererCommand::SCDestroyIviSurface& cmd)
        {
            systemCompositorDestroyIviSurface(cmd.surface);
        }

        void operator()(const RendererCommand::SCSetIviSurfaceDestRectangle& cmd)
        {
            systemCompositorSetIviSurfaceDestRectangle(cmd.surface, cmd.x, cmd.y, cmd.width, cmd.height);
        }

        void operator()(const RendererCommand::SCScreenshot& cmd)
        {
            systemCompositorScreenshot(cmd.filename, cmd.screenId);
        }

        void operator()(const RendererCommand::LogInfo& cmd)
        {
            logInfo(cmd.topic, cmd.verbose, cmd.nodeFilter);
        }

        void operator()(const RendererCommand::SetLimits_FrameBudgets& cmd)
        {
            setLimitsFrameBudgets(cmd.limitForSceneResourcesUploadMicrosec, cmd.limitForResourcesUploadMicrosec, cmd.limitForOffscreenBufferRenderMicrosec);
        }

        void operator()(const RendererCommand::SetSkippingOfUnmodifiedBuffers& cmd)
        {
            setSkippingOfUnmodifiedBuffers(cmd.enable);
        }

        void operator()(const RendererCommand::ConfirmationEcho& cmd)
        {
            handleConfirmationEcho(cmd.display, cmd.text);
        }

        template <typename T>
        void operator()(const T&)
        {
            assert(!"unhandled command");
        }

        MOCK_METHOD(void, handleScenePublished, (SceneId, EScenePublicationMode));
        MOCK_METHOD(void, handleSceneUnpublished, (SceneId));
        MOCK_METHOD(void, handleSceneReceived, (const SceneInfo&));
        MOCK_METHOD(void, handleSceneUpdate, (SceneId, const SceneUpdate&));
        MOCK_METHOD(void, setSceneState, (SceneId, RendererSceneState));
        MOCK_METHOD(void, setSceneMapping, (SceneId, DisplayHandle));
        MOCK_METHOD(void, setSceneDisplayBufferAssignment, (SceneId, OffscreenBufferHandle, int32_t));
        MOCK_METHOD(void, createDisplayContext, (const DisplayConfig&, DisplayHandle, IBinaryShaderCache*));
        MOCK_METHOD(void, destroyDisplayContext, (DisplayHandle));
        MOCK_METHOD(void, handleSceneDataLinkRequest, (SceneId, DataSlotId, SceneId, DataSlotId));
        MOCK_METHOD(void, handleDataUnlinkRequest, (SceneId, DataSlotId));
        MOCK_METHOD(void, handleSetClearFlags, (DisplayHandle, OffscreenBufferHandle, uint32_t));
        MOCK_METHOD(void, handleSetClearColor, (DisplayHandle, OffscreenBufferHandle, const glm::vec4&));
        MOCK_METHOD(void, handleSetExternallyOwnedWindowSize, (DisplayHandle, uint32_t, uint32_t));
        MOCK_METHOD(void, handlePick, (SceneId, const glm::vec2&));
        MOCK_METHOD(void, handleBufferCreateRequest, (OffscreenBufferHandle, DisplayHandle, uint32_t, uint32_t, uint32_t, bool, ERenderBufferType));
        MOCK_METHOD(void, handleDmaBufferCreateRequest, (OffscreenBufferHandle, DisplayHandle, uint32_t, uint32_t, DmaBufferFourccFormat, DmaBufferUsageFlags, DmaBufferModifiers));
        MOCK_METHOD(void, handleBufferDestroyRequest, (OffscreenBufferHandle, DisplayHandle));
        MOCK_METHOD(void, handleBufferCreateRequest, (StreamBufferHandle, DisplayHandle, WaylandIviSurfaceId));
        MOCK_METHOD(void, handleBufferDestroyRequest, (StreamBufferHandle, DisplayHandle));
        MOCK_METHOD(void, handleExternalBufferCreateRequest, (ExternalBufferHandle, DisplayHandle));
        MOCK_METHOD(void, handleExternalBufferDestroyRequest, (ExternalBufferHandle, DisplayHandle));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (OffscreenBufferHandle, SceneId, DataSlotId));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (StreamBufferHandle, SceneId, DataSlotId));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (ExternalBufferHandle, SceneId, DataSlotId));
        MOCK_METHOD(void, handleReadPixels, (DisplayHandle, OffscreenBufferHandle, uint32_t, uint32_t, uint32_t, uint32_t, bool, const String&));
        MOCK_METHOD(void, systemCompositorListIviSurfaces, ());
        MOCK_METHOD(void, systemCompositorSetIviSurfaceVisibility, (WaylandIviSurfaceId, bool));
        MOCK_METHOD(void, systemCompositorSetIviSurfaceOpacity, (WaylandIviSurfaceId, float));
        MOCK_METHOD(void, systemCompositorAddIviSurfaceToIviLayer, (WaylandIviSurfaceId, WaylandIviLayerId));
        MOCK_METHOD(void, systemCompositorSetIviLayerVisibility, (WaylandIviLayerId, bool));
        MOCK_METHOD(void, systemCompositorRemoveIviSurfaceFromIviLayer, (WaylandIviSurfaceId, WaylandIviLayerId));
        MOCK_METHOD(void, systemCompositorDestroyIviSurface, (WaylandIviSurfaceId));
        MOCK_METHOD(void, systemCompositorSetIviSurfaceDestRectangle, (WaylandIviSurfaceId, int32_t, int32_t, int32_t, int32_t));
        MOCK_METHOD(void, systemCompositorScreenshot, (const String&, int32_t));
        MOCK_METHOD(void, logInfo, (ERendererLogTopic, bool, NodeHandle));
        MOCK_METHOD(void, setLimitsFrameBudgets, (uint64_t, uint64_t, uint64_t));
        MOCK_METHOD(void, setSkippingOfUnmodifiedBuffers, (bool));
        MOCK_METHOD(void, handleConfirmationEcho, (DisplayHandle, const String&));
    };
}

#endif
