//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/ResourceUploadingManager.h"
#include "internal/RendererLib/RendererResourceRegistry.h"
#include "internal/RendererLib/FrameTimer.h"
#include "internal/RendererLib/RendererStatistics.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "ResourceUploaderMock.h"
#include "ResourceMock.h"
#include "PlatformMock.h"
#include "MockResourceHash.h"
#include "internal/Components/ResourceDeleterCallingCallback.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Watchdog/ThreadAliveNotifierMock.h"
#include "internal/Core/Utils/ThreadLocalLog.h"


namespace ramses::internal
{

    namespace
    {
        DisplayConfig makeConfig(uint64_t resourceCacheSize, SceneId preferredScene, SceneId deprivedScene)
        {
            DisplayConfig cfg;
            cfg.setGPUMemoryCacheSize(resourceCacheSize);
            if (preferredScene.isValid())
            {
                cfg.setScenePriority(preferredScene, -1);
            }
            if (deprivedScene.isValid())
            {
                cfg.setScenePriority(deprivedScene, 12);
            }
            return cfg;
        }
    }

    class AResourceUploadingManager : public ::testing::Test
    {
    public:
        explicit AResourceUploadingManager(const DisplayConfig& cfg = DisplayConfig())
            : dummyResource(EResourceType::IndexArray, 5, EDataType::UInt16, reinterpret_cast<const std::byte*>(m_dummyData), {})
            , dummyEffectResource("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "")
            , dummyManagedResourceCallback(managedResourceDeleter)
            , sceneId(66u)
            , uploader(new StrictMock<ResourceUploaderMock>)
            , asyncEffectUploader(platformMock, platformMock.renderBackendMock, notifier, 1)
            , rendererResourceUploader(resourceRegistry, std::unique_ptr<IResourceUploader>{ uploader }, platformMock.renderBackendMock, asyncEffectUploader, cfg, frameTimer, stats)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);

            InSequence s;
            EXPECT_CALL(platformMock.renderBackendMock.contextMock, disable()).WillOnce(Return(true));
            EXPECT_CALL(platformMock, createResourceUploadRenderBackend());
            EXPECT_CALL(platformMock.renderBackendMock.contextMock, enable()).WillOnce(Return(true));
            const bool status = asyncEffectUploader.createResourceUploadRenderBackendAndStartThread();
            EXPECT_TRUE(status);
        }

        ~AResourceUploadingManager() override
        {
            EXPECT_CALL(platformMock, destroyResourceUploadRenderBackend());
            asyncEffectUploader.destroyResourceUploadRenderBackendAndStopThread();
        }

        void registerAndProvideResource(ResourceContentHash hash, bool effectResource = false, const IResource* resource = nullptr, SceneId id = {})
        {
            resourceRegistry.registerResource(hash);
            resourceRegistry.addResourceRef(hash, id.isValid() ? id : sceneId);

            if (!resource)
                resource = (effectResource ? static_cast<const IResource*>(&dummyEffectResource) : &dummyResource);
            resourceRegistry.setResourceData(hash, ManagedResource{ resource, dummyManagedResourceCallback });

            ASSERT_TRUE(contains_c(resourceRegistry.getAllProvidedResources(), hash));
        }

        void makeResourceUnused(ResourceContentHash hash, SceneId id = {})
        {
            resourceRegistry.removeResourceRef(hash, id.isValid() ? id : sceneId);
            if (resourceRegistry.containsResource(hash))
            {
                ASSERT_TRUE(contains_c(resourceRegistry.getAllResourcesNotInUseByScenes(), hash));
            }
        }

        void expectResourceUploaded(ResourceContentHash hash, DeviceResourceHandle deviceHandle = ResourceUploaderMock::FakeResourceDeviceHandle)
        {
            ASSERT_TRUE(resourceRegistry.containsResource(hash));
            const ResourceDescriptor& rd = resourceRegistry.getResourceDescriptor(hash);
            EXPECT_EQ(EResourceStatus::Uploaded, rd.status);
            EXPECT_EQ(deviceHandle, rd.deviceHandle);
            EXPECT_FALSE(rd.resource);
        }

        void expectResourceUploadFailed(ResourceContentHash hash)
        {
            ASSERT_TRUE(resourceRegistry.containsResource(hash));
            const ResourceDescriptor& rd = resourceRegistry.getResourceDescriptor(hash);
            EXPECT_EQ(EResourceStatus::Broken, rd.status);
            EXPECT_FALSE(rd.deviceHandle.isValid());
            EXPECT_FALSE(rd.resource);
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

        void expectDeviceFlushOnWindows()
        {
#if defined(_WIN32)
            EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, flush());
#endif
        }

        void uploadShader(const ResourceContentHash& hash, bool expectSuccess = true)
        {
            const std::optional<DeviceResourceHandle> unsetDeviceHandle;
            EXPECT_CALL(*uploader, uploadResource(_, _, _)).WillOnce(Return(unsetDeviceHandle));

            if (expectSuccess)
            {
                EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(_));
                expectDeviceFlushOnWindows();
                EXPECT_CALL(platformMock.renderBackendMock.deviceMock, registerShader(_));
                EXPECT_CALL(*uploader, storeShaderInBinaryShaderCache(Ref(platformMock.renderBackendMock), DeviceMock::FakeShaderDeviceHandle, hash, sceneId));
            }
            else
            {
                EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).WillOnce(Invoke([](const auto& /*unused*/) {return std::move(std::unique_ptr<const GPUResource>{}); }));
                expectDeviceFlushOnWindows();
                EXPECT_CALL(platformMock.renderBackendMock.deviceMock, registerShader(_)).Times(0);
                EXPECT_CALL(*uploader, storeShaderInBinaryShaderCache(_, _, _, _)).Times(0);
            }

            rendererResourceUploader.uploadAndUnloadPendingResources();
            ASSERT_EQ(EResourceStatus::ScheduledForUpload, resourceRegistry.getResourceStatus(hash));

            constexpr std::chrono::seconds timeoutTime{ 2u };
            const auto startTime = std::chrono::steady_clock::now();
            while (resourceRegistry.getResourceStatus(hash) == EResourceStatus::ScheduledForUpload
                && std::chrono::steady_clock::now() - startTime < timeoutTime)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 5u });
                rendererResourceUploader.uploadAndUnloadPendingResources();
            }
        }

    protected:
        static const uint16_t m_dummyData[5];

        RendererResourceRegistry resourceRegistry;
        StrictMock<PlatformStrictMock> platformMock;

        const ArrayResource dummyResource;
        const EffectResource dummyEffectResource;
        NiceMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
        ResourceDeleterCallingCallback dummyManagedResourceCallback;

        const SceneId sceneId;

        FrameTimer frameTimer;
        RendererStatistics stats;
        NiceMock<ThreadAliveNotifierMock> notifier;
        StrictMock<ResourceUploaderMock>* uploader;
        AsyncEffectUploader asyncEffectUploader;
        ResourceUploadingManager rendererResourceUploader;
    };

    const uint16_t AResourceUploadingManager::m_dummyData[5] = { 0x1C };

    class AResourceUploadingManager_WithVRAMCache : public AResourceUploadingManager
    {
    public:
        AResourceUploadingManager_WithVRAMCache()
            : AResourceUploadingManager(makeConfig(30u, {}, {}))
        {
        }
    };

    class AResourceUploadingManager_ScenePriority : public AResourceUploadingManager
    {
    public:
        AResourceUploadingManager_ScenePriority()
            : AResourceUploadingManager(makeConfig(0u, getPreferredScene(), getDeprivedScene()))
        {
        }

        static SceneId getPreferredScene()
        {
            return SceneId{113114};
        }

        static SceneId getDeprivedScene()
        {
            return SceneId{551122};
        }
    };

    TEST_F(AResourceUploadingManager, hasNothingToUploadUnloadInitially)
    {
        EXPECT_FALSE(rendererResourceUploader.hasAnythingToUpload());
        // no call expectations
        rendererResourceUploader.uploadAndUnloadPendingResources();
    }

    TEST_F(AResourceUploadingManager, reportsItemsToUploadWhenRegistryHasProvidedResource)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res);

        EXPECT_TRUE(rendererResourceUploader.hasAnythingToUpload());

        makeResourceUnused(res);
    }

    TEST_F(AResourceUploadingManager, reportsItemsToUploadWhenRegistryHasScheduledForUploadResource)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res);
        resourceRegistry.setResourceScheduledForUpload(res);
        EXPECT_TRUE(resourceRegistry.hasAnyResourcesScheduledForUpload());
        EXPECT_EQ(EResourceStatus::ScheduledForUpload, resourceRegistry.getResourceStatus(res));

        EXPECT_TRUE(rendererResourceUploader.hasAnythingToUpload());

        resourceRegistry.setResourceBroken(res);
        makeResourceUnused(res);
    }

    TEST_F(AResourceUploadingManager, reportsNothingToUploadWhenProvidedResourceUnused)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res);
        makeResourceUnused(res);

        EXPECT_FALSE(rendererResourceUploader.hasAnythingToUpload());
    }

    TEST_F(AResourceUploadingManager, uploadsProvidedResource)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res);

        EXPECT_CALL(*uploader, uploadResource(_, _, _));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _));
        makeResourceUnused(res);
    }

    TEST_F(AResourceUploadingManager, unloadsUnusedResource)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res);

        EXPECT_CALL(*uploader, uploadResource(_, _, _));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res);

        makeResourceUnused(res);
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUnloaded(res);
    }

    TEST_F(AResourceUploadingManager, setsBrokenStatusForResourceFailedToUpload)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res);

        EXPECT_CALL(*uploader, uploadResource(_, _, _)).WillOnce(Return(DeviceResourceHandle::Invalid()));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploadFailed(res);

        makeResourceUnused(res);
    }

    TEST_F(AResourceUploadingManager, uploadEffectAndStoreInBinaryShaderCacheIfResourceUploaderDidNotReturnDeviceHandle)
    {
        const auto resHash = dummyEffectResource.getHash();
        registerAndProvideResource(resHash, true);

        uploadShader(resHash);
        expectResourceUploaded(resHash, DeviceMock::FakeShaderDeviceHandle);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _));
        makeResourceUnused(resHash);
    }

    TEST_F(AResourceUploadingManager, setsBrokenStatusForEffectIfUploadFailed)
    {
        const auto resHash = dummyEffectResource.getHash();
        registerAndProvideResource(resHash, true);

        uploadShader(resHash, false);
        expectResourceUploadFailed(resHash);

        makeResourceUnused(resHash);
    }

    TEST_F(AResourceUploadingManager, uploadsAllProvidedResourcesInOneUpdate_defaultUploadStrategy)
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

        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(5u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        expectResourceUploaded(res1);
        expectResourceUploaded(res2);
        expectResourceUploaded(res3);
        expectResourceUploaded(res4);
        expectResourceUploaded(res5);

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);
        makeResourceUnused(res4);
        makeResourceUnused(res5);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(5);
    }

    TEST_F(AResourceUploadingManager, willUnloadAnyRemainingResourcesWhenDestructed)
    {
        const ResourceContentHash res1(1234u, 0u);
        const ResourceContentHash res2(1235u, 0u);
        const ResourceContentHash res3(1236u, 0u);

        registerAndProvideResource(res1);
        registerAndProvideResource(res2, true);
        registerAndProvideResource(res3);

        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(3u);
        rendererResourceUploader.uploadAndUnloadPendingResources();
        Mock::VerifyAndClearExpectations(&uploader);

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(3u);
    }

    TEST_F(AResourceUploadingManager, willNotUnloadEffectResourceUntilDestructor)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res, true);

        EXPECT_CALL(*uploader, uploadResource(_, _, _));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res);

        makeResourceUnused(res);
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(0u);
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res);
        Mock::VerifyAndClearExpectations(&uploader);

        // destructor will unload kept effects
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(1u);
    }

    TEST_F(AResourceUploadingManager, uploadsAtLeastOneResourcePerUpdateIfOutOfTimeBudget)
    {
        const ResourceContentHash res1(1234u, 0u);
        const ResourceContentHash res2(1235u, 0u);
        const ResourceContentHash res3(1236u, 0u);

        registerAndProvideResource(res1, false);
        registerAndProvideResource(res2, false);
        registerAndProvideResource(res3, false);

        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, 0u);

        EXPECT_CALL(*uploader, uploadResource(_, _, _));
        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res1);
        expectResourceStatus(res2, EResourceStatus::Provided);
        expectResourceStatus(res3, EResourceStatus::Provided);

        EXPECT_CALL(*uploader, uploadResource(_, _, _));
        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res1);
        expectResourceUploaded(res2);
        expectResourceStatus(res3, EResourceStatus::Provided);

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(2u);
    }

    TEST_F(AResourceUploadingManager, uploadsBatchOfResourceBeforeCheckingIfOutOfTimeBudget)
    {
        // first resource is always uploaded so batch is check frequency constant + 1
        const auto numResourcesInBatch = static_cast<int>(rendererResourceUploader.getResourceUploadBatchSize()) + 1;
        // create resources to fill one batch and extra resource in addition
        std::vector<ResourceContentHash> resList(numResourcesInBatch + 1);
        uint64_t hash = 1234u;
        for (auto& res : resList)
        {
            res = ResourceContentHash(hash++, 0u);
            registerAndProvideResource(res, false);
        }

        // set budget to infinite to make sure the batch gets to be processed
        // then right after set budget to 0 and expect rest of batch to be uploaded till next budget check
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(numResourcesInBatch)
            .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, std::numeric_limits<uint64_t>::max()); return ResourceUploaderMock::FakeResourceDeviceHandle; }))
            .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, 0u); return ResourceUploaderMock::FakeResourceDeviceHandle; }))
            .WillRepeatedly(Return(ResourceUploaderMock::FakeResourceDeviceHandle));

        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        for (int i = 0; i < numResourcesInBatch; ++i)
            expectResourceUploaded(resList[i]);

        // last resource is outside of batch and was skipped for uploading
        expectResourceStatus(resList.back(), EResourceStatus::Provided);

        for (const auto& res : resList)
            makeResourceUnused(res);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(numResourcesInBatch);
    }

    TEST_F(AResourceUploadingManager, checksTimeBudgetForEachLargeResourceWhenUploading)
    {
        const ResourceContentHash res1(1234u, 0u);
        const ResourceContentHash res2(1235u, 0u);
        const ResourceContentHash res3(1236u, 0u);

        const std::vector<uint32_t> dummyData(ResourceUploadingManager::LargeResourceByteSizeThreshold / 4 + 1, 0u);
        const ArrayResource largeResource(EResourceType::IndexArray, static_cast<uint32_t>(dummyData.size()), EDataType::UInt32, dummyData.data(), "");

        registerAndProvideResource(res1, false, &largeResource);
        registerAndProvideResource(res2, false, &largeResource);
        registerAndProvideResource(res3, false, &largeResource);

        // set budget to infinite to make sure more than just first resource is processed
        // then right after set budget to 0 and test if the other resources were uploaded
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(2)
            .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, std::numeric_limits<uint64_t>::max()); return ResourceUploaderMock::FakeResourceDeviceHandle; }))
            .WillOnce(InvokeWithoutArgs([this]() { frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, 0u); return ResourceUploaderMock::FakeResourceDeviceHandle; }));

        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res1);
        expectResourceUploaded(res2);
        // last resource was skipped because even though within batch it is large resources and those are checked for time budget separately
        expectResourceStatus(res3, EResourceStatus::Provided);

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(2);
    }

    TEST_F(AResourceUploadingManager, uploadsOnlyResourcesFittingIntoTimeBudgetInOneUpdate_uploadSlow)
    {
        const std::vector<uint32_t> dummyData(ResourceUploadingManager::LargeResourceByteSizeThreshold / 4 + 1, 0u);
        const ArrayResource largeResource(EResourceType::IndexArray, static_cast<uint32_t>(dummyData.size()), EDataType::UInt32, dummyData.data(), "");

        // using large resources so that time budget checks happen on every resources, not just once per batch
        const ResourceContentHash res1(1234u, 0u);
        const ResourceContentHash res2(1235u, 0u);
        const ResourceContentHash res3(1236u, 0u);
        const ResourceContentHash res4(1237u, 0u);

        registerAndProvideResource(res1, false, &largeResource);
        registerAndProvideResource(res2, false, &largeResource);
        registerAndProvideResource(res3, false, &largeResource);
        registerAndProvideResource(res4, false, &largeResource);

        //set section time budget so that to be enough for upload of several resources
        const uint32_t sectionTimeBudgetMiillis = 10u;
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, sectionTimeBudgetMiillis * 1000u);

        //make sure that not all resources will be uploaded
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(AtMost(3)).WillRepeatedly(InvokeWithoutArgs([&]() {PlatformThread::Sleep(4); return ResourceUploaderMock::FakeResourceDeviceHandle; }));
        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        // expect first res uploaded
        expectResourceUploaded(res1);
        // expect last res not uploaded
        expectResourceStatus(res4, EResourceStatus::Provided);

        // upload rest
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, std::numeric_limits<uint64_t>::max());
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(ResourceUploaderMock::FakeResourceDeviceHandle));
        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res2);
        expectResourceUploaded(res3);
        expectResourceUploaded(res4);

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);
        makeResourceUnused(res4);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(4);
    }

    TEST_F(AResourceUploadingManager, uploadsOnlyResourcesFittingIntoTimeBudgetInOneUpdate_decompressSlow)
    {
        const ResourceContentHash res1(1234u, 0u);
        const ResourceContentHash res2(1235u, 0u);
        const ResourceContentHash res3(1236u, 0u);
        const ResourceContentHash res4(1237u, 0u);

        NiceMock<ResourceMock> resource1{ res1, EResourceType::IndexArray };
        NiceMock<ResourceMock> resource2{ res2, EResourceType::IndexArray };
        NiceMock<ResourceMock> resource3{ res3, EResourceType::IndexArray };
        NiceMock<ResourceMock> resource4{ res4, EResourceType::IndexArray };

        // simulate large resources so that time budget checks happen on every resources, not just once per batch
        ON_CALL(resource1, getDecompressedDataSize()).WillByDefault(Return(ResourceUploadingManager::LargeResourceByteSizeThreshold + 1));
        ON_CALL(resource2, getDecompressedDataSize()).WillByDefault(Return(ResourceUploadingManager::LargeResourceByteSizeThreshold + 1));
        ON_CALL(resource3, getDecompressedDataSize()).WillByDefault(Return(ResourceUploadingManager::LargeResourceByteSizeThreshold + 1));
        ON_CALL(resource4, getDecompressedDataSize()).WillByDefault(Return(ResourceUploadingManager::LargeResourceByteSizeThreshold + 1));
        ON_CALL(resource1, isDeCompressedAvailable()).WillByDefault(Return(true));
        ON_CALL(resource2, isDeCompressedAvailable()).WillByDefault(Return(true));
        ON_CALL(resource3, isDeCompressedAvailable()).WillByDefault(Return(true));
        ON_CALL(resource4, isDeCompressedAvailable()).WillByDefault(Return(true));

        registerAndProvideResource(res1, false, &resource1);
        registerAndProvideResource(res2, false, &resource2);
        registerAndProvideResource(res3, false, &resource3);
        registerAndProvideResource(res4, false, &resource4);

        const uint32_t sectionTimeBudgetMiillis = 10u;
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, sectionTimeBudgetMiillis * 1000u);

        //make sure that not all resources will be uploaded
        EXPECT_CALL(resource1, decompress()).Times(AtMost(3)).WillRepeatedly(InvokeWithoutArgs([]() { PlatformThread::Sleep(10); }));

        frameTimer.startFrame();
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(ResourceUploaderMock::FakeResourceDeviceHandle));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        // expect first res uploaded
        expectResourceUploaded(res1);
        // expect last res not uploaded
        expectResourceStatus(res4, EResourceStatus::Provided);

        // upload rest
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, std::numeric_limits<uint64_t>::max());
        frameTimer.startFrame();
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(AtLeast(1)).WillRepeatedly(Return(ResourceUploaderMock::FakeResourceDeviceHandle));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res2);
        expectResourceUploaded(res3);
        expectResourceUploaded(res4);

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);
        makeResourceUnused(res4);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(4);
    }

    TEST_F(AResourceUploadingManager, doesNotReportKeptEffectAsPendingUnload)
    {
        const ResourceContentHash res(1234u, 0u);
        registerAndProvideResource(res, true);

        EXPECT_CALL(*uploader, uploadResource(_, _, _));
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res);

        makeResourceUnused(res);

        EXPECT_FALSE(rendererResourceUploader.hasAnythingToUpload());

        // destructor will unload kept effects
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(1u);
    }

    TEST_F(AResourceUploadingManager_WithVRAMCache, willUploadResourcesEvenIfExceedingCacheSize)
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
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(5u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        expectResourceUploaded(res1);
        expectResourceUploaded(res2);
        expectResourceUploaded(res3);
        expectResourceUploaded(res4);
        expectResourceUploaded(res5);

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);
        makeResourceUnused(res4);
        makeResourceUnused(res5);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(5);
    }

    TEST_F(AResourceUploadingManager_WithVRAMCache, willUnloadUnusedCachedResourcesIfCacheSizeExceeded)
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

        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(5u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        makeResourceUnused(res1);
        makeResourceUnused(res2);
        makeResourceUnused(res3);
        makeResourceUnused(res4);
        makeResourceUnused(res5);

        // unload anything exceeding cache limit
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(2u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        expectResourceUploaded(res3);
        expectResourceUploaded(res4);
        expectResourceUploaded(res5);
        Mock::VerifyAndClearExpectations(&uploader);

        // destructor will unload kept resources
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(3u);
    }

    TEST_F(AResourceUploadingManager_WithVRAMCache, willOnlyUnloadResourcesWhenNewResourcesAreToBeUploadedAndSoThatCacheSizeIsUsedAtMaximum)
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
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(2u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        makeResourceUnused(res2);

        // cache stays unchanged, 10/30 used resource, 10/30 unused resource, 10/30 available
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(0u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        registerAndProvideResource(res3);
        registerAndProvideResource(res4);

        // 20 bytes is needed, 10/30 is available right away, 10/30 needs to be freed
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(1u);
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(2u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        expectResourceUploaded(res1);
        expectResourceUploaded(res3);
        expectResourceUploaded(res4);
        Mock::VerifyAndClearExpectations(&uploader);

        makeResourceUnused(res1);
        makeResourceUnused(res3);
        makeResourceUnused(res4);

        // destructor will unload kept resources
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(3u);
    }

    TEST_F(AResourceUploadingManager_WithVRAMCache, willNotUnloadAnyCachedUnusedResourceIfThereIsEnoughRemainingCacheAvailable)
    {
        // test resource has size of 10 bytes
        // cache is set to 30 bytes

        const ResourceContentHash res1(1234u, 0u);
        const ResourceContentHash res2(1235u, 0u);
        const ResourceContentHash res3(1236u, 0u);

        registerAndProvideResource(res1);
        registerAndProvideResource(res2);

        // cache is 20/30 filled
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(2u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        makeResourceUnused(res2);

        // cache stays unchanged, 10/30 used resource, 10/30 unused resource, 10/30 available
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(0u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        registerAndProvideResource(res3);

        // 10 bytes is needed, 10/30 is available right away, nothing needs to be freed
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(0u);
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(1u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        expectResourceUploaded(res1);
        expectResourceUploaded(res2);
        expectResourceUploaded(res3);
        Mock::VerifyAndClearExpectations(&uploader);

        makeResourceUnused(res1);
        makeResourceUnused(res3);

        // destructor will unload kept resources
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(3u);
    }

    TEST_F(AResourceUploadingManager_WithVRAMCache, willUnloadResourcesOfSameOrGreaterSizeOfResourcesToBeUploaded_AlreadyUsingMoreThanCacheSize)
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
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(4u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        makeResourceUnused(res1);
        makeResourceUnused(res2);

        // unload unused resources exceeding cache limit
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(1u);
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res2);

        registerAndProvideResource(res5);

        // cache is full and exactly 1 unused cached resource will be freed for 1 new resource to be uploaded
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(1u);
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(1u);
        rendererResourceUploader.uploadAndUnloadPendingResources();

        expectResourceUploaded(res3);
        expectResourceUploaded(res4);
        expectResourceUploaded(res5);
        Mock::VerifyAndClearExpectations(&uploader);

        makeResourceUnused(res3);
        makeResourceUnused(res4);
        makeResourceUnused(res5);

        // destructor will unload kept resources
        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(3u);
    }

    TEST_F(AResourceUploadingManager_ScenePriority, uploadsPreferredResourcesFirst)
    {
        const std::vector<uint32_t> dummyData(ResourceUploadingManager::LargeResourceByteSizeThreshold / 4 + 1, 0u);
        const ArrayResource largeResource(EResourceType::IndexArray, static_cast<uint32_t>(dummyData.size()), EDataType::UInt32, dummyData.data(), "");

        // using large resources so that time budget checks happen on every resource, not just once per batch
        const ResourceContentHash res1(1234u, 0u);
        const ResourceContentHash res2(1235u, 0u);
        const ResourceContentHash res3(1236u, 0u);
        const ResourceContentHash res4(1237u, 0u);

        registerAndProvideResource(res1, false, &largeResource, getDeprivedScene());
        registerAndProvideResource(res2, false, &largeResource, getDeprivedScene());
        registerAndProvideResource(res3, false, &largeResource, getPreferredScene());
        registerAndProvideResource(res4, false, &largeResource);

        //set section time budget so the first resource upload will exceed it
        const uint32_t sectionTimeBudgetMillis = 10u;
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, sectionTimeBudgetMillis * 1000u);

        //make sure that only 1 resource will be uploaded
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(1).WillRepeatedly(InvokeWithoutArgs([&]() {PlatformThread::Sleep(12); return ResourceUploaderMock::FakeResourceDeviceHandle; }));
        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        // expect preferred resource uploaded
        expectResourceUploaded(res3);
        // expect others not uploaded
        expectResourceStatus(res1, EResourceStatus::Provided);
        expectResourceStatus(res2, EResourceStatus::Provided);
        expectResourceStatus(res4, EResourceStatus::Provided);

        //make sure that only 1 resource will be uploaded
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(1).WillRepeatedly(InvokeWithoutArgs([&]() {PlatformThread::Sleep(12); return ResourceUploaderMock::FakeResourceDeviceHandle; }));
        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        // expect preferred resource uploaded
        expectResourceUploaded(res4);
        // expect others not uploaded
        expectResourceStatus(res1, EResourceStatus::Provided);
        expectResourceStatus(res2, EResourceStatus::Provided);

        // upload rest
        frameTimer.setSectionTimeBudget(EFrameTimerSectionBudget::ResourcesUpload, std::numeric_limits<uint64_t>::max());
        EXPECT_CALL(*uploader, uploadResource(_, _, _)).Times(2).WillRepeatedly(Return(ResourceUploaderMock::FakeResourceDeviceHandle));
        frameTimer.startFrame();
        rendererResourceUploader.uploadAndUnloadPendingResources();
        expectResourceUploaded(res1);
        expectResourceUploaded(res2);

        makeResourceUnused(res1, getDeprivedScene());
        makeResourceUnused(res2, getDeprivedScene());
        makeResourceUnused(res3, getPreferredScene());
        makeResourceUnused(res4);

        EXPECT_CALL(*uploader, unloadResource(_, _, _, _)).Times(4);
    }

}
