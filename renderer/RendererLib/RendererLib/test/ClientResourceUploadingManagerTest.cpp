//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/ClientResourceUploadingManager.h"
#include "RendererLib/RendererClientResourceRegistry.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/RendererStatistics.h"
#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "ResourceProviderMock.h"
#include "ResourceUploaderMock.h"
#include "RenderBackendMock.h"
#include "PlatformAbstraction/PlatformThread.h"

namespace ramses_internal{

class AClientResourceUploadingManager : public ::testing::Test
{
public:
    AClientResourceUploadingManager(bool keepEffects = false, UInt64 clientResourceCacheSize = 0u)
        : dummyResource(EResourceType_IndexArray, 5, EDataType_UInt16, reinterpret_cast<const Byte*>(m_dummyData), ResourceCacheFlag_DoNotCache, String())
        , dummyEffectResource("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache)
        , dummyManagedResourceCallback(managedResourceDeleter)
        , sceneId(66u)
        , frameTimer()
        , rendererResourceUploader(resourceRegistry, uploader, rendererBackend, keepEffects, frameTimer, stats, clientResourceCacheSize)
    {
    }

    void registerAndProvideResource(ResourceContentHash hash, bool effectResource = false, const IResource* resource = nullptr)
    {
        resourceRegistry.registerResource(hash);
        resourceRegistry.addResourceRef(hash, sceneId);
        resourceRegistry.setResourceStatus(hash, EResourceStatus_Requested);
        resourceRegistry.setResourceStatus(hash, EResourceStatus_Provided);

        if (effectResource)
        {
            ManagedResource managedRes(dummyEffectResource, dummyManagedResourceCallback);
            resourceRegistry.setResourceData(hash, managedRes, DeviceResourceHandle::Invalid(), EResourceType_Effect);
        }
        else
        {
            ManagedResource managedRes((nullptr != resource? *resource : dummyResource) , dummyManagedResourceCallback);
            resourceRegistry.setResourceData(hash, managedRes, DeviceResourceHandle::Invalid(), EResourceType_IndexArray);
        }

        ASSERT_TRUE(contains_c(resourceRegistry.getAllProvidedResources(), hash));
    }

    void makeResourceUnused(ResourceContentHash hash)
    {
        resourceRegistry.removeResourceRef(hash, sceneId);
        ASSERT_TRUE(contains_c(resourceRegistry.getAllResourcesNotInUseByScenes(), hash));
    }

    void unregisterResource(ResourceContentHash hash)
    {
        const ResourceDescriptor& rd = resourceRegistry.getResourceDescriptor(hash);
        if (!rd.sceneUsage.empty())
        {
            resourceRegistry.removeResourceRef(hash, sceneId);
        }
        resourceRegistry.unregisterResource(hash);
    }

    void expectResourceUploaded(ResourceContentHash hash)
    {
        ASSERT_TRUE(resourceRegistry.containsResource(hash));
        const ResourceDescriptor& rd = resourceRegistry.getResourceDescriptor(hash);
        EXPECT_EQ(EResourceStatus_Uploaded, rd.status);
        EXPECT_EQ(ResourceUploaderMock::FakeResourceDeviceHandle, rd.deviceHandle);
        EXPECT_TRUE(rd.resource.getResourceObject() == NULL);
    }

    void expectResourceUploadFailed(ResourceContentHash hash)
    {
        ASSERT_TRUE(resourceRegistry.containsResource(hash));
        const ResourceDescriptor& rd = resourceRegistry.getResourceDescriptor(hash);
        EXPECT_EQ(EResourceStatus_Broken, rd.status);
        EXPECT_FALSE(rd.deviceHandle.isValid());
        EXPECT_TRUE(rd.resource.getResourceObject() == NULL);
    }

    void expectResourceStatus(ResourceContentHash hash, EResourceStatus status)
    {
        ASSERT_TRUE(resourceRegistry.containsResource(hash));
        const ResourceDescriptor& rd = resourceRegistry.getResourceDescriptor(hash);
        EXPECT_EQ(status, rd.status);
    }

    void expectResourceUnloaded(ResourceContentHash hash)
    {
        EXPECT_FALSE(resourceRegistry.containsResource(hash));
    }

protected:
    static const UInt16 m_dummyData[5];

    RendererClientResourceRegistry resourceRegistry;
    StrictMock<ResourceUploaderMock> uploader;
    StrictMock<RenderBackendStrictMock> rendererBackend;

    const ArrayResource dummyResource;
    const EffectResource dummyEffectResource;
    NiceMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
    ResourceDeleterCallingCallback dummyManagedResourceCallback;

    const SceneId sceneId;

    FrameTimer frameTimer;
    RendererStatistics stats;
    ClientResourceUploadingManager rendererResourceUploader;
};

const UInt16 AClientResourceUploadingManager::m_dummyData[5] = { 0x1C };

class AClientResourceUploadingManager_KeepingEffects : public AClientResourceUploadingManager
{
public:
    AClientResourceUploadingManager_KeepingEffects()
        : AClientResourceUploadingManager(true)
    {
    }
};

class AClientResourceUploadingManager_WithVRAMCache : public AClientResourceUploadingManager
{
public:
    AClientResourceUploadingManager_WithVRAMCache()
        : AClientResourceUploadingManager(false, 30u)
    {
    }
};

TEST_F(AClientResourceUploadingManager, hasNothingToUploadUnloadInitially)
{
    EXPECT_FALSE(rendererResourceUploader.hasAnythingToUpload());
    // no call expectations
    rendererResourceUploader.uploadAndUnloadPendingResources();
}

TEST_F(AClientResourceUploadingManager, reportsItemsToUploadWhenRegsitryHasProvidedResource)
{
    const ResourceContentHash res(1234u, 0u);
    registerAndProvideResource(res);

    EXPECT_TRUE(rendererResourceUploader.hasAnythingToUpload());

    unregisterResource(res);
}

TEST_F(AClientResourceUploadingManager, reportsItemsToUnloadWhenRegsitryHasUnusedResource)
{
    const ResourceContentHash res(1234u, 0u);
    registerAndProvideResource(res);
    makeResourceUnused(res);

    EXPECT_TRUE(rendererResourceUploader.hasAnythingToUpload());

    unregisterResource(res);
}

TEST_F(AClientResourceUploadingManager, uploadsProvidedResource)
{
    const ResourceContentHash res(1234u, 0u);
    registerAndProvideResource(res);

    EXPECT_CALL(uploader, uploadResource(_, _, _));
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res);

    unregisterResource(res);
}

TEST_F(AClientResourceUploadingManager, unloadsUnusedResource)
{
    const ResourceContentHash res(1234u, 0u);
    registerAndProvideResource(res);

    EXPECT_CALL(uploader, uploadResource(_, _, _));
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res);

    makeResourceUnused(res);
    EXPECT_CALL(uploader, unloadResource(_, _, _, _));
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUnloaded(res);
}

TEST_F(AClientResourceUploadingManager, setsBrokenStatusForResourceFailedToUpload)
{
    const ResourceContentHash res(1234u, 0u);
    registerAndProvideResource(res);

    EXPECT_CALL(uploader, uploadResource(_, _, _)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploadFailed(res);

    unregisterResource(res);
}

TEST_F(AClientResourceUploadingManager, uploadsAllProvidedResourcesInOneUpdate_defaultUploadStrategy)
{
    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);
    const ResourceContentHash res4(1237u, 0u);
    const ResourceContentHash res5(1238u, 0u);

    registerAndProvideResource(res1);
    registerAndProvideResource(res2);
    registerAndProvideResource(res3);
    registerAndProvideResource(res4);
    registerAndProvideResource(res5);

    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(5u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    expectResourceUploaded(res1);
    expectResourceUploaded(res2);
    expectResourceUploaded(res3);
    expectResourceUploaded(res4);
    expectResourceUploaded(res5);

    unregisterResource(res1);
    unregisterResource(res2);
    unregisterResource(res3);
    unregisterResource(res4);
    unregisterResource(res5);
}

TEST_F(AClientResourceUploadingManager, willUnloadAnyRemainingResourcesWhenDestructed)
{
    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);

    registerAndProvideResource(res1);
    registerAndProvideResource(res2, true);
    registerAndProvideResource(res3);

    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(3u);
    rendererResourceUploader.uploadAndUnloadPendingResources();
    Mock::VerifyAndClearExpectations(&uploader);

    makeResourceUnused(res1);
    makeResourceUnused(res2);
    makeResourceUnused(res3);

    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(3u);
}

TEST_F(AClientResourceUploadingManager_KeepingEffects, willNotUnloadEffectResourceUntilDestructor)
{
    const ResourceContentHash res(1234u, 0u);
    registerAndProvideResource(res, true);

    EXPECT_CALL(uploader, uploadResource(_, _, _));
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res);

    makeResourceUnused(res);
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(0u);
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res);
    Mock::VerifyAndClearExpectations(&uploader);

    // destructor will unload kept effects
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(1u);
}

TEST_F(AClientResourceUploadingManager, uploadsAtLeastOneResourcePerUpdateIfOutOfTimeBudget)
{
    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);

    registerAndProvideResource(res1, false);
    registerAndProvideResource(res2, false);
    registerAndProvideResource(res3, false);

    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, 0u);

    EXPECT_CALL(uploader, uploadResource(_, _, _));
    frameTimer.startFrame();
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res1);
    expectResourceStatus(res2, EResourceStatus_Provided);
    expectResourceStatus(res3, EResourceStatus_Provided);

    EXPECT_CALL(uploader, uploadResource(_, _, _));
    frameTimer.startFrame();
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res1);
    expectResourceUploaded(res2);
    expectResourceStatus(res3, EResourceStatus_Provided);

    makeResourceUnused(res1);
    makeResourceUnused(res2);
    unregisterResource(res3);

    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(2u);
}

TEST_F(AClientResourceUploadingManager, uploadsBatchOfResourceBeforeCheckingIfOutOfTimeBudget)
{
    // first resource is always uploaded so batch is check frequency constant + 1
    const size_t numResourcesInBatch = ClientResourceUploadingManager::NumResourcesToUploadInBetweenTimeBudgetChecks + 1;
    // create resources to fill one batch and extra resource in addition
    std::vector<ResourceContentHash> resList(numResourcesInBatch + 1);
    UInt64 hash = 1234u;
    for (auto& res : resList)
    {
        res = ResourceContentHash(hash++, 0u);
        registerAndProvideResource(res, false);
    }

    // set budget to infinite to make sure the batch gets to be processed
    // then right after set budget to 0 and expect rest of batch to be uploaded till next budget check
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(numResourcesInBatch)
        .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, std::numeric_limits<UInt64>::max()); return ResourceUploaderMock::FakeResourceDeviceHandle; }))
        .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, 0u); return ResourceUploaderMock::FakeResourceDeviceHandle; }))
        .WillRepeatedly(Return(ResourceUploaderMock::FakeResourceDeviceHandle));

    frameTimer.startFrame();
    rendererResourceUploader.uploadAndUnloadPendingResources();
    for (UInt32 i = 0u; i < numResourcesInBatch; ++i)
        expectResourceUploaded(resList[i]);

    // last resource is outside of batch and was skipped for uploading
    expectResourceStatus(resList.back(), EResourceStatus_Provided);

    for (UInt32 i = 0u; i < numResourcesInBatch; ++i)
        makeResourceUnused(resList[i]);
    unregisterResource(resList.back());

    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(numResourcesInBatch);
}

TEST_F(AClientResourceUploadingManager, checksTimeBudgetForEachLargeResourceWhenUploading)
{
    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);

    const std::vector<UInt32> dummyData(ClientResourceUploadingManager::LargeResourceByteSizeThreshold / 4 + 1, 0u);
    const ArrayResource largeResource(EResourceType_IndexArray, static_cast<UInt32>(dummyData.size()), EDataType_UInt32, reinterpret_cast<const Byte*>(dummyData.data()), ResourceCacheFlag_DoNotCache, "");

    registerAndProvideResource(res1, false, &largeResource);
    registerAndProvideResource(res2, false, &largeResource);
    registerAndProvideResource(res3, false, &largeResource);

    // set budget to infinite to make sure more than just first resource is processed
    // then right after set budget to 0 and test if the other resources were uploaded
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(2)
        .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, std::numeric_limits<UInt64>::max()); return ResourceUploaderMock::FakeResourceDeviceHandle; }))
        .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, 0u); return ResourceUploaderMock::FakeResourceDeviceHandle; }));

    frameTimer.startFrame();
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res1);
    expectResourceUploaded(res2);
    // last resource was skipped because even though within batch it is large resources and those are checked for time budget separately
    expectResourceStatus(res3, EResourceStatus_Provided);

    makeResourceUnused(res1);
    makeResourceUnused(res2);
    unregisterResource(res3);

    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(2);
}

TEST_F(AClientResourceUploadingManager, checksTimeBudgetForEachEffectWhenUploading)
{
    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);

    registerAndProvideResource(res1, true);
    registerAndProvideResource(res2, true);
    registerAndProvideResource(res3, true);

    // set budget to infinite to make sure more than just first resource is processed
    // then right after set budget to 0 and test if the other resources were uploaded
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(2)
        .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, std::numeric_limits<UInt64>::max()); return ResourceUploaderMock::FakeResourceDeviceHandle; }))
        .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, 0u); return ResourceUploaderMock::FakeResourceDeviceHandle; }));

    frameTimer.startFrame();
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res1);
    expectResourceUploaded(res2);
    // last resource was skipped because even though within batch it is effect and those are checked for time budget separately
    expectResourceStatus(res3, EResourceStatus_Provided);

    makeResourceUnused(res1);
    makeResourceUnused(res2);
    unregisterResource(res3);

    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(2);
}

TEST_F(AClientResourceUploadingManager, uploadsOnlyResourcesFittingIntoTimeBudgetInOneUpdate)
{
    // using effect resources so that time budget checks happen on every resources, not just once per batch
    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);
    const ResourceContentHash res4(1237u, 0u);

    registerAndProvideResource(res1, true);
    registerAndProvideResource(res2, true);
    registerAndProvideResource(res3, true);
    registerAndProvideResource(res4, true);

    //set section time budget so that to be enough for upload of several resources
    const UInt32 sectionTimeBudgetMiillis = 10u;
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, sectionTimeBudgetMiillis * 1000u);

    //make sure that not all resources will be uploaded
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(AtMost(3)).WillRepeatedly(InvokeWithoutArgs([&]() {PlatformThread::Sleep(4); return ResourceUploaderMock::FakeResourceDeviceHandle; }));
    frameTimer.startFrame();
    rendererResourceUploader.uploadAndUnloadPendingResources();
    // expect first res uploaded
    expectResourceUploaded(res1);
    // expect last res not uploaded
    expectResourceStatus(res4, EResourceStatus_Provided);

    // upload rest
    frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, std::numeric_limits<UInt64>::max());
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(ResourceUploaderMock::FakeResourceDeviceHandle));
    frameTimer.startFrame();
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res2);
    expectResourceUploaded(res3);
    expectResourceUploaded(res4);

    makeResourceUnused(res1);
    makeResourceUnused(res2);
    makeResourceUnused(res3);
    makeResourceUnused(res4);

    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(4);
}

TEST_F(AClientResourceUploadingManager_KeepingEffects, doesNotReportKeptEffectAsPendingUnload)
{
    const ResourceContentHash res(1234u, 0u);
    registerAndProvideResource(res, true);

    EXPECT_CALL(uploader, uploadResource(_, _, _));
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res);

    makeResourceUnused(res);

    EXPECT_FALSE(rendererResourceUploader.hasAnythingToUpload());

    // destructor will unload kept effects
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(1u);
}

TEST_F(AClientResourceUploadingManager_WithVRAMCache, willUploadResourcesEvenIfExceedingCacheSize)
{
    // test resource has size of 10 bytes
    // cache is set to 30 bytes

    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);
    const ResourceContentHash res4(1237u, 0u);
    const ResourceContentHash res5(1238u, 0u);

    registerAndProvideResource(res1);
    registerAndProvideResource(res2);
    registerAndProvideResource(res3);
    registerAndProvideResource(res4);
    registerAndProvideResource(res5);

    // all uploaded even though cache is actually smaller
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(5u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    expectResourceUploaded(res1);
    expectResourceUploaded(res2);
    expectResourceUploaded(res3);
    expectResourceUploaded(res4);
    expectResourceUploaded(res5);

    unregisterResource(res1);
    unregisterResource(res2);
    unregisterResource(res3);
    unregisterResource(res4);
    unregisterResource(res5);
}

TEST_F(AClientResourceUploadingManager_WithVRAMCache, willUnloadUnusedCachedResourcesIfCacheSizeExceeded)
{
    // test resource has size of 10 bytes
    // cache is set to 30 bytes

    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);
    const ResourceContentHash res4(1237u, 0u);
    const ResourceContentHash res5(1238u, 0u);

    registerAndProvideResource(res1);
    registerAndProvideResource(res2);
    registerAndProvideResource(res3);
    registerAndProvideResource(res4);
    registerAndProvideResource(res5);

    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(5u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    makeResourceUnused(res1);
    makeResourceUnused(res2);
    makeResourceUnused(res3);
    makeResourceUnused(res4);
    makeResourceUnused(res5);

    // unload anything exceeding cache limit
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(2u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    expectResourceUploaded(res3);
    expectResourceUploaded(res4);
    expectResourceUploaded(res5);
    Mock::VerifyAndClearExpectations(&uploader);

    // destructor will unload kept resources
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(3u);
}

TEST_F(AClientResourceUploadingManager_WithVRAMCache, willOnlyUnloadResourcesWhenNewResourcesAreToBeUploadedAndSoThatCacheSizeIsUsedAtMaximum)
{
    // test resource has size of 10 bytes
    // cache is set to 30 bytes

    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);
    const ResourceContentHash res4(1237u, 0u);

    registerAndProvideResource(res1);
    registerAndProvideResource(res2);

    // cache is 20/30 filled
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(2u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    makeResourceUnused(res2);

    // cache stays unchanged, 10/30 used resource, 10/30 unused resource, 10/30 available
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(0u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    registerAndProvideResource(res3);
    registerAndProvideResource(res4);

    // 20 bytes is needed, 10/30 is available right away, 10/30 needs to be freed
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(1u);
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(2u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    expectResourceUploaded(res1);
    expectResourceUploaded(res3);
    expectResourceUploaded(res4);
    Mock::VerifyAndClearExpectations(&uploader);

    makeResourceUnused(res1);
    makeResourceUnused(res3);
    makeResourceUnused(res4);

    // destructor will unload kept resources
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(3u);
}

TEST_F(AClientResourceUploadingManager_WithVRAMCache, willNotUnloadAnyCachedUnusedResourceIfThereIsEnoughRemainingCacheAvailable)
{
    // test resource has size of 10 bytes
    // cache is set to 30 bytes

    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);

    registerAndProvideResource(res1);
    registerAndProvideResource(res2);

    // cache is 20/30 filled
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(2u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    makeResourceUnused(res2);

    // cache stays unchanged, 10/30 used resource, 10/30 unused resource, 10/30 available
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(0u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    registerAndProvideResource(res3);

    // 10 bytes is needed, 10/30 is available right away, nothing needs to be freed
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(0u);
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(1u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    expectResourceUploaded(res1);
    expectResourceUploaded(res2);
    expectResourceUploaded(res3);
    Mock::VerifyAndClearExpectations(&uploader);

    makeResourceUnused(res1);
    makeResourceUnused(res3);

    // destructor will unload kept resources
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(3u);
}

TEST_F(AClientResourceUploadingManager_WithVRAMCache, willUnloadResourcesOfSameOrGreaterSizeOfResourcesToBeUploaded_AlreadyUsingMoreThanCacheSize)
{
    // test resource has size of 10 bytes
    // cache is set to 30 bytes

    const ResourceContentHash res1(1234u, 0u);
    const ResourceContentHash res2(1235u, 0u);
    const ResourceContentHash res3(1236u, 0u);
    const ResourceContentHash res4(1237u, 0u);
    const ResourceContentHash res5(1238u, 0u);

    registerAndProvideResource(res1);
    registerAndProvideResource(res2);
    registerAndProvideResource(res3);
    registerAndProvideResource(res4);

    // more than cache size is used
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(4u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    makeResourceUnused(res1);
    makeResourceUnused(res2);

    // unload unused resources exceeding cache limit
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(1u);
    rendererResourceUploader.uploadAndUnloadPendingResources();
    expectResourceUploaded(res2);

    registerAndProvideResource(res5);

    // cache is full and exactly 1 unused cached resource will be freed for 1 new resource to be uploaded
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(1u);
    EXPECT_CALL(uploader, uploadResource(_, _, _)).Times(1u);
    rendererResourceUploader.uploadAndUnloadPendingResources();

    expectResourceUploaded(res3);
    expectResourceUploaded(res4);
    expectResourceUploaded(res5);
    Mock::VerifyAndClearExpectations(&uploader);

    makeResourceUnused(res3);
    makeResourceUnused(res4);
    makeResourceUnused(res5);

    // destructor will unload kept resources
    EXPECT_CALL(uploader, unloadResource(_, _, _, _)).Times(3u);
}
}
