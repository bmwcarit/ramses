//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneUpdaterTest.h"
#include "TestRandom.h"
#include "Resource/EffectResource.h"
#include <memory>

namespace ramses_internal {

TEST_F(ARendererSceneUpdater, referencesAndProvidesResourcesWhenSceneMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    mapScene();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, referencesAndProvidesOnlyResourcesInUseWhenSceneMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources(0u, MockResourceHash::IndexArrayHash3);
    update();

    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    update();

    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();

    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    mapScene();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesNotInUse)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));

    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 });
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash3 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash3);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesNotInUseOnlyAfterPendingFlushesApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    // pending flush
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // unblock resource
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Uploaded);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, sameResourceComingFromMultipleScenesIsReferencedAndProvidedMultipleTimes)
{
    createDisplayAndExpectSuccess();
    const auto s1 = createPublishAndSubscribeScene();
    const auto s2 = createPublishAndSubscribeScene();
    const auto s3 = createPublishAndSubscribeScene();
    mapScene(s1);
    mapScene(s2);
    mapScene(s3);

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s1);
    createRenderable(s1);
    setRenderableResources(s1);
    update();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s2);
    createRenderable(s2);
    setRenderableResources(s2);
    update();
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s3);
    createRenderable(s3);
    setRenderableResources(s3);
    update();
    EXPECT_EQ(3, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(3, getResourceRefCount(MockResourceHash::IndexArrayHash));

    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    unmapScene(s1);
    unmapScene(s2);
    unmapScene(s3);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, sameResourceComingFromMultipleScenesIsUnreferencedWhenSceneUnmappedOrUnpublished)
{
    createDisplayAndExpectSuccess();
    const auto s1 = createPublishAndSubscribeScene();
    const auto s2 = createPublishAndSubscribeScene();
    const auto s3 = createPublishAndSubscribeScene();
    mapScene(s1);
    mapScene(s2);
    mapScene(s3);

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s1);
    createRenderable(s1);
    setRenderableResources(s1);
    update();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s2);
    createRenderable(s2);
    setRenderableResources(s2);
    update();
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s3);
    createRenderable(s3);
    setRenderableResources(s3);
    update();
    EXPECT_EQ(3, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(3, getResourceRefCount(MockResourceHash::IndexArrayHash));

    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // destroy renderable in s1
    destroyRenderable(s1);
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s1);
    update();
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));

    // unmap s1
    unmapScene(s1);
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));

    // unmap s2
    unmapScene(s2);
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    // unpublish s3
    unpublishMappedScene(s3);
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, offersProvidedResourceToResourceCacheAndStoresOnlyIfCacheWants)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();

    EXPECT_CALL(rendererResourceCacheMock, hasResource(MockResourceHash::EffectHash, _)).WillOnce(Return(false));
    EXPECT_CALL(rendererResourceCacheMock, hasResource(MockResourceHash::IndexArrayHash, _)).WillOnce(Return(false));

    // wants effect
    EXPECT_CALL(rendererResourceCacheMock, shouldResourceBeCached(MockResourceHash::EffectHash, _, ResourceCacheFlag_DoNotCache, getSceneId())).WillOnce(Return(true));
    // does not want indices
    EXPECT_CALL(rendererResourceCacheMock, shouldResourceBeCached(MockResourceHash::IndexArrayHash, _, ResourceCacheFlag_DoNotCache, getSceneId())).WillOnce(Return(false));

    // store effect
    EXPECT_CALL(rendererResourceCacheMock, storeResource(MockResourceHash::EffectHash, _, _, ResourceCacheFlag_DoNotCache, getSceneId()));

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, triggersResourceUploadAndUnloadWhenResourceProvided)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();

    // trigger unload/upload code path
    ON_CALL(*rendererSceneUpdater->m_resourceManagerMock, hasResourcesToBeUploaded()).WillByDefault(Return(true));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadAndUnloadPendingResources());
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesInUseByMapRequestedSceneWhenSceneGetsUnpublished)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    // blocking flush so that scene will never get mapped
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    requestMapScene();
    update();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectUnloadOfSceneResources();
    unpublishMapRequestedScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesInUseByRenderedSceneWhenSceneGetsUnpublished)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    expectVertexArrayUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectUnloadOfSceneResources();
    unpublishShownScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesInUseByMappedSceneWhenSceneGetsUnsubscribedByError)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectUnloadOfSceneResources();
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(_));
    unsubscribeMappedScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesInUseByMapRequestedSceneWhenSceneGetsUnsubscribedByError)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    // blocking flush so that scene will never get mapped
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    requestMapScene();
    update();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectUnloadOfSceneResources();
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(_));
    unsubscribeMapRequestedScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesInUseByRenderedSceneWhenSceneGetsUnsubscribedByError)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    expectVertexArrayUploaded();
    update();

    expectUnloadOfSceneResources();
    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(_));
    unsubscribeShownScene();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unreferencesResourcesInUseByMappedSceneWhenUpdaterDestructed)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    update();

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, confidenceTest_doesNotReferenceOrUnreferenceResourcesIfSceneNotMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable();
    setRenderableResources();

    update();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesScenesStreamTexturesCache_SingleScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    createRenderableAndResourcesWithStreamTexture();

    expectStreamTextureUploaded();
    expectVertexArrayUploaded();
    update();
    expectRenderableResourcesClean();

    constexpr WaylandIviSurfaceId source{ 12u };
    const StreamUsage fakeStreamUsage{ { {getSceneId(0u), {streamTextureHandle}} }, {} };

    const WaylandIviSurfaceIdVector changedSources{ source };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillRepeatedly(SetArgReferee<0>(changedSources));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source)).WillOnce(ReturnRef(fakeStreamUsage));
    update();
    expectRenderableResourcesDirty();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesScenesStreamTexturesCache_MultipleScenes)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    createRenderableAndResourcesWithStreamTexture(0u);
    expectStreamTextureUploaded();
    expectVertexArrayUploaded();
    update();

    createRenderableAndResourcesWithStreamTexture(1u);
    expectVertexArrayUploaded(1u);
    update();

    expectRenderableResourcesClean(0u);
    expectRenderableResourcesClean(1u);

    constexpr WaylandIviSurfaceId source{ 12u };
    const StreamUsage fakeStreamUsage{ { {getSceneId(0u), {streamTextureHandle}}, {getSceneId(1u), {streamTextureHandle}} }, {} };

    const WaylandIviSurfaceIdVector changedSources{ source };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillRepeatedly(SetArgReferee<0>(changedSources));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).Times(2).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source)).WillOnce(ReturnRef(fakeStreamUsage));
    update();
    expectRenderableResourcesDirty(0u);
    expectRenderableResourcesDirty(1u);

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, updatesScenesStreamTexturesCache_MultipleScenes_MultipleSources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();
    mapScene(0u);
    mapScene(1u);
    showScene(0u);
    showScene(1u);

    createRenderableAndResourcesWithStreamTexture(0u);
    expectStreamTextureUploaded();
    expectVertexArrayUploaded(0u);
    update();

    createRenderableAndResourcesWithStreamTexture(1u);
    expectVertexArrayUploaded(1u);
    update();

    expectRenderableResourcesClean(0u);
    expectRenderableResourcesClean(1u);

    constexpr WaylandIviSurfaceId source1{ 12u };
    constexpr WaylandIviSurfaceId source2{ 13u };
    const StreamUsage fakeStreamUsage1{ { {getSceneId(0u), {streamTextureHandle}} }, {} };
    const StreamUsage fakeStreamUsage2{ { {getSceneId(1u), {streamTextureHandle}} }, {} };

    const WaylandIviSurfaceIdVector changedSources{ source1, source2 };
    EXPECT_CALL(renderer.m_embeddedCompositingManager, dispatchStateChangesOfSources(_, _, _)).WillRepeatedly(SetArgReferee<0>(changedSources));
    EXPECT_CALL(renderer.m_embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).Times(2).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source1)).WillOnce(ReturnRef(fakeStreamUsage1));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getStreamUsage(source2)).WillOnce(ReturnRef(fakeStreamUsage2));
    update();
    expectRenderableResourcesDirty(0u);
    expectRenderableResourcesDirty(1u);

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, pendingFlushesAreNotAppliedIfResourcesNotReady)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();

    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, appliesPendingFlushesAtOnceAndInOrderWhenUnblocked_mapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    expectNoEvent();

    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    const SceneVersionTag version1(1u);
    const SceneVersionTag version2(2u);
    const SceneVersionTag version3(3u);

    performFlush(0u, version1);
    update();
    expectNoEvent();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush(0u, version2);
    update();
    expectNoEvent();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush(0u, version3);
    update();
    expectNoEvent();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock pending flushes
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    RendererEventVector events;
    RendererEventVector dummy;
    rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
    ASSERT_EQ(3u, events.size());
    EXPECT_EQ(ERendererEventType::SceneFlushed, events[0].eventType);
    EXPECT_EQ(ERendererEventType::SceneFlushed, events[1].eventType);
    EXPECT_EQ(ERendererEventType::SceneFlushed, events[2].eventType);
    EXPECT_EQ(version1.getValue(), events[0].sceneVersionTag.getValue());
    EXPECT_EQ(version2.getValue(), events[1].sceneVersionTag.getValue());
    EXPECT_EQ(version3.getValue(), events[2].sceneVersionTag.getValue());

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, uploadsAndUnloadsVertexArray)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    createRenderable();
    setRenderableResources();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    expectVertexArrayUploaded();
    update();
    expectRenderableResourcesClean();

    destroyRenderable();
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    expectVertexArrayUnloaded();
    update();
    expectRenderableResourcesClean();

    //next call to update does not release to any more attempts to unload
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotUploadVertexArrayIfRenderableVisilityOff)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();
    createRenderable(0u, false, false, EVisibilityMode::Off);
    setRenderableResources();

    update();
    expectRenderableResourcesDirty();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, appliesPendingFlushesAtOnceAndInOrderWhenUnblocked_shown)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    expectVertexArrayUploaded();
    expectNoEvent();

    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    const SceneVersionTag version1(1u);
    const SceneVersionTag version2(2u);
    const SceneVersionTag version3(3u);

    performFlush(0u, version1);
    update();
    expectNoEvent();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush(0u, version2);
    update();
    expectNoEvent();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush(0u, version3);
    update();
    expectNoEvent();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock pending flushes
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    RendererEventVector events;
    RendererEventVector dummy;
    rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
    ASSERT_EQ(3u, events.size());
    EXPECT_EQ(ERendererEventType::SceneFlushed, events[0].eventType);
    EXPECT_EQ(ERendererEventType::SceneFlushed, events[1].eventType);
    EXPECT_EQ(ERendererEventType::SceneFlushed, events[2].eventType);
    EXPECT_EQ(version1.getValue(), events[0].sceneVersionTag.getValue());
    EXPECT_EQ(version2.getValue(), events[1].sceneVersionTag.getValue());
    EXPECT_EQ(version3.getValue(), events[2].sceneVersionTag.getValue());

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, pendingFlushesAreNotAppliedUntilBlockingResourceUploadedEvenIfUnreferencedByOtherFlush)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    expectVertexArrayUploaded();
    expectNoEvent();

    // block on indices not uploaded
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // replace indices with another one which is uploaded right away
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // even though not used anymore wait until previously blocking resource uploaded
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Uploaded);
    // it will be unreferenced as unused right after
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    performFlush();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, swappingResourcesWhileBlockedEndsUpInEqualRefAndUnrefCounts)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();

    // block on indices1
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash)).Times(AnyNumber()).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // swap indices1 for indices2
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash2)).Times(AnyNumber()).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // swap back - indices1 is referenced and provided again (client sends resource once more)
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // swap once more - indices2 is referenced and provided again (client sends resource once more)
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // unblock both
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Uploaded);
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Uploaded);
    // indices is unused and therefore unreferenced
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash }, 0u, 2); // was reffed twice, no zero
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 }, 0u, 1); // unrefs are applied sequentially after apply - indices2 was swapped in/out
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // destroy renderable so there are no more resources in use
    destroyRenderable();
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash2 });
    update();

    expectNoResourceReferencedByScene();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, pendingFlushesWillBeAppliedIfSceneGetsUnmapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    unmapScene();

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, resourceUsedByRenderedSceneUnreferencedInFlushWillBeUnreferencedOnceFlushIsReady)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // renderable with resources uploaded and shown
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    expectVertexArrayUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // swap indices and block on it
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash2)).WillOnce(Return(DeviceResourceHandle::Invalid())).RetiresOnSaturation();
    expectVertexArrayUnloaded();

    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Uploaded);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canRemapScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    unmapScene();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    mapScene();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, canRemapSceneWithUnusedTextureSampler)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable(0u, false, true); // with sampler that will not be set
    setRenderableResources();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    unmapScene();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    mapScene();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, mappedSceneWithBlockingFlushGetsUnmappedAndRemapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    unmapScene();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // resource will not be blocked next time used
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);

    expectResourcesReferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    mapScene();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    EXPECT_CALL(renderer.m_platform, destroyResourceUploadRenderBackend());
    expectUnloadOfSceneResources();
    destroySceneUpdater();
}

TEST_F(ARendererSceneUpdater, newRenderTargetIsUploadedOnlyAfterPendingFlushApplied)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    update();

    // emulate blocking flush with new RT
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    createRenderTargetWithBuffers();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock pending flushes
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    expectRenderTargetUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUnloadedOnlyAfterPendingFlushAppliedIfAlreadyUsedByRendererScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectRenderTargetUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // emulate blocking flush with release of RT
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    destroyRenderTargetWithBuffers();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock pending flushes
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    expectRenderTargetUnloaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUploadedOnceWhenMappedIfCreatedBeforeMapping)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderTargetWithBuffers();
    update();

    expectRenderTargetUploaded();
    mapScene();

    expectRenderTargetUnloaded();
    destroyRenderTargetWithBuffers();
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUploadedOnceWhenCreatedIfAlreadyMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectRenderTargetUploaded();
    update();

    expectRenderTargetUnloaded();
    destroyRenderTargetWithBuffers();
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unloadsAndUploadsRenderTargetWithSameHandleDestroyedAndCreatedInSameLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectRenderTargetUploaded();
    update();

    destroyRenderTargetWithBuffers();
    createRenderTargetWithBuffers();
    expectRenderTargetUnloaded();
    expectRenderTargetUploaded();
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, doesNotUploadOrUnloadRenderTargetWithSameHandleCreatedAndDestroyedInSameLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    destroyRenderTargetWithBuffers();
    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetAndBuffersAreReuploadedAfterSceneRemapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createRenderTargetWithBuffers();
    expectRenderTargetUploaded();
    update();

    unmapScene();
    update();

    expectRenderTargetUploaded();
    mapScene();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, blitPassesReuploadedAfterSceneRemapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    createBlitPass();
    expectBlitPassUploaded();
    update();

    unmapScene();
    update();

    expectBlitPassUploaded();
    mapScene();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, blockedResourceAndUnmapWillReferenceThatResourceWithNextMap)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    unmapScene();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    mapScene();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canDestroyDisplayIfThereArePendingUploads)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, onlyMapsASceneIfAllNeededResourcesAreUploaded)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // simulate upload
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);

    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, onlyMapsASceneIfAllNeededResourcesAreUploaded_WithTwoDifferentBlockingResources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    // block 2 resources
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock 1st resource
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);

    // still blocked
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock 2nd resource
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Uploaded);

    // only now is scene mapped
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWhenSceneIsMapRequestedWithMapFailedEvent)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId());
    expectInternalSceneStateEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnmapped });
    EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId()));

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWhenSceneIsMappingAndUploadingWithMapFailedEvent)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectUnloadOfSceneResources();
    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId());
    expectInternalSceneStateEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnmapped });
    EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId()));

    update();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, unmappingSceneWhenSceneIsMappingAndUploadingWillUnloadItsResources)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    setRenderableResources();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    expectUnloadOfSceneResources();
    rendererSceneUpdater->handleSceneUnmappingRequest(getSceneId());
    expectInternalSceneStateEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnmapped });
    EXPECT_EQ(ESceneState::Subscribed, sceneStateExecutor.getSceneState(getSceneId()));

    update();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, renderTargetIsUploadedWhenSceneMappingAndUploadingEvenIfMappingBlockedByFlush)
{
    // create scene with RT
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable();
    createRenderTargetWithBuffers();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // request map but block resource
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    expectRenderTargetUploaded();
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock flush by making resource available
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);

    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);

    update();

    unmapScene();
    destroyDisplay();
}

// This is to reproduce and prove fix for a crash that happens in very very special constellation
// Note: this is probably outdated setup not reproducible in current code, keeping as confidence test
TEST_F(ARendererSceneUpdater, confidenceTest_renderTargetIsUploadedInCorrectOrderAfterSceneMappedWithShowRequestAndBlockingFlushComingAtSameTime)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    // create renderable
    createRenderTargetWithBuffers();
    createRenderable();
    setRenderableResources();

    // request map scene
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    // block resource
    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    expectRenderTargetUploaded();
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // do one loop in uploading state that will upload resolvable resources
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // swap invalid resource for another invalid (this will put the first invalid into 'to be unreferenced' list in current implementation)
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock mapping
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Uploaded);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });

    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);

    //////////////////
    // unmap scene will unload render target, invalid resources is still in 'to be unreferenced' list
    unmapScene();
    update();

    // request map
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    expectResourcesReferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash2 });
    expectRenderTargetUploaded();
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // the scene will get to mapped state and within that very frame do:
    // 1. add dummy sync flush
    // 2. switch to rendered state (show scene)
    performFlush(0u);
    update();
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);
    expectVertexArrayUploaded();
    // request show scene
    showScene();

    // if render target (or any other scene resources) is not uploaded, this would crash/assert
    update();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, multipleFlushesWithRemoveAndAddResourceReferenceWhileAlreadyBlocked)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    createRenderable();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // add new resource reference
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // remove and add again the previously added resource reference
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    update();
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash }, 0u);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();

    // update
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash }, 0u, 1);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 });
    expectVertexArrayUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, multipleFlushesWithAddAndRemoveBlockingResourceWhileAlreadyBlocked)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    createRenderable();
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // remove and add again the blocking resource
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash2 });
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash });
    expectResourcesProvided(2);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();

    // update
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Uploaded);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 });
    expectVertexArrayUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, multipleFlushesWithAddAndRemoveAndAddBlockingResourceReferenceWhileAlreadyBlocked)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    createRenderable();
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // add, remove and add again another blocking resource (MockResourceHash::IndexArrayHash)
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash }, 0u, 2);
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash2 }, 0u, 1);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    expectResourcesProvided(3);
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    update();

    // update
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Uploaded);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash }, 0u, 1);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 }, 0u, 1);
    expectVertexArrayUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, multipleFlushesWithRemoveAndAddAndRemoveBlockingResourceReferenceWhileAlreadyBlocked)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    // create blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    createRenderable();
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // remove, add and remove again another blocking resource (MockResourceHash::IndexArrayHash)
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash2 }, 0u, 2);
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash }, 0u, 1);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    expectResourcesProvided(3);
    update();

    // update
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unblock flushes
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Uploaded);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash }, 0u, 2);
    expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 }, 0u, 1);
    expectVertexArrayUploaded();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // update
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, willForceMapSceneAfterMaximumNumberOfPendingFlushesReached)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    // mapping blocked by effect
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));
    update();

    // add blocking flush so that upcoming flushes are queuing up
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    setRenderableResources();
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // will force apply and log blocking resources
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceType(_)).Times(AnyNumber());
    for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
    {
        performFlushWithCreateNodeAction();
        update();
    }
    // expect force mapped
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, willForceMapSceneAfterMaximumWaitingTimeReached)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    createRenderable();
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    // mapping blocked by effect
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));
    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));
    update();

    constexpr std::chrono::milliseconds testTimeout{ 30 };
    rendererSceneUpdater->setForceMapTimeout(testTimeout);
    std::this_thread::sleep_for(testTimeout * 2);
    update();
    update();

    // expect force mapped
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceAppliesPendingFlushesAfterMaximumNumberReachedWhenSceneMappedOrRendered)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    update();

    // add blocking flush so that upcoming flushes are queuing up
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash)).Times(AnyNumber()).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));

    setRenderableResources();
    update();

    // type queried for logging of missing resources
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceType(_)).Times(AnyNumber());

    // mapped state
    {
        // flushes are blocked due to unresolved resource
        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlushWithCreateNodeAction();
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    }

    showScene();

    // rendered state
    {
        // add new blocking flush so that upcoming flushes are queuing up
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
        expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
        reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash2)).Times(AnyNumber()).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
        setRenderableResources(0u, MockResourceHash::IndexArrayHash2);

        // flushes are blocked due to unresolved resource
        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlushWithCreateNodeAction();
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    }

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, reactsOnDynamicChangesOfFlushForceApplyLimit)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    update();

    // add blocking flush so that upcoming flushes are queuing up
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash });
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash)).Times(AnyNumber()).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    setRenderableResources();
    update();

    // Reduce flush limit -> expect force flush earlier
    constexpr UInt newShorterFlushLimit = ForceApplyFlushesLimit / 2u;
    rendererSceneUpdater->setLimitFlushesForceApply(newShorterFlushLimit);

    // type queried for logging of missing resources
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceType(_)).Times(AnyNumber());

    // mapped state
    {
        // flushes are blocked due to unresolved resource
        for (UInt i = 0u; i < newShorterFlushLimit + 1u; ++i)
        {
            performFlushWithCreateNodeAction();
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    }

    showScene();

    // rendered state
    {
        // add new blocking flush so that upcoming flushes are queuing up
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash });
        expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash2 });
        reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
        EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceDeviceHandle(MockResourceHash::IndexArrayHash2)).Times(AnyNumber()).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
        setRenderableResources(0u, MockResourceHash::IndexArrayHash2);

        // flushes are blocked due to unresolved resource
        for (UInt i = 0u; i < newShorterFlushLimit + 1u; ++i)
        {
            performFlushWithCreateNodeAction();
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    }

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, applyingPendingFlushesAfterMaximumNumberOfPendingFlushesReachedDoesNotAffectOtherScene)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createPublishAndSubscribeScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash }, 0u);
    createRenderable(0u);
    mapScene(0u);
    expectVertexArrayUploaded(0u);
    showScene(0u);

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash }, 1u);
    createRenderable(1u);
    mapScene(1u);
    expectVertexArrayUploaded(1u);
    showScene(1u);

    // blocking resource
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);

    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash }, 0u);
    setRenderableResources(0u, MockResourceHash::IndexArrayHash);
    expectVertexArrayUnloaded(0u);
    update();

    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash }, 1u);
    setRenderableResources(1u, MockResourceHash::IndexArrayHash);
    expectVertexArrayUnloaded(1u);
    update();

    // type queried for logging of missing resources
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceType(_)).Times(AnyNumber());

    {
        // flushes are blocked due to unresolved resource
        expectVertexArrayUploaded(1u);
        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlushWithCreateNodeAction(1u);
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        // but only for scene 1, scene 0 stays blocked as the number of pending flushes there is below the maximum threshold
        EXPECT_FALSE(lastFlushWasAppliedOnRendererScene(0u));
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(1u));

        // repeat for scene 0
        expectVertexArrayUploaded(0u);
        for (UInt i = 0u; i < ForceApplyFlushesLimit + 1u; ++i)
        {
            performFlushWithCreateNodeAction(0u);
            update();
        }

        // after maximum of pending flushes was reached the flushes were applied regardless of missing resource
        // also for scene 0 now
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(0u));
        EXPECT_TRUE(lastFlushWasAppliedOnRendererScene(1u));
    }

    hideScene(0u);
    hideScene(1u);
    unmapScene(0u);
    unmapScene(1u);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, nonBlockingFlushesGetAppliedEvenIfSceneIsBlockedToBeMappedDueToInvalidResourceFromBefore)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();

    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));
    // scene is now blocked to be mapped due to effect

    // blocking flush cannot be applied
    expectResourcesReferencedAndProvided({ MockResourceHash::IndexArrayHash });
    setRenderableResources();
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    // scene still cannot be mapped because it still uses effect, also flushes are blocking now
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock indices meaning that the pending flushes are non-blocking
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Uploaded);
    update();
    // pending flushes were applied even though scene is waiting to be mapped
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    // scene still cannot be mapped because it still uses effect
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    update();

    // unblock also effect
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    update();
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWithPendingFlushAndRequestMapInSingleLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    // blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());

    // unmap and request map
    unmapScene();
    expectResourcesReferenced({ MockResourceHash::EffectHash });
    requestMapScene();
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));

    // scene is not mapped so pending flushes are applied to scene
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    // and scene switches to mapping/uploading state
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // next update causes all resources in use to be uploaded if available
    update();
    // scene is now blocked to be mapped due to use of effect
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock effect
    reportResourceAs(MockResourceHash::EffectHash, EResourceStatus::Uploaded);
    update();
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    update();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, canUnmapSceneWithPendingFlushAndRequestMapAndAddAnotherPendingFlushInSingleLoop)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    // blocking flush
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    reportResourceAs(MockResourceHash::IndexArrayHash, EResourceStatus::Provided);
    update();
    EXPECT_FALSE(lastFlushWasAppliedOnRendererScene());
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // unmap and request map
    unmapScene();
    requestMapScene();
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // in addition add one more blocking flush replacing previous blocking resource
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    EXPECT_EQ(ESceneState::MapRequested, sceneStateExecutor.getSceneState(getSceneId()));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    // indices1 was unreferenced when unmapped
    // it will be also replaced with indices2 when flush from above gets applied so it will not be requested again when mapped
    // scene is not mapped so pending flushes are applied to scene
    expectResourcesReferenced({ MockResourceHash::EffectHash }, 0u, 1); // sync logic to compensate missing ref for new logic when scene re-mapped
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash2 }, 0u, 1); // new logic (new flush with provided indices2)
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Provided);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());
    // and scene switches to mapping/uploading state
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2)); // new logic ref comes later before providing

    // next update causes all resources in use to be uploaded if available
    expectResourcesProvided(1); // only indices2 is provided because it was kept by new logic for mapping
    update();
    // scene is now blocked to be mapped due to use of indices2
    EXPECT_EQ(ESceneState::MappingAndUploading, sceneStateExecutor.getSceneState(getSceneId()));

    // unblock indices2
    reportResourceAs(MockResourceHash::IndexArrayHash2, EResourceStatus::Uploaded);
    update();
    expectInternalSceneStateEvent(ERendererEventType::SceneMapped);
    EXPECT_EQ(ESceneState::Mapped, sceneStateExecutor.getSceneState(getSceneId()));

    update();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceUnsubscribesSceneIfSceneResourcesUploadExceedsBudgetWhileMapping)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();

    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneResourcesUpload, 0u);

    // create many scene resources (if not enough scene resources the budget is not even checked)
    for (UInt32 i = 0; i < 40; ++i)
    {
        createRenderTargetWithBuffers(0u, RenderTargetHandle{ i }, RenderBufferHandle{ i * 2 }, RenderBufferHandle{ i * 2 + 1 });
        update();
        expectNoEvent();
    }

    // expect scene unsubscribe
    requestMapScene();

    // context is enabled twice, first before uploading, second when unloading due to forced unsubscribe/destroy
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadRenderTargetBuffer(_, getSceneId(), _)).Times(AtLeast(2));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadRenderTarget(_, _, getSceneId())).Times(AnyNumber());
    // render buffers are collected first therefore render targets might or might not be uploaded before interruption, depending on checking frequency (internal logic of scene resources uploader)

    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(_));

    expectUnloadOfSceneResources();
    update();
    expectInternalSceneStateEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect });

    update();
    expectNoEvent();

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, forceUnsubscribesSceneIfSceneResourcesUploadExceedsBudgetWhileMapping_doesNotAffectOtherAlreadyRenderedScene)
{
    createDisplayAndExpectSuccess();
    const auto sceneIdx1 = createPublishAndSubscribeScene();

    // create scene with some resources and render it
    mapScene(sceneIdx1);
    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable(sceneIdx1);
    setRenderableResources(sceneIdx1);
    expectVertexArrayUploaded();
    update();
    showScene(sceneIdx1);

    // next one is malicious scene with too many scene resources
    const auto sceneIdx2 = createPublishAndSubscribeScene();
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::SceneResourcesUpload, 0u);

    // create many scene resources (if not enough scene resources the budget is not even checked)
    for (UInt32 i = 0; i < 40; ++i)
    {
        createRenderTargetWithBuffers(sceneIdx2, RenderTargetHandle{ i }, RenderBufferHandle{ i * 2 }, RenderBufferHandle{ i * 2 + 1 });
        update();
        expectNoEvent();
    }

    // expect scene unsubscribe
    requestMapScene(sceneIdx2);

    // context is enabled twice, first before uploading, second when unloading due to forced unsubscribe/destroy
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadRenderTargetBuffer(_, getSceneId(sceneIdx2), _)).Times(AtLeast(2));
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadRenderTarget(_, _, getSceneId(sceneIdx2))).Times(AnyNumber());
    // render buffers are collected first therefore render targets might or might not be uploaded before interruption, depending on checking frequency (internal logic of scene resources uploader)

    EXPECT_CALL(sceneEventSender, sendUnsubscribeScene(_));

    expectUnloadOfSceneResources(sceneIdx2);
    update();
    expectInternalSceneStateEvents({ ERendererEventType::SceneMapFailed, ERendererEventType::SceneUnsubscribedIndirect });

    update();
    expectNoEvent();

    // hide and unmap first scene
    hideScene(sceneIdx1);
    unmapScene(sceneIdx1);

    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, confidenceTest_forceApplyPendingFlushes_keepCyclingThroughResourcesAndSimulateRandomlyCannotUpload)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();
    showScene();

    const std::array<ResourceContentHash, 3> resourcesToCycle{ MockResourceHash::IndexArrayHash, MockResourceHash::IndexArrayHash2, MockResourceHash::IndexArrayHash3 };

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, resourcesToCycle[0] });
    createRenderable();
    setRenderableResources(0u, resourcesToCycle[0]);
    expectVertexArrayUploaded();
    update();

    constexpr int NumFlushesToForceApply = 5;
    rendererSceneUpdater->setLimitFlushesForceApply(NumFlushesToForceApply);

    // due to random resources not uploaded strict un/ref mocking is not feasible
    // important check is at end that resources ref count reaches zero
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, referenceResourcesForScene(getSceneId(), _)).Times(AnyNumber());
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, unreferenceResourcesForScene(getSceneId(), _)).Times(AnyNumber());
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, provideResourceData(_)).Times(AnyNumber());
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, uploadVertexArray(_, _, getSceneId())).Times(AnyNumber());
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, unloadVertexArray(_, getSceneId())).Times(AnyNumber());
    // will force apply and log blocking resources
    EXPECT_CALL(*rendererSceneUpdater->m_resourceManagerMock, getResourceType(_)).Times(AnyNumber());

    // start from 1, 0 is active at this point
    for (int i = 1; i < 100; ++i)
    {
        ResourceContentHash nextRes = resourcesToCycle[i % 3];

        if (lastFlushWasAppliedOnRendererScene())
        {
            // reset reporting of all resources to default - uploaded
            for (const auto r : resourcesToCycle)
                reportResourceAs(r, EResourceStatus::Uploaded);

            // randomly block next resource
            if (TestRandom::Get(0, 10) > 7)
                reportResourceAs(nextRes, EResourceStatus::Provided);
        }

        setRenderableResources(0u, nextRes);
        update();
    }

    destroyRenderable();
    update();

    // in case loop ended with blocking flush reset reporting of all resources to uploaded
    for (const auto r : resourcesToCycle)
        reportResourceAs(r, EResourceStatus::Uploaded);
    update();
    EXPECT_TRUE(lastFlushWasAppliedOnRendererScene());

    // make sure that after destroying renderable reference counts reach zero
    expectNoResourceReferencedByScene();

    hideScene();
    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, referencesAndUnreferencesResourcesSharedByTwoRenderablesFromTwoScenes)
{
    createDisplayAndExpectSuccess();

    const auto s1 = createPublishAndSubscribeScene();
    const auto s2 = createPublishAndSubscribeScene();
    mapScene(s1);
    mapScene(s2);

    expectResourcesReferenced({ MockResourceHash::EffectHash }, s1);
    expectResourcesReferenced({ MockResourceHash::EffectHash }, s2);
    expectResourcesProvided(2);
    createRenderable(s1);
    createRenderable(s2);
    update();
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash3));

    // res1 used
    {
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash }, s1);
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash }, s2);
        expectResourcesProvided(2);
        setRenderableResources(s1);
        setRenderableResources(s2);
        update();
    }
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash3));

    // res2 used, res1 unneeded
    {
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash }, s1);
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash }, s2);
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash2 }, s1);
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash2 }, s2);
        expectResourcesProvided(2);
        setRenderableResources(s1, MockResourceHash::IndexArrayHash2);
        setRenderableResources(s2, MockResourceHash::IndexArrayHash2);
        update();
    }
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash2));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash3));

    // res3 used, res2 unneeded, res1 unloaded
    {
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 }, s1);
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash2 }, s2);
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash3 }, s1);
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash3 }, s2);
        expectResourcesProvided(2);
        setRenderableResources(s1, MockResourceHash::IndexArrayHash3);
        setRenderableResources(s2, MockResourceHash::IndexArrayHash3);
        update();
    }
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash3));

    // res1 used, res3 unneeded, res2 unloaded
    {
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash3 }, s1);
        expectResourcesUnreferenced({ MockResourceHash::IndexArrayHash3 }, s2);
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash }, s1);
        expectResourcesReferenced({ MockResourceHash::IndexArrayHash }, s2);
        expectResourcesProvided(2);
        setRenderableResources(s1, MockResourceHash::IndexArrayHash);
        setRenderableResources(s2, MockResourceHash::IndexArrayHash);
        update();
    }
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(2, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash3));

    destroyRenderable(s1);
    destroyRenderable(s2);
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s1);
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash }, s2);
    update();
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash3));
    expectNoResourceReferencedByScene(s1);
    expectNoResourceReferencedByScene(s2);

    unmapScene(s1);
    unmapScene(s2);
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, ConsolidatesNoMoreNeededResourcesBeforeMapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    createRenderable();

    // res1 used
    setRenderableResources();
    update();

    // res2 used, res1 unneeded
    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);
    update();

    // res3 used, res1 + res2 unneeded
    setRenderableResources(0u, MockResourceHash::IndexArrayHash3);
    update();

    // even if cycled through 3 resources only last one is uploaded
    expectResourcesReferencedAndProvided_altogether({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash3 });
    mapScene();

    destroyRenderable();
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash3 });
    update();
    expectNoResourceReferencedByScene();

    unmapScene();
    destroyDisplay();
}

/////////////////////
// tests to make sure request logic and new push logic can co-exist
/////////////////////

TEST_F(ARendererSceneUpdater, keepsTrackOfRefsAfterSceneRemapped)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    update();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    unmapScene();
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));

    expectResourcesReferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    mapScene();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    destroyRenderable();
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    update();
    expectNoResourceReferencedByScene();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, keepsTrackOfRefsAfterSceneRemapped_resourceAddedInBetweenReMappingIsProvided)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash });
    createRenderable();
    update();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));

    unmapScene();
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));

    setRenderableResources();

    expectResourcesReferenced({ MockResourceHash::EffectHash }); // extra ref due to re-map for new logic
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash }); // ref for new logic from arrived resource
    expectResourcesProvided(); // provided resource added in between mapping
    mapScene();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));

    destroyRenderable();
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    update();
    expectNoResourceReferencedByScene();

    unmapScene();
    destroyDisplay();
}

TEST_F(ARendererSceneUpdater, keepsTrackOfRefsAfterSceneRemapped_resourceAddedAndRemovedInBetweenReMapping)
{
    createDisplayAndExpectSuccess();
    createPublishAndSubscribeScene();
    mapScene();

    expectResourcesReferencedAndProvided({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash });
    createRenderable();
    setRenderableResources();
    update();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    unmapScene();
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    setRenderableResources(0u, MockResourceHash::IndexArrayHash2);

    expectResourcesReferenced({ MockResourceHash::EffectHash }); // extra ref due to re-map for new logic
    expectResourcesReferenced({ MockResourceHash::IndexArrayHash2 }); // ref for new logic from arrived resource
    expectResourcesProvided(); // provided resource added in between mapping
    mapScene();
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::EffectHash));
    EXPECT_EQ(0, getResourceRefCount(MockResourceHash::IndexArrayHash));
    EXPECT_EQ(1, getResourceRefCount(MockResourceHash::IndexArrayHash2));

    destroyRenderable();
    expectResourcesUnreferenced({ MockResourceHash::EffectHash, MockResourceHash::IndexArrayHash2 });
    update();
    expectNoResourceReferencedByScene();

    unmapScene();
    destroyDisplay();
}

}
