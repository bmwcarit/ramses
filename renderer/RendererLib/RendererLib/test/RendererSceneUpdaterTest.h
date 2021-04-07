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
#include "SceneUtils/ResourceUtils.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DisplayEventHandlerManager.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "Animation/AnimationSystem.h"
#include "Animation/ActionCollectingAnimationSystem.h"
#include "Scene/SceneDataBinding.h"
#include "ComponentMocks.h"
#include "ResourceDeviceHandleAccessorMock.h"
#include "RendererResourceCacheMock.h"
#include "MockResourceHash.h"
#include "SceneReferenceLogicMock.h"
#include "SceneAllocateHelper.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Collections/Pair.h"
#include "RendererSceneEventSenderMock.h"
#include "RendererSceneUpdaterFacade.h"
#include <unordered_set>
#include <memory>
#include "Components/SceneUpdate.h"
#include "Watchdog/ThreadAliveNotifierMock.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal {

class ARendererSceneUpdater : public ::testing::Test
{
public:
    ARendererSceneUpdater()
        : rendererScenes(rendererEventCollector)
        , expirationMonitor(rendererScenes, rendererEventCollector)
        , renderer(platformFactoryMock, rendererScenes, rendererEventCollector, expirationMonitor, rendererStatistics)
        , sceneStateExecutor(renderer, sceneEventSender, rendererEventCollector)
        , rendererSceneUpdater(new RendererSceneUpdaterFacade(platformFactoryMock, renderer, rendererScenes, sceneStateExecutor, rendererEventCollector, frameTimer, expirationMonitor, notifier, &rendererResourceCacheMock))
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);

        rendererSceneUpdater->setSceneReferenceLogicHandler(sceneReferenceLogic);
        frameTimer.startFrame();
        rendererSceneUpdater->setLimitFlushesForceApply(ForceApplyFlushesLimit);
        rendererSceneUpdater->setLimitFlushesForceUnsubscribe(ForceUnsubscribeFlushLimit);

        EXPECT_CALL(renderer, setClearColor(_, _, _)).Times(0);
        // called explicitly from tests, no sense in tracking
        EXPECT_CALL(*rendererSceneUpdater, handleSceneUpdate(_, _)).Times(AnyNumber());
        // resource cache exists but reports it has all resources already - only specific tests cases test further
        EXPECT_CALL(rendererResourceCacheMock, hasResource(_, _)).Times(AnyNumber()).WillRepeatedly(Return(true));
    }

    virtual void TearDown() override
    {
        expectNoEvent();
    }

protected:
    void update()
    {
        renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
        EXPECT_CALL(sceneReferenceLogic, update());
        for (auto& resMgr : rendererSceneUpdater->m_resourceManagerMocks)
            EXPECT_CALL(*resMgr.second, hasResourcesToBeUploaded());
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
        rendererSceneUpdater->handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote);
        EXPECT_TRUE(sceneStateExecutor.getSceneState(sceneId) == ESceneState::Published);
        expectInternalSceneStateEvent(ERendererEventType::ScenePublished);
    }

    void requestSceneSubscription(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = getSceneId(sceneIndex);
        EXPECT_CALL(sceneEventSender, sendSubscribeScene(sceneId));
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
            expectInternalSceneStateEvent(ERendererEventType::SceneSubscribed);
        }

        return sceneIndex;
    }

    void expectEventsEqual(const std::initializer_list<ERendererEventType> expectedEvents, const RendererEventVector& actualEvents)
    {
        ASSERT_EQ(expectedEvents.size(), actualEvents.size());
        auto eventIt = actualEvents.cbegin();
        for (auto expectedEvent : expectedEvents)
        {
            EXPECT_EQ(expectedEvent, eventIt->eventType);
            ++eventIt;
        }
    }

    void expectEvents(const std::initializer_list<ERendererEventType> expectedEvents)
    {
        RendererEventVector events;
        RendererEventVector dummy;
        rendererEventCollector.appendAndConsumePendingEvents(events, dummy);
        expectEventsEqual(expectedEvents, events);
        EXPECT_TRUE(dummy.empty()) << " expected renderer events only but there are also unchecked scene events";
    }

    void expectSceneEvents(const std::initializer_list<ERendererEventType> expectedEvents)
    {
        RendererEventVector events;
        RendererEventVector dummy;
        rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
        expectEventsEqual(expectedEvents, events);
        EXPECT_TRUE(dummy.empty()) << " expected scene events only but there are also unchecked renderer events";
    }

    void expectInternalSceneStateEvents(const std::initializer_list<ERendererEventType> expectedEvents)
    {
        InternalSceneStateEvents events;
        rendererEventCollector.dispatchInternalSceneStateEvents(events);
        ASSERT_EQ(expectedEvents.size(), events.size()) << "internal scene state events";
        auto expectedIt = expectedEvents.begin();
        for (size_t i = 0; i < events.size(); ++i)
            EXPECT_EQ(*expectedIt++, events[i].type) << "internal scene state event #" << i;
    }

    void expectNoEvent()
    {
        RendererEventVector events;
        RendererEventVector sceneEvents;
        rendererEventCollector.appendAndConsumePendingEvents(events, sceneEvents);
        EXPECT_TRUE(events.empty()) << " pending unchecked renderer events";
        EXPECT_TRUE(sceneEvents.empty()) << " pending unchecked scene events";

        expectInternalSceneStateEvents({});
    }

    void expectEvent(ERendererEventType eventType)
    {
        expectEvents({ eventType });
    }

    void expectSceneEvent(ERendererEventType eventType)
    {
        expectSceneEvents({ eventType });
    }

    void expectInternalSceneStateEvent(ERendererEventType eventType)
    {
        expectInternalSceneStateEvents({ eventType });
    }

    void requestMapScene(UInt32 sceneIndex = 0u, DisplayHandle displayHandle = DisplayHandle1)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneMappingRequest(sceneId, displayHandle);
        expectNoEvent();
    }

    void mapScene(UInt32 sceneIndex = 0u, DisplayHandle displayHandle = DisplayHandle1)
    {
        requestMapScene(sceneIndex, displayHandle);
        update(); // will set from map requested to being mapped and uploaded (if all pending flushes applied)
        update(); // will set to mapped (if all resources uploaded)
        expectInternalSceneStateEvent(ERendererEventType::SceneMapped);
    }

    bool assignSceneToDisplayBuffer(UInt32 sceneIndex, OffscreenBufferHandle buffer = {}, Int32 renderOrder = 0)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        const bool result = rendererSceneUpdater->handleSceneDisplayBufferAssignmentRequest(sceneId, buffer, renderOrder);
        EXPECT_EQ(renderOrder, renderer.getSceneGlobalOrder(sceneId));
        return result;
    }

    void showScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneShowRequest(sceneId);
        update();
        expectInternalSceneStateEvent(ERendererEventType::SceneShown);
    }

    void hideScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneHideRequest(sceneId);
        expectInternalSceneStateEvent(ERendererEventType::SceneHidden);
    }

    void unmapScene(UInt32 sceneIndex = 0u, DisplayHandle display = DisplayHandle1, bool expectResourcesUnload = true)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        if (expectResourcesUnload)
            expectUnloadOfSceneResources(sceneIndex, display);

        rendererSceneUpdater->handleSceneUnmappingRequest(sceneId);
        expectInternalSceneStateEvent(ERendererEventType::SceneUnmapped);
        expectNoResourceReferencedByScene(sceneIndex, display);
    }

    void expectNoResourceReferencedByScene(UInt32 sceneIndex = 0u, DisplayHandle display = DisplayHandle1)
    {
        rendererSceneUpdater->m_resourceManagerMocks[display]->expectNoResourceReferencesForScene(getSceneId(sceneIndex));
    }

    int getResourceRefCount(ResourceContentHash resource, DisplayHandle display = DisplayHandle1)
    {
        return rendererSceneUpdater->m_resourceManagerMocks[display]->getResourceRefCount(resource);
    }

    void expectUnloadOfSceneResources(UInt32 sceneIndex = 0u, DisplayHandle display = DisplayHandle1)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], unloadAllSceneResourcesForScene(sceneId));
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], unreferenceAllResourcesForScene(sceneId));
    }

    void unpublishMapRequestedScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(sceneId));
        rendererSceneUpdater->handleSceneUnpublished(sceneId);
        expectInternalSceneStateEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_FALSE(renderer.getDisplaySceneIsAssignedTo(sceneId).isValid());
    }

    void unpublishMappedScene(UInt32 sceneIndex = 0u, DisplayHandle display = DisplayHandle1)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        expectUnloadOfSceneResources(sceneIndex, display);
        EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(sceneId));
        rendererSceneUpdater->handleSceneUnpublished(sceneId);
        expectInternalSceneStateEvents({ ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });
        EXPECT_FALSE(renderer.getDisplaySceneIsAssignedTo(sceneId).isValid());
    }

    void unpublishShownScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(sceneId));
        rendererSceneUpdater->handleSceneUnpublished(sceneId);
        expectInternalSceneStateEvents({ ERendererEventType::SceneHiddenIndirect, ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect, ERendererEventType::SceneUnpublished });

        EXPECT_FALSE(renderer.getDisplaySceneIsAssignedTo(sceneId).isValid());
    }

    void unsubscribeMapRequestedScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneUnsubscriptionRequest(sceneId, true);

        expectInternalSceneStateEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect });

        EXPECT_FALSE(renderer.getDisplaySceneIsAssignedTo(sceneId).isValid());
    }

    void unsubscribeMappedScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneUnsubscriptionRequest(sceneId, true);

        expectInternalSceneStateEvents({ ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect });

        EXPECT_FALSE(renderer.getDisplaySceneIsAssignedTo(sceneId).isValid());
    }

    void unsubscribeShownScene(UInt32 sceneIndex = 0u)
    {
        const SceneId sceneId = stagingScene[sceneIndex]->getSceneId();
        rendererSceneUpdater->handleSceneUnsubscriptionRequest(sceneId, true);

        expectInternalSceneStateEvents({ ERendererEventType::SceneHiddenIndirect, ERendererEventType::SceneUnmappedIndirect, ERendererEventType::SceneUnsubscribedIndirect });
        EXPECT_FALSE(renderer.getDisplaySceneIsAssignedTo(sceneId).isValid());
    }

    NodeHandle performFlushWithCreateNodeAction(UInt32 sceneIndex = 0u, size_t numNodes = 1u)
    {
        NodeHandle nodeHandle;
        IScene& scene = *stagingScene[sceneIndex];
        SceneAllocateHelper sceneAllocator(scene);
        for (size_t i = 0; i < numNodes; ++i)
            nodeHandle = sceneAllocator.allocateNode();
        performFlush(sceneIndex);

        return nodeHandle;
    }

    void expectReadPixelsEvents(const std::initializer_list<std::tuple<DisplayHandle, OffscreenBufferHandle, bool /*success*/>>& expectedEvents)
    {
        RendererEventVector events;
        RendererEventVector dummy;
        rendererEventCollector.appendAndConsumePendingEvents(events, dummy);
        ASSERT_EQ(events.size(), expectedEvents.size());

        auto eventsIt = events.cbegin();
        for (const auto& expectedEvent : expectedEvents)
        {
            const auto& event = *eventsIt;
            EXPECT_EQ(event.displayHandle, std::get<0>(expectedEvent));
            EXPECT_EQ(event.offscreenBuffer, std::get<1>(expectedEvent));
            if (std::get<2>(expectedEvent))
                EXPECT_EQ(event.eventType, ERendererEventType::ReadPixelsFromFramebuffer);
            else
                EXPECT_EQ(event.eventType, ERendererEventType::ReadPixelsFromFramebufferFailed);
        }

    }

    void performFlush(UInt32 sceneIndex = 0u, SceneVersionTag version = SceneVersionTag::Invalid(), const SceneSizeInformation* sizeInfo = nullptr, const FlushTimeInformation& timeInfo = {}, const SceneReferenceActionVector& sceneRefActions = {})
    {
        ActionCollectingScene& scene = *stagingScene[sceneIndex];
        const SceneSizeInformation newSizeInfo = (sizeInfo ? *sizeInfo : scene.getSceneSizeInformation());
        const SceneSizeInformation currSizeInfo = rendererScenes.hasScene(scene.getSceneId()) ? rendererScenes.getScene(scene.getSceneId()).getSceneSizeInformation() : SceneSizeInformation();

        SceneUpdate update;
        update.actions = (std::move(scene.getSceneActionCollection()));

        SceneActionCollectionCreator creator(update.actions);

        ResourceContentHashVector resources;
        ResourceChanges resourceChanges;
        ResourceUtils::GetAllResourcesFromScene(resources, scene);
        ResourceUtils::DiffResources(previousResources[sceneIndex], resources, resourceChanges);
        previousResources[sceneIndex].swap(resources);
        resourceChanges.m_sceneResourceActions = scene.getSceneResourceActions();

        for (const auto& resHash : resourceChanges.m_resourcesAdded)
            update.resources.push_back(MockResourceHash::GetManagedResource(resHash));

        update.flushInfos = {1u, version, newSizeInfo, resourceChanges, sceneRefActions, timeInfo, newSizeInfo>currSizeInfo, true};
        scene.resetResourceChanges();
        rendererSceneUpdater->handleSceneUpdate(stagingScene[sceneIndex]->getSceneId(), std::move(update));
    }

    void performFlushWithExpiration(UInt32 sceneIndex, UInt32 expirationTS)
    {
        const FlushTimeInformation timeInfo{ FlushTime::Clock::time_point(std::chrono::milliseconds(expirationTS)), {}, FlushTime::Clock::getClockType() };
        performFlush(sceneIndex, {}, nullptr, timeInfo);
    }

    bool lastFlushWasAppliedOnRendererScene(UInt32 sceneIndex = 0u)
    {
        return !rendererSceneUpdater->hasPendingFlushes(stagingScene[sceneIndex]->getSceneId());
    }

    void createDisplayAndExpectSuccess(DisplayHandle displayHandle = DisplayHandle1, const DisplayConfig& displayConfig = DisplayConfig())
    {
        EXPECT_CALL(*rendererSceneUpdater, createResourceManager(_, _, _, displayHandle, _, _, _, _));
        EXPECT_CALL(platformFactoryMock, createResourceUploadRenderBackend(_));
        rendererSceneUpdater->createDisplayContext(displayConfig, displayHandle, nullptr);
        EXPECT_TRUE(renderer.hasDisplayController(displayHandle));
        expectEvent(ERendererEventType::DisplayCreated);

        // no offscreen buffers reported uploaded by default
        ON_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(_)).WillByDefault(Return(DeviceResourceHandle::Invalid()));
        ON_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferColorBufferDeviceHandle(_)).WillByDefault(Return(DeviceResourceHandle::Invalid()));
        // scene updater queries display/offscreen buffer from device handle when updating modified scenes states - these are invalid for display framebuffer
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferHandle(DisplayControllerMock::FakeFrameBufferHandle)).Times(AnyNumber()).WillRepeatedly(Return(OffscreenBufferHandle::Invalid()));

        // scene updater logic might call resource manager for ref/unref of resources with empty list - no need to track those
        ResourceContentHashVector emptyResources;
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], referenceResourcesForScene(_, emptyResources)).Times(AnyNumber());
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], unreferenceResourcesForScene(_, emptyResources)).Times(AnyNumber());

        // querying of resources state happens often, concrete tests can control status reported
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getResourceStatus(_)).Times(AnyNumber());

        // by default skip path triggering upload/unload, concrete tests can override
        ON_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], hasResourcesToBeUploaded()).WillByDefault(Return(false));
    }

    void destroyDisplay(DisplayHandle displayHandle = DisplayHandle1, bool expectFail = false)
    {
        if (!expectFail)
        {
            EXPECT_CALL(platformFactoryMock, destroyResourceUploadRenderBackend(_));
            EXPECT_CALL(*rendererSceneUpdater, destroyResourceManager(displayHandle));
            rendererSceneUpdater->m_resourceManagerMocks[displayHandle]->expectNoResourceReferences();
        }
        rendererSceneUpdater->destroyDisplayContext(displayHandle);
        if (expectFail)
        {
            expectEvent(ERendererEventType::DisplayDestroyFailed);
        }
        else
        {
            EXPECT_FALSE(renderer.hasDisplayController(displayHandle));
            expectEvent(ERendererEventType::DisplayDestroyed);
        }
    }

    void createRenderable(UInt32 sceneIndex = 0u, bool withVertexArray = false, bool withTextureSampler = false)
    {
        createRenderableNoFlush(sceneIndex, withVertexArray, withTextureSampler);
        performFlush(sceneIndex);
    }

    void createRenderableNoFlush(UInt32 sceneIndex = 0u, bool withVertexArray = false, bool withTextureSampler = false)
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
        scene.allocateDataLayout({ DataFieldInfo{EDataType::Vector2I}, DataFieldInfo{EDataType::Vector2I} }, MockResourceHash::EffectHash, camDataLayoutHandle);
        scene.allocateCamera(ECameraProjectionType::Perspective, scene.allocateNode(), scene.allocateDataInstance(camDataLayoutHandle, camDataHandle), cameraHandle);

        scene.addRenderableToRenderGroup(renderGroupHandle, renderableHandle, 0u);
        scene.addRenderGroupToRenderPass(renderPassHandle, renderGroupHandle, 0u);
        scene.setRenderPassCamera(renderPassHandle, cameraHandle);
        scene.setRenderPassEnabled(renderPassHandle, true);

        DataFieldInfoVector uniformDataFields;
        if (withTextureSampler)
        {
            uniformDataFields.push_back(DataFieldInfo{ EDataType::TextureSampler2D });
        }
        scene.allocateDataLayout(uniformDataFields, MockResourceHash::EffectHash, uniformDataLayoutHandle);
        scene.allocateDataInstance(uniformDataLayoutHandle, uniformDataInstanceHandle);
        scene.setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, uniformDataInstanceHandle);

        DataFieldInfoVector geometryDataFields;
        geometryDataFields.push_back(DataFieldInfo{ EDataType::Indices, 1u, EFixedSemantics::Indices });
        if (withVertexArray)
        {
            geometryDataFields.push_back(DataFieldInfo{ EDataType::Vector3Buffer, 1u, EFixedSemantics::Invalid });
        }
        scene.allocateDataLayout(geometryDataFields, MockResourceHash::EffectHash, geometryDataLayoutHandle);
        scene.allocateDataInstance(geometryDataLayoutHandle, geometryDataInstanceHandle);
        scene.setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Geometry, geometryDataInstanceHandle);
    }

    void destroyRenderable(UInt32 sceneIndex = 0u)
    {
        const RenderGroupHandle renderGroupHandle(3u);
        IScene& scene = *stagingScene[sceneIndex];
        scene.removeRenderableFromRenderGroup(renderGroupHandle, renderableHandle);
        scene.releaseRenderable(renderableHandle);

        performFlush(sceneIndex);
    }

    void setRenderableResources(UInt32 sceneIndex = 0u, ResourceContentHash indexArrayHash = MockResourceHash::IndexArrayHash)
    {
        setRenderableResourcesNoFlush(sceneIndex, indexArrayHash);
        performFlush(sceneIndex);
    }

    void setRenderableResourcesNoFlush(UInt32 sceneIndex = 0u, ResourceContentHash indexArrayHash = MockResourceHash::IndexArrayHash)
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.setDataResource(geometryDataInstanceHandle, DataFieldHandle(0u), indexArrayHash, DataBufferHandle::Invalid(), 0u, 0u, 0u);
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
        scene.allocateStreamTexture(WaylandIviSurfaceId{ 1u }, ResourceContentHash::Invalid(), streamTextureHandle);
        scene.allocateTextureSampler({ {}, streamTextureHandle }, samplerHandle);
    }

    void createRenderableAndResourcesWithStreamTexture(UInt32 sceneIndex = 0u, DisplayHandle displayHandle = DisplayHandle1)
    {
        expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, sceneIndex, displayHandle);
        createRenderable(sceneIndex, false, true);
        setRenderableResources(sceneIndex);

        createSamplerWithStreamTexture(sceneIndex);
        setRenderableStreamTexture(sceneIndex);

        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], uploadStreamTexture(streamTextureHandle, _, getSceneId(sceneIndex)));
    }

    void removeRenderableResources(UInt32 sceneIndex = 0u)
    {
        IScene& scene = *stagingScene[sceneIndex];
        // it is not allowed to set both resource and data buffer as invalid, so for the purpose of the test cases here we set to non-existent data buffer with handle 0
        scene.setDataResource(geometryDataInstanceHandle, DataFieldHandle(0u), ResourceContentHash::Invalid(), DataBufferHandle(0u), 0u, 0u, 0u);

        performFlush(sceneIndex);
    }

    void expectResourcesReferenced(const ResourceContentHashVector& resources, UInt32 sceneIdx = 0u, DisplayHandle display = DisplayHandle1, int times = 1)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], referenceResourcesForScene(getSceneId(sceneIdx), resources)).Times(times);
    }

    void expectResourcesProvided(DisplayHandle display = DisplayHandle1, int times = 1)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], provideResourceData(_)).Times(times);
    }

    void expectResourcesReferencedAndProvided(const ResourceContentHashVector& resources, UInt32 sceneIdx = 0u, DisplayHandle display = DisplayHandle1)
    {
        // most tests add 1 resource in 1 flush, referencing logic is executed per flush
        for (const auto& res : resources)
            expectResourcesReferenced({ res }, sceneIdx, display, 1);
        expectResourcesProvided(display, int(resources.size()));
    }

    void expectResourcesReferencedAndProvided_altogether(const ResourceContentHashVector& resources, UInt32 sceneIdx = 0u, DisplayHandle display = DisplayHandle1)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], referenceResourcesForScene(getSceneId(sceneIdx), resources));
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], provideResourceData(_)).Times(int(resources.size()));
    }

    void expectResourcesUnreferenced(const ResourceContentHashVector& resources, UInt32 sceneIdx = 0u, DisplayHandle display = DisplayHandle1, int times = 1)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], unreferenceResourcesForScene(getSceneId(sceneIdx), resources)).Times(times);
    }

    void reportResourceAs(ResourceContentHash resource, EResourceStatus status, DisplayHandle display = DisplayHandle1)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], getResourceStatus(resource)).Times(AnyNumber()).WillRepeatedly(Return(status));
    }

    void expectOffscreenBufferUploaded(OffscreenBufferHandle buffer, DisplayHandle displayHandle = DisplayHandle1, DeviceResourceHandle rtDeviceHandleToReturn = DeviceMock::FakeRenderTargetDeviceHandle, bool doubleBuffered = false, ERenderBufferType depthStencilBufferType = ERenderBufferType_DepthStencilBuffer)
    {
        {
            InSequence seq;
            EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(DeviceResourceHandle::Invalid()));
            EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], uploadOffscreenBuffer(buffer, _, _, _, doubleBuffered, depthStencilBufferType));
            EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer)).WillRepeatedly(Return(rtDeviceHandleToReturn));
        }
        // scene updater queries offscreen buffer from device handle when updating modified scenes states
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferHandle(rtDeviceHandleToReturn)).Times(AnyNumber()).WillRepeatedly(Return(buffer));
    }

    void expectOffscreenBufferDeleted(OffscreenBufferHandle buffer, DisplayHandle displayHandle = DisplayHandle1, DeviceResourceHandle deviceHandle = DeviceMock::FakeRenderTargetDeviceHandle)
    {
        InSequence seq;
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer)).WillOnce(Return(deviceHandle));
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], unloadOffscreenBuffer(buffer));
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getOffscreenBufferDeviceHandle(buffer)).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    }

    void expectStreamBufferUploaded(StreamBufferHandle buffer, WaylandIviSurfaceId source, DisplayHandle displayHandle = DisplayHandle1, DeviceResourceHandle rtDeviceHandleToReturn = DeviceMock::FakeRenderTargetDeviceHandle)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], uploadStreamBuffer(buffer, source));
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getStreamBufferDeviceHandle(buffer)).Times(AnyNumber()).WillRepeatedly(Return(rtDeviceHandleToReturn));
    }

    void expectStreamBufferDeleted(StreamBufferHandle buffer, DisplayHandle displayHandle = DisplayHandle1)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], unloadStreamBuffer(buffer));
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[displayHandle], getStreamBufferDeviceHandle(buffer)).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    }

    void expectBlitPassUploaded()
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[DisplayHandle1], uploadRenderTargetBuffer(_, getSceneId(0u), _)).Times(2);
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[DisplayHandle1], uploadBlitPassRenderTargets(_, _, _, getSceneId(0u)));
    }

    void expectBlitPassDeleted()
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[DisplayHandle1], unloadRenderTargetBuffer(_, getSceneId(0u))).Times(2);
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[DisplayHandle1], unloadBlitPassRenderTargets(_, getSceneId(0u)));
    }

    void expectStreamTextureUploaded(DisplayHandle displayHandle = DisplayHandle1)
    {
        static constexpr DeviceResourceHandle FakeStreamTextureDeviceHandle{ 987 };
        EXPECT_CALL(*renderer.getDisplayMock(displayHandle).m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillRepeatedly(Return(FakeStreamTextureDeviceHandle));
    }

    void createRenderTargetWithBuffers(UInt32 sceneIndex = 0u, RenderTargetHandle renderTargetHandle = RenderTargetHandle{ 0u }, RenderBufferHandle bufferHandle = RenderBufferHandle{ 0u }, RenderBufferHandle depthHandle = RenderBufferHandle{ 1u })
    {
        IScene& scene = *stagingScene[sceneIndex];
        scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u }, bufferHandle);
        scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_ReadWrite, 0u }, depthHandle);
        scene.allocateRenderTarget(renderTargetHandle);
        scene.addRenderTargetRenderBuffer(renderTargetHandle, bufferHandle);
        scene.addRenderTargetRenderBuffer(renderTargetHandle, depthHandle);
        performFlush(sceneIndex);
    }

    void expectRenderTargetUploaded(DisplayHandle display = DisplayHandle1, UInt32 sceneIdx = 0u)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], uploadRenderTargetBuffer(_, getSceneId(sceneIdx), _)).Times(2);
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], uploadRenderTarget(_, _, getSceneId(sceneIdx)));
    }

    void expectRenderTargetUnloaded(DisplayHandle display = DisplayHandle1, UInt32 sceneIdx = 0u)
    {
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], unloadRenderTargetBuffer(_, getSceneId(sceneIdx))).Times(2);
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMocks[display], unloadRenderTarget(_, getSceneId(sceneIdx)));
    }

    void createBlitPass()
    {
        const BlitPassHandle blitPassHandle(0u);
        IScene& scene = *stagingScene[0];
        const RenderBufferHandle sourceBufferHandle = scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });
        const RenderBufferHandle destinationBufferHandle = scene.allocateRenderBuffer({ 16u, 16u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u });
        scene.allocateBlitPass(sourceBufferHandle, destinationBufferHandle, blitPassHandle);
        performFlush();
    }

    void destroyRenderTargetWithBuffers(UInt32 sceneIndex = 0u)
    {
        const RenderTargetHandle renderTargetHandle(0u);
        const RenderBufferHandle bufferHandle(0u);
        const RenderBufferHandle depthHandle(1u);
        IScene& scene = *stagingScene[sceneIndex];
        scene.releaseRenderTarget(renderTargetHandle);
        scene.releaseRenderBuffer(bufferHandle);
        scene.releaseRenderBuffer(depthHandle);
        performFlush(sceneIndex);
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

        const DataLayoutHandle dataLayout1 = scene1.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
        const DataLayoutHandle dataLayout2 = scene2.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
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
        expectSceneEvents({ ERendererEventType::SceneDataSlotConsumerCreated, ERendererEventType::SceneDataSlotProviderCreated });

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
        DataSlotHandle providerDataSlot = scene1.allocateDataSlot({ EDataSlotType_TextureProvider, providerId, NodeHandle(), DataInstanceHandle::Invalid(), MockResourceHash::TextureHash, TextureSamplerHandle() });
        if (nullptr != providerDataSlotHandleOut)
            *providerDataSlotHandleOut = providerDataSlot;
        scene2.allocateDataSlot({ EDataSlotType_TextureConsumer, consumerId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler });

        performFlush(providerSceneIdx);
        performFlush(consumerSceneIdx);
        update();
        expectSceneEvents({ ERendererEventType::SceneDataSlotConsumerCreated, ERendererEventType::SceneDataSlotProviderCreated });

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
        expectSceneEvent(ERendererEventType::SceneDataSlotConsumerCreated);

        return consumerId;
    }

    template <typename BUFFERHANDLE>
    void createBufferLink(BUFFERHANDLE buffer, SceneId consumerSceneId, DataSlotId consumerId, bool expectFail = false)
    {
        rendererSceneUpdater->handleBufferToSceneDataLinkRequest(buffer, consumerSceneId, consumerId);
        expectSceneEvent(expectFail ? ERendererEventType::SceneDataBufferLinkFailed : ERendererEventType::SceneDataBufferLinked);
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
        expectSceneEvents({ ERendererEventType::SceneDataSlotConsumerCreated, ERendererEventType::SceneDataSlotProviderCreated });

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
        expectSceneEvent(expectSuccess ? ERendererEventType::SceneDataLinked : ERendererEventType::SceneDataLinkFailed);
    }

    void unlinkConsumer(SceneId consumerSceneId, DataSlotId consumerId)
    {
        rendererSceneUpdater->handleDataUnlinkRequest(consumerSceneId, consumerId);
        expectSceneEvent(ERendererEventType::SceneDataUnlinked);
    }

    void destroySceneUpdater()
    {
        rendererSceneUpdater.reset();
        expectEvent(ERendererEventType::DisplayDestroyed);
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

    void expectModifiedScenesReportedToRenderer(std::initializer_list<UInt32> indices = {0u})
    {
        for (auto idx : indices)
        {
            const SceneId sceneId = stagingScene[idx]->getSceneId();
            EXPECT_CALL(renderer, markBufferWithSceneAsModified(sceneId));
        }
    }

    void expectNoModifiedScenesReportedToRenderer()
    {
        EXPECT_CALL(renderer, markBufferWithSceneAsModified(_)).Times(0u);
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

        using ContainerTraitsClass = DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType;
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
        expectOffscreenBufferUploaded(buffer, DisplayHandle1, DeviceMock::FakeRenderTargetDeviceHandle, true);
        EXPECT_TRUE(rendererSceneUpdater->handleBufferCreateRequest(buffer, DisplayHandle1, 1u, 1u, 0u, true, ERenderBufferType_DepthStencilBuffer));
        EXPECT_TRUE(assignSceneToDisplayBuffer(interruptedSceneIdx, buffer));

        showScene(sceneIdx);
        showScene(interruptedSceneIdx);
        update();

        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(DisplayHandle1);
        EXPECT_CALL(*displayMock.m_displayController, getRenderBackend()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, getDisplayBuffer()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, clearBuffer(_, _, _)).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, getEmbeddedCompositingManager()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_embeddedCompositingManager, notifyClients()).Times(AnyNumber());

        EXPECT_CALL(platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
        EXPECT_CALL(*displayMock.m_displayController, handleWindowEvents());
        EXPECT_CALL(*displayMock.m_displayController, canRenderNewFrame()).WillOnce(Return(true));
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
        EXPECT_CALL(*displayMock.m_displayController, clearBuffer(_, _, _)).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, getEmbeddedCompositingManager()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_embeddedCompositingManager, notifyClients()).Times(AnyNumber());

        EXPECT_CALL(platformFactoryMock.windowEventsPollingManagerMock, pollWindowsTillAnyCanRender());
        EXPECT_CALL(*displayMock.m_displayController, handleWindowEvents());
        EXPECT_CALL(*displayMock.m_displayController, canRenderNewFrame()).WillOnce(Return(true));
        EXPECT_CALL(*displayMock.m_displayController, executePostProcessing()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, swapBuffers()).Times(AnyNumber());
        EXPECT_CALL(*displayMock.m_displayController, renderScene(_, _, _, _, _)).Times(AnyNumber());

        renderer.doOneRenderLoop();
    }

    void readPixels(DisplayHandle display, OffscreenBufferHandle obHandle, UInt32 x, UInt32 y, UInt32 width, UInt32 height, Bool fullscreen, Bool sendViaDLT, const String& filename)
    {
        ScreenshotInfo screenshotInfo;
        screenshotInfo.rectangle.x = x;
        screenshotInfo.rectangle.y = y;
        screenshotInfo.rectangle.width = width;
        screenshotInfo.rectangle.height = height;
        screenshotInfo.fullScreen = fullscreen;
        screenshotInfo.sendViaDLT = sendViaDLT;
        screenshotInfo.filename = filename;

        rendererSceneUpdater->handleReadPixels(display, obHandle, std::move(screenshotInfo));
    }

    void expectDisplayControllerReadPixels(DisplayHandle display, DeviceResourceHandle deviceHandle, UInt32 x, UInt32 y, UInt32 width, UInt32 height)
    {
        DisplayStrictMockInfo& displayMock = renderer.getDisplayMock(display);
        EXPECT_CALL(*displayMock.m_displayController, readPixels(deviceHandle, x, y, width, height, _)).WillOnce(Invoke(
            [](auto, auto, auto, auto w, auto h, auto& dataOut) {
                dataOut.resize(w * h * 4);
            }
        ));
    }

    static const DisplayHandle DisplayHandle1;
    static const DisplayHandle DisplayHandle2;

    const RenderableHandle renderableHandle{ 1 };
    const DataInstanceHandle uniformDataInstanceHandle{ 0 };
    const DataInstanceHandle geometryDataInstanceHandle{ 1 };

    const TextureSamplerHandle samplerHandle{ 2 };
    const StreamTextureHandle streamTextureHandle{ 0 };

    std::vector<std::unique_ptr<ActionCollectingScene>> stagingScene;
    DataSlotId dataSlotIdForDataLinking{9911u};

    static constexpr UInt ForceApplyFlushesLimit = 10u;
    static constexpr UInt ForceUnsubscribeFlushLimit = 20u;

    StrictMock<RendererResourceCacheMock> rendererResourceCacheMock;
    StrictMock<PlatformStrictMockWithPerRendererComponents> platformFactoryMock;
    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    SceneExpirationMonitor expirationMonitor;
    StrictMock<SceneReferenceLogicMock> sceneReferenceLogic;
    RendererStatistics rendererStatistics;
    StrictMock<RendererMockWithStrictMockDisplay> renderer;
    StrictMock<RendererSceneEventSenderMock> sceneEventSender;
    FrameTimer frameTimer;
    SceneStateExecutor sceneStateExecutor;
    NiceMock<ThreadAliveNotifierMock> notifier;
    std::unique_ptr<RendererSceneUpdaterFacade> rendererSceneUpdater;

    std::unordered_map<UInt32, ResourceContentHashVector> previousResources;
};
}

#endif
