//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENEUPDATERTEST_H
#define RAMSES_RENDERERSCENEUPDATERTEST_H

#include "renderer_common_gmock_header.h"
#include "RendererMock.h"
#include "RendererEventCollector.h"
#include "SceneAPI/TextureSampler.h"
#include "Scene/ActionCollectingScene.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DisplayEventHandlerManager.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "Animation/AnimationSystem.h"
#include "Animation/ActionCollectingAnimationSystem.h"
#include "Scene/SceneDataBinding.h"
#include "ComponentMocks.h"
#include "ResourceProviderMock.h"
#include "RendererResourceCacheMock.h"
#include "SceneAllocateHelper.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Collections/Pair.h"

namespace ramses_internal {

class ARendererSceneUpdater : public ::testing::Test
{
public:
    ARendererSceneUpdater()
        : renderableHandle(1u)
        , uniformDataInstanceHandle(0u)
        , geometryDataInstanceHandle(1u)
        , samplerHandle(2u)
        , streamTextureHandle(0u)
        , platformFactoryMock()
        , rendererEventCollector()
        , rendererScenes(rendererEventCollector)
        , expirationMonitor(rendererScenes, rendererEventCollector)
        , renderer(platformFactoryMock, rendererScenes, rendererEventCollector, expirationMonitor, rendererStatistics)
        , sceneGraphConsumerComponent()
        , resourceUploader(renderer.getStatistics())
        , sceneStateExecutor(renderer, sceneGraphConsumerComponent, rendererEventCollector)
        , rendererSceneUpdater(new RendererSceneUpdater(renderer, rendererScenes, sceneStateExecutor, rendererEventCollector, frameTimer, expirationMonitor, &rendererResourceCacheMock))
    {
        frameTimer.startFrame();
        rendererSceneUpdater->setLimitFlushesForceApply(ForceApplyFlushesLimit);
        rendererSceneUpdater->setLimitFlushesForceUnsubscribe(ForceUnsubscribeFlushLimit);
    }

    ~ARendererSceneUpdater()
    {
        delete rendererSceneUpdater;
    }

protected:
    void update()
    {
        renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
        rendererSceneUpdater->updateScenes();
    }

    UInt32 createStagingScene()
    {
        const UInt32 sceneIndex = static_cast<UInt32>(stagingScene.size());
        const SceneId sceneId(sceneIndex);
        stagingScene.emplace_back(new ActionCollectingScene(SceneInfo(sceneId)));

        return sceneIndex;
    }

    SceneId getSceneId(UInt32 sceneIndex = 0u)
    {
        return stagingScene[sceneIndex]->getSceneId();
    }

    void publishScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = getSceneId(sceneIndex);
        rendererSceneUpdater->handleScenePublished(sceneId, Guid(true), EScenePublicationMode_LocalAndRemote);
        EXPECT_TRUE(sceneStateExecutor.getSceneState(sceneId) == ESceneState::Published);
        expectEvent(ERendererEventType_ScenePublished);
    }

    void requestSceneSubscription(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = getSceneId(sceneIndex);
        EXPECT_CALL(sceneGraphConsumerComponent, subscribeScene(_, sceneId));
        rendererSceneUpdater->handleSceneSubscriptionRequest(sceneId);
        EXPECT_TRUE(sceneStateExecutor.getSceneState(sceneId) == ESceneState::SubscriptionRequested);
    }

    void receiveScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = getSceneId(sceneIndex);
        rendererSceneUpdater->handleSceneReceived(SceneInfo(sceneId));
        EXPECT_TRUE(sceneStateExecutor.getSceneState(sceneId) == ESceneState::SubscriptionPending);
        EXPECT_TRUE(rendererScenes.hasScene(sceneId));
    }

    UInt32 createPublishAndSubscribeScene(bool withInitialFlush = true)
    {
        const UInt32 sceneIndex = createStagingScene();
        const SceneId sceneId = getSceneId(sceneIndex);

        publishScene(sceneIndex);
        requestSceneSubscription(sceneIndex);
        receiveScene(sceneIndex);

        if (withInitialFlush)
        {
            //receive initial flush
            performFlush(sceneIndex);
            EXPECT_TRUE(sceneStateExecutor.getSceneState(sceneId) == ESceneState::Subscribed);
            expectEvents({ ERendererEventType_SceneSubscribed });
        }

        return sceneIndex;
    }

    void expectEvents(const std::initializer_list<ERendererEventType> expectedEvents)
    {
        RendererEventVector events;
        rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(expectedEvents.size(), events.size());
        auto eventIt = events.cbegin();
        for (auto expectedEvent : expectedEvents)
        {
            EXPECT_EQ(expectedEvent, eventIt->eventType);
            ++eventIt;
        }
    }

    void expectNoEvent()
    {
        expectEvents({});
    }

    void expectEvent(ERendererEventType eventType)
    {
        expectEvents({ eventType });
    }

    void requestMapScene(UInt32 sceneIndex = 0u, DisplayHandle displayHandle = DisplayHandle1)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneMappingRequest(sceneId, displayHandle, 0u);
        expectNoEvent();
    }

    void mapScene(UInt32 sceneIndex = 0u, DisplayHandle displayHandle = DisplayHandle1)
    {
        requestMapScene(sceneIndex, displayHandle);
        update(); // will set from map requested to being mapped and uploaded (if all pending flushes applied)
        update(); // will set to mapped (if all resources uploaded)
        expectEvent(ERendererEventType_SceneMapped);
    }

    bool assignSceneToOffscreenBuffer(UInt32 sceneIndex, OffscreenBufferHandle buffer)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        return rendererSceneUpdater->handleSceneOffscreenBufferAssignmentRequest(sceneId, buffer);
    }

    bool assignSceneToFramebuffer(UInt32 sceneIndex)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        return rendererSceneUpdater->handleSceneFramebufferAssignmentRequest(sceneId);
    }

    void showScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneShowRequest(sceneId);
        update();
        expectEvent(ERendererEventType_SceneShown);
    }

    void hideScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneHideRequest(sceneId);
        expectEvent(ERendererEventType_SceneHidden);
    }

    void unmapScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneUnmappingRequest(sceneId);
        expectEvent(ERendererEventType_SceneUnmapped);
    }

    void unpublishMapRequestedScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, sceneId));
        rendererSceneUpdater->handleSceneUnpublished(sceneId);
        expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnsubscribedIndirect, ERendererEventType_SceneUnpublished });
        EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
    }

    void unpublishMappedScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, sceneId));
        rendererSceneUpdater->handleSceneUnpublished(sceneId);
        expectEvents({ ERendererEventType_SceneUnmappedIndirect, ERendererEventType_SceneUnsubscribedIndirect, ERendererEventType_SceneUnpublished });
        EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
    }

    void unpublishShownScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(_, sceneId));
        rendererSceneUpdater->handleSceneUnpublished(sceneId);
        expectEvents({ ERendererEventType_SceneHiddenIndirect, ERendererEventType_SceneUnmappedIndirect, ERendererEventType_SceneUnsubscribedIndirect, ERendererEventType_SceneUnpublished });

        EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
    }

    void unsubscribeMapRequestedScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneUnsubscriptionRequest(sceneId, true);

        expectEvents({ ERendererEventType_SceneMapFailed, ERendererEventType_SceneUnsubscribedIndirect });

        EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
    }

    void unsubscribeMappedScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneUnsubscriptionRequest(sceneId, true);

        expectEvents({ ERendererEventType_SceneUnmappedIndirect, ERendererEventType_SceneUnsubscribedIndirect });

        EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
    }

    void unsubscribeShownScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneUnsubscriptionRequest(sceneId, true);

        expectEvents({ ERendererEventType_SceneHiddenIndirect, ERendererEventType_SceneUnmappedIndirect, ERendererEventType_SceneUnsubscribedIndirect });
        EXPECT_FALSE(renderer.getDisplaySceneIsMappedTo(sceneId).isValid());
    }

    NodeHandle performFlushWithCreateNodeAction(UInt32 sceneIndex = 0u, size_t numNodes = 1u, bool synchronous = false)
    {
        NodeHandle nodeHandle;
        IScene& scene = *stagingScene[sceneIndex];
        SceneAllocateHelper sceneAllocator(scene);
        for (size_t i = 0; i < numNodes; ++i)
            nodeHandle = sceneAllocator.allocateNode();
        performFlush(sceneIndex, synchronous);

        return nodeHandle;
    }

    void expectContextEnable(DisplayHandle displayHandle = DisplayHandle1, UInt32 times = 1u)
    {
        EXPECT_CALL(*renderer.getDisplayMock(displayHandle).m_displayController, getRenderBackend()).Times(times);
        EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->surfaceMock, enable()).Times(times);
    }

    void performFlush(UInt32 sceneIndex = 0u, bool synchronous = false, SceneVersionTag version = InvalidSceneVersionTag, const SceneSizeInformation* sizeInfo = nullptr, const FlushTimeInformation& timeInfo = {})
    {
        ActionCollectingScene& scene = *stagingScene[sceneIndex];
        const SceneSizeInformation newSizeInfo = (sizeInfo ? *sizeInfo : scene.getSceneSizeInformation());
        const SceneSizeInformation currSizeInfo = rendererScenes.hasScene(scene.getSceneId()) ? rendererScenes.getScene(scene.getSceneId()).getSceneSizeInformation() : SceneSizeInformation();

        SceneActionCollection sceneActions(std::move(scene.getSceneActionCollection()));

        SceneActionCollectionCreator creator(sceneActions);
        creator.flush(1u, synchronous, newSizeInfo > currSizeInfo, newSizeInfo, scene.getResourceChanges(), timeInfo, version);
        scene.clearResourceChanges();
        rendererSceneUpdater->handleSceneActions(stagingScene[sceneIndex]->getSceneId(), sceneActions);
    }

    void performFlushWithExpiration(UInt32 sceneIndex, UInt32 expirationTS)
    {
        const FlushTimeInformation timeInfo{ FlushTime::Clock::time_point(std::chrono::milliseconds(expirationTS)), {} };
        performFlush(sceneIndex, true, {}, nullptr, timeInfo);
    }

    bool lastFlushWasAppliedOnRendererScene(UInt32 sceneIndex = 0u)
    {
        return !rendererSceneUpdater->hasPendingFlushes(stagingScene[sceneIndex]->getSceneId());
    }

    void createDisplayAndExpectSuccess(DisplayHandle displayHandle = DisplayHandle1, const DisplayConfig& displayConfig = DisplayConfig())
    {
        rendererSceneUpdater->createDisplayContext(displayConfig, (displayHandle == DisplayHandle1 ? resourceProvider1 : resourceProvider2), resourceUploader, displayHandle);
        EXPECT_TRUE(renderer.hasDisplayController(displayHandle));
        expectEvent(ERendererEventType_DisplayCreated);
    }

    void destroyDisplay(DisplayHandle displayHandle = DisplayHandle1, bool expectFail = false)
    {
        if (!expectFail)
        {
            expectContextEnable(displayHandle);
        }
        rendererSceneUpdater->destroyDisplayContext(displayHandle);
        if (expectFail)
        {
            expectEvent(ERendererEventType_DisplayDestroyFailed);
        }
        else
        {
            EXPECT_FALSE(renderer.hasDisplayController(displayHandle));
            expectEvent(ERendererEventType_DisplayDestroyed);
        }
    }

    void createRenderable(UInt32 sceneIndex = 0u, bool synchronous = false, bool withVertexArray = false, bool withTextureSampler = false)
    {
        const NodeHandle renderableNode(1u);
        const RenderPassHandle renderPassHandle(2u);
        const RenderGroupHandle renderGroupHandle(3u);
        const CameraHandle cameraHandle(4u);
        const DataLayoutHandle uniformDataLayoutHandle(0u);
        const DataLayoutHandle geometryDataLayoutHandle(1u);
        const DataLayoutHandle camDataLayoutHandle(2u);
        const DataInstanceHandle camDataHandle(2u);

        IScene& scene = *stagingScene[sceneIndex];

        scene.allocateRenderPass(0u, renderPassHandle);
        scene.allocateRenderGroup(0u, 0u, renderGroupHandle);
        scene.allocateNode(0u, renderableNode);
        scene.allocateRenderable(renderableNode, renderableHandle);
        scene.allocateDataLayout({ {EDataType_Vector2I}, {EDataType_Vector2I} }, camDataLayoutHandle);
        scene.allocateCamera(ECameraProjectionType_Perspective, scene.allocateNode(), scene.allocateDataInstance(camDataLayoutHandle, camDataHandle), cameraHandle);

        scene.addRenderableToRenderGroup(renderGroupHandle, renderableHandle, 0u);
        scene.addRenderGroupToRenderPass(renderPassHandle, renderGroupHandle, 0u);
        scene.setRenderPassCamera(renderPassHandle, cameraHandle);
        scene.setRenderPassEnabled(renderPassHandle, true);

        DataFieldInfoVector uniformDataFields;
        if (withTextureSampler)
        {
            uniformDataFields.push_back({ EDataType_TextureSampler });
        }
        scene.allocateDataLayout(uniformDataFields, uniformDataLayoutHandle);
        scene.allocateDataInstance(uniformDataLayoutHandle, uniformDataInstanceHandle);
        scene.setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, uniformDataInstanceHandle);

        DataFieldInfoVector geometryDataFields;
        geometryDataFields.push_back({ EDataType_Indices, 1u, EFixedSemantics_Indices });
        if (withVertexArray)
        {
            geometryDataFields.push_back({ EDataType_Vector3Buffer, 1u, EFixedSemantics_VertexPositionAttribute });
        }
        scene.allocateDataLayout(geometryDataFields, geometryDataLayoutHandle);
        scene.allocateDataInstance(geometryDataLayoutHandle, geometryDataInstanceHandle);
        scene.setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Geometry, geometryDataInstanceHandle);

        performFlush(sceneIndex, synchronous);
    }

    void destroyRenderable(UInt32 sceneIndex = 0u, bool synchronous = false)
    {
        const RenderGroupHandle renderGroupHandle(0u);
        IScene& scene = *stagingScene[sceneIndex];
        scene.removeRenderableFromRenderGroup(renderGroupHandle, renderableHandle);
        scene.releaseRenderable(renderableHandle);

        performFlush(sceneIndex, synchronous);
    }

    void setRenderableResources(UInt32 sceneIndex = 0u, ResourceContentHash indexArrayHash = ResourceProviderMock::FakeIndexArrayHash, bool synchronous = false)
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.setRenderableEffect(renderableHandle, ResourceProviderMock::FakeEffectHash);
        scene.setDataResource(geometryDataInstanceHandle, DataFieldHandle(0u), indexArrayHash, DataBufferHandle::Invalid(), 0u);

        performFlush(sceneIndex, synchronous);
    }

    void setRenderableVertexArray(UInt32 sceneIndex = 0u, ResourceContentHash vertexArrayHash = ResourceProviderMock::FakeVertArrayHash, bool synchronous = false)
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.setDataResource(geometryDataInstanceHandle, DataFieldHandle(1u), vertexArrayHash, DataBufferHandle::Invalid(), 0u);
        performFlush(sceneIndex, synchronous);
    }

    void setRenderableStreamTexture(UInt32 sceneIndex = 0u)
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.setDataTextureSamplerHandle(uniformDataInstanceHandle, DataFieldHandle(0u), samplerHandle);
        performFlush(sceneIndex);
    }

    void createSamplerWithStreamTexture(UInt32 sceneIndex = 0u)
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.allocateStreamTexture(1u, ResourceContentHash::Invalid(), streamTextureHandle);
        scene.allocateTextureSampler({ {}, streamTextureHandle }, samplerHandle);
    }

    void createRenderableAndResourcesWithStreamTexture(UInt32 sceneIndex = 0u, bool uploadClientResources = true)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        createRenderable(sceneIndex, false, false, true);
        setRenderableResources(sceneIndex);

        createSamplerWithStreamTexture(sceneIndex);
        setRenderableStreamTexture(sceneIndex);

        expectResourceRequest(DisplayHandle1, sceneIndex);
        if (uploadClientResources)
        {
            expectContextEnable();
            expectRenderableResourcesUploaded(DisplayHandle1, true, true, false, true);
        }
        else
        {
            EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, uploadStreamTexture(_, _, sceneId));
        }
    }

    void removeRenderableResources(UInt32 sceneIndex = 0u, bool synchronous = false)
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.setRenderableEffect(renderableHandle, ResourceContentHash::Invalid());
        // it is not allowed to set both resource and data buffer as invalid, so for the purpose of the test cases here we set to non-existent data buffer with handle 0
        scene.setDataResource(geometryDataInstanceHandle, DataFieldHandle(0u), ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u);

        performFlush(sceneIndex, synchronous);
    }

    void expectResourceRequest(DisplayHandle displayHandle = DisplayHandle1, UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        EXPECT_CALL((displayHandle == DisplayHandle1 ? resourceProvider1 : resourceProvider2), requestResourceAsyncronouslyFromFramework(_, _, sceneId));
    }

    void expectRenderableResourcesUploaded(DisplayHandle displayHandle = DisplayHandle1, bool effect = true, bool indexBuffer = true, bool vertexArray = false, bool streamTexture = false)
    {
        if (effect)
        {
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, uploadShader(_));
        }
        if (indexBuffer)
        {
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, allocateIndexBuffer(_, _));
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, uploadIndexBufferData(_, _, _));
        }
        if (vertexArray)
        {
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, allocateVertexBuffer(_, _));
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, uploadVertexBufferData(_, _, _));
        }

        if (streamTexture)
        {
            EXPECT_CALL(*renderer.getDisplayMock(displayHandle).m_embeddedCompositingManager, uploadStreamTexture(_, _, _));
        }
    }

    void expectResourceRequestCancel(ResourceContentHash hash, DisplayHandle displayHandle = DisplayHandle1)
    {
        EXPECT_CALL((displayHandle == DisplayHandle1 ? resourceProvider1 : resourceProvider2), cancelResourceRequest(hash, _));
    }

    void expectRenderableResourcesDeleted(DisplayHandle displayHandle = DisplayHandle1, bool effect = true, bool indexBuffer = true, bool vertexArray = false, bool streamTexture = false)
    {
        if (effect)
        {
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, deleteShader(_));
        }
        if (indexBuffer)
        {
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, deleteIndexBuffer(_));
        }
        if (vertexArray)
        {
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, deleteVertexBuffer(_));
        }

        if (streamTexture)
        {
            EXPECT_CALL(*renderer.getDisplayMock(displayHandle).m_embeddedCompositingManager, deleteStreamTexture(_, _, _));
        }
    }

    void expectRenderTargetUploaded(DisplayHandle displayHandle = DisplayHandle1, bool expectClear = false, DeviceResourceHandle rtDeviceHandleToReturn = DeviceMock::FakeRenderTargetDeviceHandle, bool doubleBuffered = false)
    {
        {
            InSequence seq;
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, uploadRenderBuffer(_)).Times(2u);
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, uploadRenderTarget(_)).WillOnce(Return(rtDeviceHandleToReturn));

            if (doubleBuffered)
            {
                EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, uploadRenderBuffer(_));
                EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, uploadRenderTarget(_));
                EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
            }
        }

        if (expectClear)
        {
            const UInt32 expectationTimes = doubleBuffered ? 2u : 1u;
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, activateRenderTarget(_)).Times(expectationTimes);
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, colorMask(true, true, true, true)).Times(expectationTimes);
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f })).Times(expectationTimes);
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, depthWrite(EDepthWrite::Enabled)).Times(expectationTimes);
            RenderState::ScissorRegion scissorRegion{};
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion)).Times(expectationTimes);
            EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, clear(_)).Times(expectationTimes);
        }
    }

    void expectBlitPassUploaded()
    {
        {
            InSequence seq;
            EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, uploadRenderBuffer(_)).Times(2u);
            EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, uploadRenderTarget(_)).Times(2u);
        }
    }

    void expectRenderTargetDeleted(DisplayHandle displayHandle = DisplayHandle1, UInt32 times = 1u, bool doubleBuffered = false)
    {
        EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, deleteRenderBuffer(_)).Times((doubleBuffered? 3u : 2u) * times);
        EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, deleteRenderTarget(_)).Times((doubleBuffered ? 2u : 1u) * times);
        EXPECT_CALL(renderer.getDisplayMock(displayHandle).m_renderBackend->deviceMock, unpairRenderTargets(_)).Times((doubleBuffered ? times : 0u));
    }

    void expectBlitPassDeleted()
    {
        EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderTarget(_)).Times(2u);
        EXPECT_CALL(renderer.getDisplayMock(DisplayHandle1).m_renderBackend->deviceMock, deleteRenderBuffer(_)).Times(2u);
    }

    void createRenderTargetWithBuffers(UInt32 sceneIndex = 0u, bool synchronous = false, RenderTargetHandle renderTargetHandle = RenderTargetHandle{ 0u }, RenderBufferHandle bufferHandle = RenderBufferHandle{ 0u }, RenderBufferHandle depthHandle = RenderBufferHandle{ 1u })
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u }, bufferHandle);
        scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_DepthBuffer, ETextureFormat_Depth24, ERenderBufferAccessMode_ReadWrite, 0u }, depthHandle);
        scene.allocateRenderTarget(renderTargetHandle);
        scene.addRenderTargetRenderBuffer(renderTargetHandle, bufferHandle);
        scene.addRenderTargetRenderBuffer(renderTargetHandle, depthHandle);
        performFlush(sceneIndex, synchronous);
    }

    void createBlitPass()
    {
        const BlitPassHandle blitPassHandle(0u);
        IScene& scene = *stagingScene[0];
        const RenderBufferHandle sourceBufferHandle = scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });
        const RenderBufferHandle destinationBufferHandle = scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });
        scene.allocateBlitPass(sourceBufferHandle, destinationBufferHandle, blitPassHandle);
        performFlush();
    }

    void destroyRenderTargetWithBuffers(UInt32 sceneIndex = 0u, bool synchronous = false)
    {
        const RenderTargetHandle renderTargetHandle(0u);
        const RenderBufferHandle bufferHandle(0u);
        const RenderBufferHandle depthHandle(1u);
        IScene& scene = *stagingScene[sceneIndex];
        scene.releaseRenderTarget(renderTargetHandle);
        scene.releaseRenderBuffer(bufferHandle);
        scene.releaseRenderBuffer(depthHandle);
        performFlush(sceneIndex, synchronous);
    }

    void destroyBlitPass()
    {
        const BlitPassHandle blitPassHandle(0u);
        IScene& scene = *stagingScene[0];
        scene.releaseBlitPass(blitPassHandle);
        performFlush();
    }

    std::pair<DataSlotId, DataSlotId> createDataSlots(DataInstanceHandle& dataRefConsumer, float providedValue, DataInstanceHandle* dataRefProviderOut = nullptr, UInt32 providerSceneIdx = 0u, UInt32 consumerSceneIdx = 1u)
    {
        DataInstanceHandle dataRefProvider;

        IScene& scene1 = *stagingScene[providerSceneIdx];
        IScene& scene2 = *stagingScene[consumerSceneIdx];

        const DataLayoutHandle dataLayout1 = scene1.allocateDataLayout({ DataFieldInfo(EDataType_Float) });
        const DataLayoutHandle dataLayout2 = scene2.allocateDataLayout({ DataFieldInfo(EDataType_Float) });
        dataRefProvider = scene1.allocateDataInstance(dataLayout1);
        dataRefConsumer = scene2.allocateDataInstance(dataLayout2);
        if (nullptr != dataRefProviderOut)
            *dataRefProviderOut = dataRefProvider;

        scene1.setDataSingleFloat(dataRefProvider, DataFieldHandle(0u), providedValue);
        scene2.setDataSingleFloat(dataRefConsumer, DataFieldHandle(0u), 0.f);

        const DataSlotId providerId(getNextFreeDataSlotIdForDataLinking());
        const DataSlotId consumerId(getNextFreeDataSlotIdForDataLinking());
        scene1.allocateDataSlot({ EDataSlotType_DataProvider, providerId, NodeHandle(), dataRefProvider, ResourceContentHash::Invalid(), TextureSamplerHandle() });
        scene2.allocateDataSlot({ EDataSlotType_DataConsumer, consumerId, NodeHandle(), dataRefConsumer, ResourceContentHash::Invalid(), TextureSamplerHandle() });

        performFlush(providerSceneIdx);
        performFlush(consumerSceneIdx);
        update();
        expectEvents({ ERendererEventType_SceneDataSlotConsumerCreated, ERendererEventType_SceneDataSlotProviderCreated });

        return{ providerId, consumerId };
    }

    void createDataSlotsAndLinkThem(DataInstanceHandle& dataRefConsumer, float providedValue, DataInstanceHandle* dataRefProviderOut = nullptr, UInt32 providerSceneIdx = 0u, UInt32 consumerSceneIdx = 1u)
    {
        const auto providerConsumer = createDataSlots(dataRefConsumer, providedValue, dataRefProviderOut, providerSceneIdx, consumerSceneIdx);
        linkProviderToConsumer(getSceneId(providerSceneIdx), providerConsumer.first, getSceneId(consumerSceneIdx), providerConsumer.second);
    }

    void updateProviderDataSlot(UInt32 sceneIdx, DataInstanceHandle dataRefProvider, float newProvidedValue)
    {
        IScene& scene = *stagingScene[sceneIdx];
        scene.setDataSingleFloat(dataRefProvider, DataFieldHandle(0u), newProvidedValue);
    }

    std::pair<DataSlotId, DataSlotId> createTextureSlots(DataSlotHandle* providerDataSlotHandleOut = nullptr, UInt32 providerSceneIdx = 0u, UInt32 consumerSceneIdx = 1u)
    {
        IScene& scene1 = *stagingScene[providerSceneIdx];
        IScene& scene2 = *stagingScene[consumerSceneIdx];

        const TextureSamplerHandle sampler = scene2.allocateTextureSampler({ {}, RenderBufferHandle(999) });

        const DataSlotId providerId(getNextFreeDataSlotIdForDataLinking());
        const DataSlotId consumerId(getNextFreeDataSlotIdForDataLinking());
        DataSlotHandle providerDataSlot = scene1.allocateDataSlot({ EDataSlotType_TextureProvider, providerId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash(0x1234u, 0), TextureSamplerHandle() });
        if (nullptr != providerDataSlotHandleOut)
            *providerDataSlotHandleOut = providerDataSlot;
        scene2.allocateDataSlot({ EDataSlotType_TextureConsumer, consumerId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler });

        performFlush(providerSceneIdx);
        performFlush(consumerSceneIdx);
        update();
        expectEvents({ ERendererEventType_SceneDataSlotConsumerCreated, ERendererEventType_SceneDataSlotProviderCreated });

        return{ providerId, consumerId };
    }

    void createTextureSlotsAndLinkThem(DataSlotHandle* providerDataSlotHandleOut = nullptr, UInt32 providerSceneIdx = 0u, UInt32 consumerSceneIdx = 1u, bool expectSuccess = true)
    {
        const auto providerConsumer = createTextureSlots(providerDataSlotHandleOut, providerSceneIdx, consumerSceneIdx);
        linkProviderToConsumer(getSceneId(providerSceneIdx), providerConsumer.first, getSceneId(consumerSceneIdx), providerConsumer.second, expectSuccess);
    }

    DataSlotId createTextureConsumer(UInt32 sceneIndex)
    {
        IScene& scene = *stagingScene[sceneIndex];
        const TextureSamplerHandle sampler = scene.allocateTextureSampler({ {}, RenderBufferHandle(999) });
        const DataSlotId consumerId(getNextFreeDataSlotIdForDataLinking());
        scene.allocateDataSlot({ EDataSlotType_TextureConsumer, consumerId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler });

        performFlush(sceneIndex);
        update();
        expectEvent(ERendererEventType_SceneDataSlotConsumerCreated);

        return consumerId;
    }

    void createBufferLink(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId, bool expectFail = false)
    {
        rendererSceneUpdater->handleBufferToSceneDataLinkRequest(buffer, consumerSceneId, consumerId);
        expectEvent(expectFail ? ERendererEventType_SceneDataBufferLinkFailed : ERendererEventType_SceneDataBufferLinked);
    }

    void updateProviderTextureSlot(UInt32 sceneIdx, DataSlotHandle providerDataSlot, ResourceContentHash newProvidedValue)
    {
        IScene& scene = *stagingScene[sceneIdx];
        scene.setDataSlotTexture(providerDataSlot, newProvidedValue);
    }

    std::pair<DataSlotId, DataSlotId> createTransformationSlots(TransformHandle* providerTransformHandleOut = nullptr, UInt32 providerSceneIdx = 0u, UInt32 consumerSceneIdx = 1u)
    {
        IScene& scene1 = *stagingScene[providerSceneIdx];
        IScene& scene2 = *stagingScene[consumerSceneIdx];

        const auto nodeHandle1 = scene1.allocateNode();
        const auto nodeHandle2 = scene2.allocateNode();

        const auto providerTransformHandle = scene1.allocateTransform(nodeHandle1);
        scene2.allocateTransform(nodeHandle2);
        if (nullptr != providerTransformHandleOut)
            *providerTransformHandleOut = providerTransformHandle;

        const DataSlotId providerId(getNextFreeDataSlotIdForDataLinking());
        const DataSlotId consumerId(getNextFreeDataSlotIdForDataLinking());
        scene1.allocateDataSlot({ EDataSlotType_TransformationProvider, providerId, nodeHandle1, {}, ResourceContentHash::Invalid(), TextureSamplerHandle() });
        scene2.allocateDataSlot({ EDataSlotType_TransformationConsumer, consumerId, nodeHandle2, {}, ResourceContentHash::Invalid(), TextureSamplerHandle() });

        performFlush(providerSceneIdx);
        performFlush(consumerSceneIdx);
        update();
        expectEvents({ ERendererEventType_SceneDataSlotConsumerCreated, ERendererEventType_SceneDataSlotProviderCreated });

        return { providerId, consumerId };
    }

    void createTransformationSlotsAndLinkThem(TransformHandle* providerTransformHandleOut = nullptr, UInt32 providerSceneIdx = 0u, UInt32 consumerSceneIdx = 1u)
    {
        const auto providerConsumer = createTransformationSlots(providerTransformHandleOut, providerSceneIdx, consumerSceneIdx);
        linkProviderToConsumer(getSceneId(providerSceneIdx), providerConsumer.first, getSceneId(consumerSceneIdx), providerConsumer.second);
    }

    void linkProviderToConsumer(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId, bool expectSuccess = true)
    {
        rendererSceneUpdater->handleSceneDataLinkRequest(providerSceneId, providerId, consumerSceneId, consumerId);
        expectEvent(expectSuccess ? ERendererEventType_SceneDataLinked : ERendererEventType_SceneDataLinkFailed);
    }

    void unlinkConsumer(SceneId consumerSceneId, DataSlotId consumerId)
    {
        rendererSceneUpdater->handleDataUnlinkRequest(consumerSceneId, consumerId);
        expectEvent(ERendererEventType_SceneDataUnlinked);
    }

    void destroySceneUpdater()
    {
        expectContextEnable(DisplayHandle1, 2u);
        delete rendererSceneUpdater;
        rendererSceneUpdater = NULL;
    }

    void expectRenderableResourcesClean(UInt32 sceneIndex = 0u)
    {
        const RendererCachedScene& scene = rendererScenes.getScene(stagingScene[sceneIndex]->getSceneId());
        EXPECT_FALSE(scene.renderableResourcesDirty(renderableHandle));
    }

    void expectRenderableResourcesDirty(UInt32 sceneIndex = 0u)
    {
        const RendererCachedScene& scene = rendererScenes.getScene(stagingScene[sceneIndex]->getSceneId());
        EXPECT_TRUE(scene.renderableResourcesDirty(renderableHandle));
    }

    void expectEmbeddedCompositingManagerReturnsDeviceHandle(DeviceResourceHandle deviceResourceHandle, UInt32 times = 1u)
    {
        EXPECT_CALL(*renderer.getDisplayMock(DisplayHandle1).m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).Times(times).WillRepeatedly(Return(deviceResourceHandle));
    }

    void expectNoScenesModified()
    {
        EXPECT_EQ(0u, rendererSceneUpdater->getModifiedScenes().count());
    }

    void expectScenesModified(std::initializer_list<UInt32> indices = {0u})
    {
        const auto& modifiedScenes = rendererSceneUpdater->getModifiedScenes();
        ASSERT_EQ(indices.size(), modifiedScenes.count());
        for(auto idx : indices)
        {
            const SceneId sceneId = stagingScene[idx]->getSceneId();
            EXPECT_TRUE(modifiedScenes.hasElement(sceneId));
        }
    }

    AnimationHandle createRealTimeActiveAnimation(UInt32 sceneIndex = 0u)
    {
        const AnimationSystemHandle animationSystemHandle(10u);

        ActionCollectingScene& scene = *stagingScene[sceneIndex];
        IAnimationSystem* animationSystem = new ActionCollectingAnimationSystem(EAnimationSystemFlags_RealTime, scene.getSceneActionCollection(), AnimationSystemSizeInformation());
        scene.addAnimationSystem(animationSystem, animationSystemHandle);

        const NodeHandle nodeHandle1 = scene.allocateNode();
        const TransformHandle transHandle1 = scene.allocateTransform(nodeHandle1);

        const SplineHandle splineHandle1 = animationSystem->allocateSpline(ESplineKeyType_Basic, EDataTypeID_Vector3f);
        animationSystem->setSplineKeyBasicVector3f(splineHandle1, 0u, Vector3(111.f, -999.f, 66.f));
        animationSystem->setSplineKeyBasicVector3f(splineHandle1, 100000u, Vector3(11.f, -99.f, 666.f));

        typedef DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType ContainerTraitsClass;
        const DataBindHandle dataBindHandle1 = animationSystem->allocateDataBinding(scene, ContainerTraitsClass::TransformNode_Rotation, transHandle1.asMemoryHandle(), InvalidMemoryHandle);

        const AnimationInstanceHandle animInstHandle1 = animationSystem->allocateAnimationInstance(splineHandle1, EInterpolationType_Linear, EVectorComponent_All);
        animationSystem->addDataBindingToAnimationInstance(animInstHandle1, dataBindHandle1);

        const auto animationHandle = animationSystem->allocateAnimation(animInstHandle1);

        const UInt64 systemTime = PlatformTime::GetMillisecondsAbsolute();
        animationSystem->setAnimationStartTime(animationHandle, systemTime);
        animationSystem->setAnimationStopTime(animationHandle, systemTime + 100000u);

        performFlush();
        update();

        PlatformThread::Sleep(1u);
        update();
        EXPECT_TRUE(rendererScenes.getScene(scene.getSceneId()).getAnimationSystem(animationSystemHandle)->hasActiveAnimations());

        return animationHandle;
    }

    void stopAnimation(AnimationHandle animationHandle, UInt32 sceneIndex = 0)
    {
        const AnimationSystemHandle animationSystemHandle(10u);

        IScene& scene = *stagingScene[sceneIndex];
        IAnimationSystem& animationSystem = *scene.getAnimationSystem(animationSystemHandle);

        const UInt64 systemTime = PlatformTime::GetMillisecondsAbsolute();
        animationSystem.setAnimationStopTime(animationHandle, systemTime);
        performFlush();
        PlatformThread::Sleep(1u);
        update();

        EXPECT_FALSE(rendererScenes.getScene(scene.getSceneId()).getAnimationSystem(animationSystemHandle)->hasActiveAnimations());
    }

    DataSlotId getNextFreeDataSlotIdForDataLinking()
    {
        return DataSlotId(dataSlotIdForDataLinking.getReference()++);
    }

    OffscreenBufferHandle showAndInitiateInterruptedRendering(UInt32 sceneIdx, UInt32 interruptedSceneIdx)
    {
        // assign scene to double buffered OB
        const OffscreenBufferHandle buffer(111u);
        expectContextEnable();
        expectRenderTargetUploaded(DisplayHandle1, true, DeviceMock::FakeRenderTargetDeviceHandle, true);
        EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, true));
        EXPECT_TRUE(assignSceneToOffscreenBuffer(interruptedSceneIdx, buffer));

        showScene(sceneIdx);
        showScene(interruptedSceneIdx);
        update();

        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(DisplayHandle1);
        EXPECT_CALL(*displayMock.m_displayController, getRenderBackend()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, getDisplayBuffer()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, clearBuffer(_, _)).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, getEmbeddedCompositingManager()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_embeddedCompositingManager, notifyClients()).Times(AnyNumber());

        EXPECT_CALL(platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
        EXPECT_CALL(*displayMock.m_displayController, handleWindowEvents());
        EXPECT_CALL(*displayMock.m_displayController, canRenderNewFrame()).WillOnce(Return(true));
        EXPECT_CALL(*displayMock.m_displayController, enableContext());
        EXPECT_CALL(*displayMock.m_displayController, executePostProcessing());
        EXPECT_CALL(*displayMock.m_displayController, swapBuffers());

        EXPECT_CALL(*displayMock.m_displayController, renderScene(Ref(rendererScenes.getScene(getSceneId(sceneIdx))), DisplayControllerMock::FakeFrameBufferHandle, _, _, _));
        SceneRenderExecutionIterator interruptedState;
        interruptedState.incrementRenderableIdx();
        EXPECT_CALL(*displayMock.m_displayController, renderScene(Ref(rendererScenes.getScene(getSceneId(interruptedSceneIdx))), DeviceMock::FakeRenderTargetDeviceHandle, _, _, _)).WillOnce(Return(interruptedState));

        renderer.doOneRenderLoop();
        EXPECT_TRUE(renderer.hasAnyBufferWithInterruptedRendering());

        return buffer;
    }

    void doRenderLoop()
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(DisplayHandle1);
        EXPECT_CALL(*displayMock.m_displayController, getRenderBackend()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, getDisplayBuffer()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, clearBuffer(_, _)).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, getEmbeddedCompositingManager()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_embeddedCompositingManager, notifyClients()).Times(AnyNumber());

        EXPECT_CALL(platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
        EXPECT_CALL(*displayMock.m_displayController, handleWindowEvents());
        EXPECT_CALL(*displayMock.m_displayController, canRenderNewFrame()).WillOnce(Return(true));
        EXPECT_CALL(*displayMock.m_displayController, enableContext()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, executePostProcessing()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, swapBuffers()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, renderScene(_, _, _, _, _)).Times(AnyNumber());

        renderer.doOneRenderLoop();
    }

    static const DisplayHandle DisplayHandle1;
    static const DisplayHandle DisplayHandle2;

    static const ResourceContentHash InvalidResource1;
    static const ResourceContentHash InvalidResource2;

    const RenderableHandle renderableHandle;
    const DataInstanceHandle uniformDataInstanceHandle;
    const DataInstanceHandle geometryDataInstanceHandle;

    const TextureSamplerHandle samplerHandle;
    const StreamTextureHandle streamTextureHandle;

    std::vector<std::unique_ptr<ActionCollectingScene>> stagingScene;
    DataSlotId dataSlotIdForDataLinking{9911u};

    static constexpr UInt ForceApplyFlushesLimit = 10u;
    static constexpr UInt ForceUnsubscribeFlushLimit = 20u;

    NiceMock<RendererResourceCacheMock> rendererResourceCacheMock;
    StrictMock<PlatformFactoryStrictMock> platformFactoryMock;
    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    SceneExpirationMonitor expirationMonitor;
    RendererStatistics rendererStatistics;
    StrictMock<RendererMockWithStrictMockDisplay> renderer;
    StrictMock<SceneGraphConsumerComponentMock> sceneGraphConsumerComponent;
    StrictMock<ResourceProviderMock> resourceProvider1;
    StrictMock<ResourceProviderMock> resourceProvider2;
    FrameTimer frameTimer;
    ResourceUploader resourceUploader;
    SceneStateExecutor sceneStateExecutor;
    RendererSceneUpdater* rendererSceneUpdater;
};
}
#endif
