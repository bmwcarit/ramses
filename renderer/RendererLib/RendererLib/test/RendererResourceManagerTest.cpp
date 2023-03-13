//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/RendererStatistics.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/EDataBufferType.h"
#include "SceneAPI/TextureEnums.h"
#include "ResourceDeviceHandleAccessorMock.h"
#include "RendererResourceCacheMock.h"
#include "RendererResourceCacheFake.h"
#include "PlatformMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include "MockResourceHash.h"
#include "ResourceUploaderMock.h"
#include "Watchdog/ThreadAliveNotifierMock.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal {
using namespace testing;

namespace
{
    DisplayConfig makeConfig(bool keepEffects)
    {
        DisplayConfig cfg;
        cfg.setKeepEffectsUploaded(keepEffects);
        return cfg;
    }
}

class ARendererResourceManager : public ::testing::Test
{
public:
    explicit ARendererResourceManager(bool keepEffects = false)
        : fakeSceneId(66u)
        , asyncEffectUploader(platform, platform.renderBackendMock, notifier, 1)
        , resourceManager(platform.renderBackendMock, std::unique_ptr<IResourceUploader>{ resUploader }, asyncEffectUploader, embeddedCompositingManager, makeConfig(keepEffects), frameTimer, stats)
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);

        InSequence s;
        EXPECT_CALL(platform.renderBackendMock.contextMock, disable()).WillOnce(Return(true));
        EXPECT_CALL(platform, createResourceUploadRenderBackend());
        EXPECT_CALL(platform.renderBackendMock.contextMock, enable()).WillOnce(Return(true));
        const bool status = asyncEffectUploader.createResourceUploadRenderBackendAndStartThread();
        EXPECT_TRUE(status);
    }

    ~ARendererResourceManager()
    {
        // no actual unload expected but clears internal lists
        resourceManager.unloadAllSceneResourcesForScene(fakeSceneId);

        EXPECT_CALL(platform, destroyResourceUploadRenderBackend());
        asyncEffectUploader.destroyResourceUploadRenderBackendAndStopThread();
    }

    void referenceResource(ResourceContentHash hash, SceneId sceneId)
    {
        const ResourceContentHashVector list{ hash };

        resourceManager.referenceResourcesForScene(sceneId, list);
        EXPECT_TRUE(contains_c(*resourceManager.getResourcesInUseByScene(sceneId), hash));
    }

    void unreferenceResource(ResourceContentHash hash, SceneId sceneId)
    {
        const ResourceContentHashVector list{ hash };

        resourceManager.unreferenceResourcesForScene(sceneId, list);
        EXPECT_TRUE(!resourceManager.getResourcesInUseByScene(sceneId) || !contains_c(*resourceManager.getResourcesInUseByScene(sceneId), hash));
    }

    void expectStreamUsedBy(WaylandIviSurfaceId source, const std::vector<StreamBufferHandle>& sbUsage)
    {
        EXPECT_THAT(sbUsage, ::testing::UnorderedElementsAreArray(resourceManager.getStreamUsage(source)));
    }

    void expectResourceUploaded(const ResourceContentHash& hash, EResourceType type, DeviceResourceHandle resultDeviceHandle = ResourceUploaderMock::FakeResourceDeviceHandle)
    {
        EXPECT_CALL(*resUploader, uploadResource(Ref(platform.renderBackendMock), _, _)).WillOnce(Invoke([hash, type, resultDeviceHandle](auto&, const auto& rd, auto&) {
                EXPECT_EQ(hash, rd.hash);
                EXPECT_EQ(type, rd.type);

                return resultDeviceHandle;
            }));
    }

    void expectResourceUnloaded(const ResourceContentHash& hash, EResourceType type, DeviceResourceHandle deviceHandle = ResourceUploaderMock::FakeResourceDeviceHandle)
    {
        EXPECT_CALL(*resUploader, unloadResource(Ref(platform.renderBackendMock), type, hash , deviceHandle));
    }

    void expectDeviceFlushOnWindows()
    {
#if defined(_WIN32)
        EXPECT_CALL(platform.resourceUploadRenderBackendMock.deviceMock, flush());
#endif
    }

    void uploadShader(const ResourceContentHash& hash, bool expectSuccess = true)
    {
        EXPECT_CALL(*resUploader, uploadResource(Ref(platform.renderBackendMock), _, _)).WillOnce(Invoke([hash](auto&, const auto& rd, auto&) {
                EXPECT_EQ(hash, rd.hash);
                EXPECT_EQ(EResourceType_Effect, rd.type);

                return std::optional<DeviceResourceHandle>{};
            }));

        if(expectSuccess)
        {
            EXPECT_CALL(platform.resourceUploadRenderBackendMock.deviceMock, uploadShader(_));
            expectDeviceFlushOnWindows();
            EXPECT_CALL(platform.renderBackendMock.deviceMock, registerShader(_));
            EXPECT_CALL(*resUploader, storeShaderInBinaryShaderCache(Ref(platform.renderBackendMock), _, _, _));
        }
        else
        {
            EXPECT_CALL(platform.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).WillRepeatedly(Invoke([](const auto&) {return std::move(std::unique_ptr<const GPUResource>{}); }));
            expectDeviceFlushOnWindows();
            EXPECT_CALL(platform.renderBackendMock.deviceMock, registerShader(_)).Times(0);
            EXPECT_CALL(*resUploader, storeShaderInBinaryShaderCache(Ref(platform.renderBackendMock), _, _, _)).Times(0);
        }

        resourceManager.uploadAndUnloadPendingResources();
        ASSERT_EQ(EResourceStatus::ScheduledForUpload, resourceManager.getResourceStatus(hash));

        constexpr std::chrono::seconds timeoutTime{ 2u };
        const auto startTime = std::chrono::steady_clock::now();
        while (resourceManager.getResourceStatus(hash) == EResourceStatus::ScheduledForUpload
            && std::chrono::steady_clock::now() - startTime < timeoutTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 5u });
            resourceManager.uploadAndUnloadPendingResources();
        }
    }

protected:
    StrictMock<PlatformStrictMock> platform;
    StrictMock<RenderBackendStrictMock> additionalRenderer;
    StrictMock<EmbeddedCompositingManagerMock> embeddedCompositingManager;
    SceneId fakeSceneId;
    RendererStatistics stats;
    StrictMock<ResourceUploaderMock>* resUploader = new StrictMock<ResourceUploaderMock>;
    FrameTimer frameTimer;
    NiceMock<ThreadAliveNotifierMock> notifier;
    AsyncEffectUploader asyncEffectUploader;
    RendererResourceManager resourceManager;
};

TEST_F(ARendererResourceManager, PerformsCorrectlyResourceLifecycle)
{
    constexpr ResourceContentHash resource = MockResourceHash::VertArrayHash;

    // register some resource
    referenceResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getResourceDeviceHandle(resource).isValid());

    // provide data
    const auto managedRes = MockResourceHash::GetManagedResource(resource);
    resourceManager.provideResourceData(managedRes);
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getResourceDeviceHandle(resource).isValid());

    // upload the resource
    expectResourceUploaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(resource).isValid());
    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());

    // delete resource
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.unreferenceResourcesForScene(fakeSceneId, { resource });
    expectResourceUnloaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&platform.renderBackendMock);
}

TEST_F(ARendererResourceManager, ReferencedResourceHasCorrectStatus)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;

    referenceResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getResourceDeviceHandle(resource).isValid());

    unreferenceResource(resource, fakeSceneId);
}

TEST_F(ARendererResourceManager, ReferencedAndProvidedResourceHasCorrectStatus)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;

    referenceResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource));
    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_FALSE(resourceManager.getResourceDeviceHandle(resource).isValid());

    const auto managedRes = MockResourceHash::GetManagedResource(resource);
    resourceManager.provideResourceData(managedRes);
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getResourceDeviceHandle(resource).isValid());

    unreferenceResource(resource, fakeSceneId);
}

TEST_F(ARendererResourceManager, unreferencingResourceThatWasNotUploadedYetWillNotReportItAsPendingForUploadAnymore)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;

    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(resource));

    unreferenceResource(resource, fakeSceneId);
    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());
}

TEST_F(ARendererResourceManager, DeletesResourceUsedBySingleSceneWhenUnreferenced)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;

    // request some resource
    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    expectResourceUploaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(resource).isValid());

    resourceManager.unreferenceResourcesForScene(fakeSceneId, { resource });
    expectResourceUnloaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&platform.renderBackendMock);
}

TEST_F(ARendererResourceManager, referencingAnUnloadedResourceAgainShouldUploadItAgain)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;
    const auto managedRes = MockResourceHash::GetManagedResource(resource);

    // ref
    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(managedRes);
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    expectResourceUploaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();

    // unref
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.unreferenceResourcesForScene(fakeSceneId, resources);
    expectResourceUnloaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();

    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());

    Mock::VerifyAndClearExpectations(&platform.renderBackendMock);

    // ref again
    referenceResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource));
    resourceManager.provideResourceData(managedRes);
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    expectResourceUploaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();

    //general clean-up (expect needed because of strict mock)
    unreferenceResource(resource, fakeSceneId);
    expectResourceUnloaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();
}

TEST_F(ARendererResourceManager, deletesNoLongerNeededResourcesWhenSceneDestroyed)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;

    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    expectResourceUploaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();

    ResourceContentHashVector usedResources;
    usedResources.push_back(resource);
    resourceManager.unreferenceResourcesForScene(fakeSceneId, usedResources);
    expectResourceUnloaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&platform.renderBackendMock);
}

TEST_F(ARendererResourceManager, keepsTrackOfStreamUsageByStreamBuffers)
{
    constexpr WaylandIviSurfaceId source{ 666u };
    constexpr WaylandIviSurfaceId source2{ 667u };
    constexpr SceneId scene1{ 13u };
    constexpr SceneId scene2{ 14u };
    constexpr StreamBufferHandle sb1{ 21u };
    constexpr StreamBufferHandle sb2{ 22u };
    constexpr StreamBufferHandle sb3{ 23u };

    EXPECT_CALL(embeddedCompositingManager, refStream(source)).Times(2);
    EXPECT_CALL(embeddedCompositingManager, refStream(source2));
    resourceManager.uploadStreamBuffer(sb1, source);
    resourceManager.uploadStreamBuffer(sb2, source);
    resourceManager.uploadStreamBuffer(sb3, source2);
    expectStreamUsedBy(source, { sb1, sb2 });
    expectStreamUsedBy(source2, { sb3 });

    EXPECT_CALL(embeddedCompositingManager, unrefStream(source)).Times(2);
    EXPECT_CALL(embeddedCompositingManager, unrefStream(source2));
    resourceManager.unloadStreamBuffer(sb1);
    resourceManager.unloadStreamBuffer(sb2);
    resourceManager.unloadStreamBuffer(sb3);
    expectStreamUsedBy(source, {});
    expectStreamUsedBy(source2, {});

    resourceManager.unloadAllSceneResourcesForScene(scene1);
    resourceManager.unloadAllSceneResourcesForScene(scene2);
}

TEST_F(ARendererResourceManager, doesNotDeleteResourcesNeededByAnotherSceneWhenSceneDestroyed)
{
    ResourceContentHash vertResource = MockResourceHash::VertArrayHash;
    ResourceContentHash indexResource = MockResourceHash::IndexArrayHash;

    const DeviceResourceHandle vertDeviceHandle{ 111u };
    const DeviceResourceHandle indexDeviceHandle{ 222u };

    const SceneId fakeSceneId2(4u);

    //index array is also needed by scene 2
    referenceResource(vertResource, fakeSceneId);
    referenceResource(indexResource, fakeSceneId);
    referenceResource(indexResource, fakeSceneId2);

    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(vertResource));
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(indexResource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    InSequence s;
    expectResourceUploaded(vertResource, EResourceType_VertexArray, vertDeviceHandle);
    expectResourceUploaded(indexResource, EResourceType_IndexArray, indexDeviceHandle);
    resourceManager.uploadAndUnloadPendingResources();

    resourceManager.unreferenceResourcesForScene(fakeSceneId, { vertResource, indexResource });
    expectResourceUnloaded(vertResource, EResourceType_VertexArray, vertDeviceHandle);
    resourceManager.uploadAndUnloadPendingResources();

    Mock::VerifyAndClearExpectations(&platform.renderBackendMock);

    //general clean-up (expect needed because of strict mock)
    unreferenceResource(indexResource, fakeSceneId2);
    expectResourceUnloaded(indexResource, EResourceType_IndexArray, indexDeviceHandle);
    resourceManager.uploadAndUnloadPendingResources();
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadDataBuffer_IndexBuffer)
{
    const DataBufferHandle dataBuffer(1u);
    const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
    const EDataType dataType = EDataType::UInt32;
    const UInt32 sizeInBytes = 1024u;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateIndexBuffer(dataType, sizeInBytes));
    resourceManager.uploadDataBuffer(dataBuffer, dataBufferType, dataType, sizeInBytes, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeIndexBufferDeviceHandle, resourceManager.getDataBufferDeviceHandle(dataBuffer, fakeSceneId));

    const Byte dummyData[10] = {};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadIndexBufferData(DeviceMock::FakeIndexBufferDeviceHandle, dummyData, 7u));
    resourceManager.updateDataBuffer(dataBuffer, 7u, dummyData, fakeSceneId);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteIndexBuffer(DeviceMock::FakeIndexBufferDeviceHandle));
    resourceManager.unloadDataBuffer(dataBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadDataBuffer_VertexBuffer)
{
    const DataBufferHandle dataBuffer(1u);
    const EDataBufferType dataBufferType = EDataBufferType::VertexBuffer;
    const EDataType dataType = EDataType::UInt32;
    const UInt32 sizeInBytes = 1024u;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateVertexBuffer(sizeInBytes));
    resourceManager.uploadDataBuffer(dataBuffer, dataBufferType, dataType, sizeInBytes, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeVertexBufferDeviceHandle, resourceManager.getDataBufferDeviceHandle(dataBuffer, fakeSceneId));

    const Byte dummyData[10] = {};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadVertexBufferData(DeviceMock::FakeVertexBufferDeviceHandle, dummyData, 7u));
    resourceManager.updateDataBuffer(dataBuffer, 7u, dummyData, fakeSceneId);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteVertexBuffer(DeviceMock::FakeVertexBufferDeviceHandle));
    resourceManager.unloadDataBuffer(dataBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadTextureBuffer_WithOneMipLevel)
{
    InSequence seq;
    const TextureBufferHandle textureBuffer(1u);
    const UInt32 expectedSize = (10u * 20u) * 4u;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateTexture2D(10u, 20u, ETextureFormat::RGBA8, DefaultTextureSwizzleArray, 1u, expectedSize));
    resourceManager.uploadTextureBuffer(textureBuffer, 10u, 20u, ETextureFormat::RGBA8, 1u, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeTextureDeviceHandle, resourceManager.getTextureBufferDeviceHandle(textureBuffer, fakeSceneId));

    const Byte dummyTexture[10] = {};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, bindTexture(DeviceMock::FakeTextureDeviceHandle));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadTextureData(DeviceMock::FakeTextureDeviceHandle, 0u, 2u, 3u, 0u, 4u, 5u, 1u, dummyTexture, 0u));
    resourceManager.updateTextureBuffer(textureBuffer, 0u, 2u, 3u, 4u, 5u, dummyTexture, fakeSceneId);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteTexture(DeviceMock::FakeTextureDeviceHandle));
    resourceManager.unloadTextureBuffer(textureBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadTextureBuffer_WithSeveralMipLevels)
{
    InSequence seq;
    const TextureBufferHandle textureBuffer(1u);
    const UInt32 expectedSize = (10u * 20u + 5u * 10u)* 4u;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateTexture2D(10u, 20u, ETextureFormat::RGBA8, DefaultTextureSwizzleArray, 2u, expectedSize));
    resourceManager.uploadTextureBuffer(textureBuffer, 10u, 20u, ETextureFormat::RGBA8, 2u, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeTextureDeviceHandle, resourceManager.getTextureBufferDeviceHandle(textureBuffer, fakeSceneId));

    const Byte dummyTexture[10] = {};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, bindTexture(DeviceMock::FakeTextureDeviceHandle));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadTextureData(DeviceMock::FakeTextureDeviceHandle, 1u, 2u, 3u, 0u, 4u, 5u, 1u, _, 0u));
    resourceManager.updateTextureBuffer(textureBuffer, 1u, 2u, 3u, 4u, 5u, dummyTexture, fakeSceneId);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteTexture(DeviceMock::FakeTextureDeviceHandle));
    resourceManager.unloadTextureBuffer(textureBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUnloadRenderTargetBuffer)
{
    RenderBufferHandle bufferHandle(1u);
    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));
    resourceManager.uploadRenderTargetBuffer(bufferHandle, fakeSceneId, colorBuffer);

    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getRenderTargetBufferDeviceHandle(bufferHandle, fakeSceneId));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_));
    resourceManager.unloadRenderTargetBuffer(bufferHandle, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUnloadBlitPassRenderTargets)
{
    const BlitPassHandle blitPassHandle(100u);
    const RenderBufferHandle sourceRenderBufferHandle(101u);
    const RenderBufferHandle destinationRenderBufferHandle(102u);

    const RenderBuffer renderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);

    const DeviceResourceHandle sourceRenderBufferDeviceHandle(201u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock,
        uploadRenderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u))
        .WillOnce(Return(sourceRenderBufferDeviceHandle));
    resourceManager.uploadRenderTargetBuffer(sourceRenderBufferHandle, fakeSceneId, renderBuffer);

    const DeviceResourceHandle destinationRenderBufferDeviceHandle(202u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock,
        uploadRenderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u))
        .WillOnce(Return(destinationRenderBufferDeviceHandle));
    resourceManager.uploadRenderTargetBuffer(destinationRenderBufferHandle, fakeSceneId, renderBuffer);

    const DeviceResourceHandle blittingRenderTargetSrcDeviceHandle(203u);
    const DeviceResourceHandle blittingRenderTargetDstDeviceHandle(204u);
    DeviceHandleVector rbsSrc;
    rbsSrc.push_back(sourceRenderBufferDeviceHandle);
    DeviceHandleVector rbsDst;
    rbsDst.push_back(destinationRenderBufferDeviceHandle);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(Eq(rbsSrc))).WillOnce(Return(blittingRenderTargetSrcDeviceHandle));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(Eq(rbsDst))).WillOnce(Return(blittingRenderTargetDstDeviceHandle));
    resourceManager.uploadBlitPassRenderTargets(blitPassHandle, sourceRenderBufferHandle, destinationRenderBufferHandle, fakeSceneId);

    DeviceResourceHandle actualBlitPassRTSrc;
    DeviceResourceHandle actualBlitPassRTDst;
    resourceManager.getBlitPassRenderTargetsDeviceHandle(blitPassHandle, fakeSceneId, actualBlitPassRTSrc, actualBlitPassRTDst);
    EXPECT_EQ(blittingRenderTargetSrcDeviceHandle, actualBlitPassRTSrc);
    EXPECT_EQ(blittingRenderTargetDstDeviceHandle, actualBlitPassRTDst);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(actualBlitPassRTSrc));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(actualBlitPassRTDst));
    resourceManager.unloadBlitPassRenderTargets(blitPassHandle, fakeSceneId);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2);
    resourceManager.unloadRenderTargetBuffer(sourceRenderBufferHandle, fakeSceneId);
    resourceManager.unloadRenderTargetBuffer(destinationRenderBufferHandle, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUnloadRenderTargetWithBuffers)
{
    RenderTargetHandle targetHandle(1u);

    RenderBufferHandleVector bufferHandles;
    bufferHandles.push_back(RenderBufferHandle(1u));
    bufferHandles.push_back(RenderBufferHandle(5u));

    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthBuffer(800u, 600u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_ReadWrite, 0u);

    {
        InSequence seq;
        EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));
        EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(800u, 600u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_ReadWrite, 0u));
        EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    }

    resourceManager.uploadRenderTargetBuffer(bufferHandles[0], fakeSceneId, colorBuffer);
    resourceManager.uploadRenderTargetBuffer(bufferHandles[1], fakeSceneId, depthBuffer);

    resourceManager.uploadRenderTarget(targetHandle, bufferHandles, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getRenderTargetBufferDeviceHandle(bufferHandles[0], fakeSceneId));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getRenderTargetBufferDeviceHandle(bufferHandles[1], fakeSceneId));
    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getRenderTargetDeviceHandle(targetHandle, fakeSceneId));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2);
    resourceManager.unloadRenderTargetBuffer(bufferHandles[0], fakeSceneId);
    resourceManager.unloadRenderTargetBuffer(bufferHandles[1], fakeSceneId);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    resourceManager.unloadRenderTarget(targetHandle, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUnloadVertexArray)
{
    VertexArrayInfo vaInfo;
    vaInfo.shader = DeviceMock::FakeShaderDeviceHandle;
    vaInfo.indexBuffer = DeviceMock::FakeIndexBufferDeviceHandle;
    vaInfo.vertexBuffers.push_back({ DeviceMock::FakeVertexBufferDeviceHandle,DataFieldHandle{2u}, 3u, 4u, EDataType::Vector2F, 5u, 6u });

    const RenderableHandle renderable{ 123u };
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateVertexArray(Ref(vaInfo)));
    resourceManager.uploadVertexArray(renderable, vaInfo, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeVertexArrayDeviceHandle, resourceManager.getVertexArrayDeviceHandle(renderable, fakeSceneId));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteVertexArray(DeviceMock::FakeVertexArrayDeviceHandle));
    resourceManager.unloadVertexArray(renderable, fakeSceneId);
}

TEST_F(ARendererResourceManager, GetsInvalidDeviceHandleForUnknownOffscreenBuffer)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
    EXPECT_FALSE(resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle).isValid());
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorBuffer)
{
    const OffscreenBufferHandle bufferHandle(1u);

    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, false, ERenderBufferType_InvalidBuffer);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_));
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorAndDepthBuffers)
{
    const OffscreenBufferHandle bufferHandle(1u);
    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_WriteOnly, 0u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, false, ERenderBufferType_DepthBuffer);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2u);
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorAndDepthStencilAttached)
{
    const OffscreenBufferHandle bufferHandle(1u);
    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat::Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 0u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2u);
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorAndDepthStencilAttachedWithMSAAenabled)
{
    const OffscreenBufferHandle bufferHandle(1u);
    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 4u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat::Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 4u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 4u, false, ERenderBufferType_DepthStencilBuffer);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2u);
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorBuffer_WithDoubleBuffering)
{
    const OffscreenBufferHandle bufferHandle(1u);
    const DeviceResourceHandle colorBufferDeviceHandle1{ 7771u };
    const DeviceResourceHandle colorBufferDeviceHandle2{ 7778u };
    const DeviceResourceHandle offscreenBufferDeviceHandle1{ 7798u };
    const DeviceResourceHandle offscreenBufferDeviceHandle2{ 7799u };
    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u))
        .WillOnce(Return(colorBufferDeviceHandle1));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(Eq(DeviceHandleVector({ colorBufferDeviceHandle1 })))).WillOnce(Return(offscreenBufferDeviceHandle1));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u))
        .WillOnce(Return(colorBufferDeviceHandle2));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(Eq(DeviceHandleVector({ colorBufferDeviceHandle2 })))).WillOnce(Return(offscreenBufferDeviceHandle2));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(offscreenBufferDeviceHandle1));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(offscreenBufferDeviceHandle2));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, true, ERenderBufferType_InvalidBuffer);

    EXPECT_EQ(offscreenBufferDeviceHandle1, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(colorBufferDeviceHandle1, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(offscreenBufferDeviceHandle1));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_)).Times(2);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2u);
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorAndDepthStencilBuffers_WithDoubleBuffering)
{
    const OffscreenBufferHandle bufferHandle(1u);
    const DeviceResourceHandle colorBufferDeviceHandle1{ 7771u };
    const DeviceResourceHandle colorBufferDeviceHandle2{ 7778u };
    const DeviceResourceHandle depthBufferDeviceHandle{ 7796u };
    const DeviceResourceHandle offscreenBufferDeviceHandle1{ 7798u };
    const DeviceResourceHandle offscreenBufferDeviceHandle2{ 7799u };
    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u))
        .WillOnce(Return(colorBufferDeviceHandle1));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat::Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 0u))
        .WillOnce(Return(depthBufferDeviceHandle));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(Eq(DeviceHandleVector({colorBufferDeviceHandle1, depthBufferDeviceHandle})))).WillOnce(Return(offscreenBufferDeviceHandle1));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u))
        .WillOnce(Return(colorBufferDeviceHandle2));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(Eq(DeviceHandleVector({ colorBufferDeviceHandle2, depthBufferDeviceHandle })))).WillOnce(Return(offscreenBufferDeviceHandle2));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(offscreenBufferDeviceHandle1));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(offscreenBufferDeviceHandle2));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, true, ERenderBufferType_DepthStencilBuffer);

    EXPECT_EQ(offscreenBufferDeviceHandle1, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(colorBufferDeviceHandle1, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(offscreenBufferDeviceHandle1));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_)).Times(2);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(3u);
}

TEST_F(ARendererResourceManager, UploadsDmaOffscreenBuffer)
{
    constexpr OffscreenBufferHandle bufferHandle(1u);

    constexpr DmaBufferFourccFormat bufferFormat{ 777u };
    constexpr DmaBufferUsageFlags bufferUsageFlags{ 888u };
    constexpr DmaBufferModifiers bufferModifiers{ 999u };
    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadDmaRenderBuffer(10u, 20u, bufferFormat, bufferUsageFlags, bufferModifiers));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadDmaOffscreenBuffer(bufferHandle, 10u, 20u, bufferFormat, bufferUsageFlags, bufferModifiers);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeDmaRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, getDmaRenderBufferFD(DeviceMock::FakeDmaRenderBufferDeviceHandle)).WillOnce(Return(123));
    EXPECT_EQ(123, resourceManager.getDmaOffscreenBufferFD(bufferHandle));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, getDmaRenderBufferStride(DeviceMock::FakeDmaRenderBufferDeviceHandle)).WillOnce(Return(432u));
    EXPECT_EQ(432u, resourceManager.getDmaOffscreenBufferStride(bufferHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, destroyDmaRenderBuffer(_));
}

TEST_F(ARendererResourceManager, CanUnloadOffscreenBuffer_WithColorBuffer)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(_, _, _, _, _, _));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, false, ERenderBufferType_InvalidBuffer);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_));
    resourceManager.unloadOffscreenBuffer(bufferHandle);

    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
}

TEST_F(ARendererResourceManager, CanUnloadOffscreenBuffer_WithColorAndDepthStencilBuffers)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(_, _, _, _, _, _)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2u);
    resourceManager.unloadOffscreenBuffer(bufferHandle);

    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
}

TEST_F(ARendererResourceManager, CanUnloadDoubleBufferedOffscreenBuffer_WithColorBuffer)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(_, _, _, _, _, _)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f })).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled)).Times(2u);
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, true, ERenderBufferType_InvalidBuffer);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2u);
    resourceManager.unloadOffscreenBuffer(bufferHandle);

    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
}

TEST_F(ARendererResourceManager, CanUnloadDoubleBufferedOffscreenBuffer_WithColorAndDepthStencilBuffers)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(_, _, _, _, _, _)).Times(3u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f })).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled)).Times(2u);
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, true, ERenderBufferType_DepthStencilBuffer);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(3u);
    resourceManager.unloadOffscreenBuffer(bufferHandle);

    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
}

TEST_F(ARendererResourceManager, CanUnloadDmaOffscreenBuffer)
{
    constexpr OffscreenBufferHandle bufferHandle(1u);

    constexpr DmaBufferFourccFormat bufferFormat{ 777u };
    constexpr DmaBufferUsageFlags bufferUsageFlags{ 888u };
    constexpr DmaBufferModifiers bufferModifiers{ 999u };
    InSequence seq;
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadDmaRenderBuffer(10u, 20u, bufferFormat, bufferUsageFlags, bufferModifiers));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_));
    resourceManager.uploadDmaOffscreenBuffer(bufferHandle, 10u, 20u, bufferFormat, bufferUsageFlags, bufferModifiers);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeDmaRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, destroyDmaRenderBuffer(_));
    resourceManager.unloadOffscreenBuffer(bufferHandle);
    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
}

TEST_F(ARendererResourceManager, UploadsAndUnloadsStreamBuffers)
{
    constexpr StreamBufferHandle sbHandle1{ 1u };
    constexpr StreamBufferHandle sbHandle2{ 2u };
    constexpr WaylandIviSurfaceId sbSrc1{ 11u };
    constexpr WaylandIviSurfaceId sbSrc2{ 12u };
    EXPECT_CALL(embeddedCompositingManager, refStream(sbSrc1));
    resourceManager.uploadStreamBuffer(sbHandle1, sbSrc1);
    EXPECT_CALL(embeddedCompositingManager, refStream(sbSrc2));
    resourceManager.uploadStreamBuffer(sbHandle2, sbSrc2);

    EXPECT_CALL(embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(sbSrc1)).WillOnce(Return(DeviceMock::FakeRenderTargetDeviceHandle));
    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getStreamBufferDeviceHandle(sbHandle1));
    EXPECT_CALL(embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(sbSrc2)).WillOnce(Return(DeviceMock::FakeRenderBufferDeviceHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getStreamBufferDeviceHandle(sbHandle2));

    EXPECT_CALL(embeddedCompositingManager, unrefStream(sbSrc1));
    resourceManager.unloadStreamBuffer(sbHandle1);
    EXPECT_CALL(embeddedCompositingManager, unrefStream(sbSrc2));
    resourceManager.unloadStreamBuffer(sbHandle2);
}

TEST_F(ARendererResourceManager, returnsInvalidDeviceHandleForUnknownStreamBuffer)
{
    EXPECT_FALSE(resourceManager.getStreamBufferDeviceHandle(StreamBufferHandle::Invalid()).isValid());
    EXPECT_FALSE(resourceManager.getStreamBufferDeviceHandle(StreamBufferHandle{ 9999u }).isValid());
}

TEST_F(ARendererResourceManager, UploadsAndUnloadsExternalBuffers)
{
    constexpr ExternalBufferHandle ebHandle1{ 1u };
    constexpr ExternalBufferHandle ebHandle2{ 2u };

    constexpr DeviceResourceHandle deviceHandle1{ 11111u };
    constexpr DeviceResourceHandle deviceHandle2{ 22222u };
    EXPECT_CALL(platform.renderBackendMock.deviceMock, isExternalTextureExtensionSupported()).WillOnce(Return(true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateExternalTexture()).WillOnce(Return(deviceHandle1));
    resourceManager.uploadExternalBuffer(ebHandle1);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, isExternalTextureExtensionSupported()).WillOnce(Return(true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateExternalTexture()).WillOnce(Return(deviceHandle2));
    resourceManager.uploadExternalBuffer(ebHandle2);

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteTexture(deviceHandle1));
    resourceManager.unloadExternalBuffer(ebHandle1);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteTexture(deviceHandle2));
    resourceManager.unloadExternalBuffer(ebHandle2);
}

TEST_F(ARendererResourceManager, FailsUploadingExternalBuffersIfExtensionNotSupported)
{
    constexpr ExternalBufferHandle ebHandle{ 1u };

    EXPECT_CALL(platform.renderBackendMock.deviceMock, isExternalTextureExtensionSupported()).WillOnce(Return(false));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateExternalTexture()).Times(0);
    resourceManager.uploadExternalBuffer(ebHandle);
}

TEST_F(ARendererResourceManager, CanGetDeviceHandleForExternalBuffer)
{
    constexpr ExternalBufferHandle ebHandle{ 1u };

    constexpr DeviceResourceHandle deviceHandle{ 11111u };
    EXPECT_CALL(platform.renderBackendMock.deviceMock, isExternalTextureExtensionSupported()).WillOnce(Return(true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateExternalTexture()).WillOnce(Return(deviceHandle));
    resourceManager.uploadExternalBuffer(ebHandle);

    EXPECT_EQ(deviceHandle, resourceManager.getExternalBufferDeviceHandle(ebHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteTexture(deviceHandle));
    resourceManager.unloadExternalBuffer(ebHandle);
}

TEST_F(ARendererResourceManager, ReturnsInvalidDeviceHandleForUnknownExternalBuffer)
{
    constexpr ExternalBufferHandle ebHandle{ 1u };
    EXPECT_FALSE(resourceManager.getExternalBufferDeviceHandle(ebHandle).isValid());
}

TEST_F(ARendererResourceManager, CanGetGlIdForExternalBuffer)
{
    constexpr ExternalBufferHandle ebHandle{ 1u };

    constexpr DeviceResourceHandle deviceHandle{ 11111u };
    EXPECT_CALL(platform.renderBackendMock.deviceMock, isExternalTextureExtensionSupported()).WillOnce(Return(true));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateExternalTexture()).WillOnce(Return(deviceHandle));
    resourceManager.uploadExternalBuffer(ebHandle);

    uint32_t glId{ 567u };
    EXPECT_CALL(platform.renderBackendMock.deviceMock, getTextureAddress(deviceHandle)).WillOnce(Return(glId));
    EXPECT_EQ(glId, resourceManager.getExternalBufferGlId(ebHandle));

    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteTexture(deviceHandle));
    resourceManager.unloadExternalBuffer(ebHandle);
}

TEST_F(ARendererResourceManager, ReturnsInvalidGlIdForUnknownExternalBuffer)
{
    constexpr ExternalBufferHandle ebHandle{ 1u };
    EXPECT_EQ(0u, resourceManager.getExternalBufferGlId(ebHandle));
}

TEST_F(ARendererResourceManager, UploadAndDeleteValidShader)
{
    auto effectRes = std::make_unique<const EffectResource>("", "", "", EDrawMode::NUMBER_OF_ELEMENTS, EffectInputInformationVector{}, EffectInputInformationVector{}, "", ResourceCacheFlag_DoNotCache);
    const ResourceContentHash resHash = effectRes->getHash();

    // request some resources
    referenceResource(resHash, fakeSceneId);
    resourceManager.provideResourceData(std::move(effectRes));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    uploadShader(resHash);

    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resHash));
    unreferenceResource(resHash, fakeSceneId);
    expectResourceUnloaded(resHash, EResourceType_Effect, DeviceMock::FakeShaderDeviceHandle);
    resourceManager.uploadAndUnloadPendingResources();
}

TEST_F(ARendererResourceManager, DoesNotUnregisterResourceThatWasUploaded)
{
    const ResourceContentHash resource = MockResourceHash::VertArrayHash;

    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    expectResourceUploaded(resource, EResourceType_VertexArray);
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(resource).isValid());

    resourceManager.unreferenceResourcesForScene(fakeSceneId, { resource });
    ASSERT_TRUE(resourceManager.getRendererResourceRegistry().containsResource(resource));

    expectResourceUnloaded(resource, EResourceType_VertexArray);
}

TEST_F(ARendererResourceManager, DoesNotUnregisterResourceThatWasScheduledForUpload)
{
    auto effectRes = std::make_unique<const EffectResource>("", "", "", EDrawMode::NUMBER_OF_ELEMENTS, EffectInputInformationVector{}, EffectInputInformationVector{}, "", ResourceCacheFlag_DoNotCache);
    const ResourceContentHash resHash = effectRes->getHash();

    // request some resources
    referenceResource(resHash, fakeSceneId);
    resourceManager.provideResourceData(std::move(effectRes));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    //simulate shader was not uploaded by ResourceUploader (not found in bin shader cache) so it gets uploaded by AsyncEffectUploader
    EXPECT_CALL(*resUploader, uploadResource(Ref(platform.renderBackendMock), _, _)).WillOnce(Invoke([](auto&, const auto&, auto&) {
        return std::optional<DeviceResourceHandle>{};
        }));

    EXPECT_CALL(platform.resourceUploadRenderBackendMock.deviceMock, uploadShader(_));
    expectDeviceFlushOnWindows();
    EXPECT_CALL(platform.renderBackendMock.deviceMock, registerShader(_));
    EXPECT_CALL(*resUploader, storeShaderInBinaryShaderCache(Ref(platform.renderBackendMock), _, _, _));
    resourceManager.uploadAndUnloadPendingResources();
    ASSERT_EQ(EResourceStatus::ScheduledForUpload, resourceManager.getResourceStatus(resHash));

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
    ASSERT_TRUE(resourceManager.getRendererResourceRegistry().containsResource(resHash));

    constexpr std::chrono::seconds timeoutTime{ 2u };
    const auto startTime = std::chrono::steady_clock::now();
    while (resourceManager.getResourceStatus(resHash) == EResourceStatus::ScheduledForUpload
        && std::chrono::steady_clock::now() - startTime < timeoutTime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 5u });
        resourceManager.uploadAndUnloadPendingResources();
    }

    expectResourceUnloaded(resHash, EResourceType_Effect, DeviceMock::FakeShaderDeviceHandle);
}

TEST_F(ARendererResourceManager, CanUploadAndUnloadEffectOwnedBySceneThatGetsDestroyed_ConfidenceTest)
{
    auto effectRes = std::make_unique<const EffectResource>("", "", "", EDrawMode::NUMBER_OF_ELEMENTS, EffectInputInformationVector{}, EffectInputInformationVector{}, "", ResourceCacheFlag_DoNotCache);
    const ResourceContentHash resHash = effectRes->getHash();

    // request some resources
    referenceResource(resHash, fakeSceneId);
    resourceManager.provideResourceData(std::move(effectRes));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    //simulate shader was not uploaded by ResourceUploader (not found in bin shader cache) so it gets uploaded by AsyncEffectUploader
    EXPECT_CALL(*resUploader, uploadResource(Ref(platform.renderBackendMock), _, _)).WillOnce(Invoke([resHash](auto&, const auto& rd, auto&) {
        EXPECT_EQ(resHash, rd.hash);
        EXPECT_EQ(EResourceType_Effect, rd.type);

        return std::optional<DeviceResourceHandle>{};
        }));

    //block call to upload shader to simulate scene getting unreferenced while AsyncEffectUploader is still uploading
    std::promise<void> barrier;
    auto future = barrier.get_future();
    EXPECT_CALL(platform.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).WillOnce(Invoke([&](const auto&) {
        future.get();
        return std::make_unique<const GPUResource>(1u, 2u);
        }));
    expectDeviceFlushOnWindows();
    EXPECT_CALL(platform.renderBackendMock.deviceMock, registerShader(_));
    EXPECT_CALL(*resUploader, storeShaderInBinaryShaderCache(_, _, _, _));

    resourceManager.uploadAndUnloadPendingResources();
    ASSERT_EQ(EResourceStatus::ScheduledForUpload, resourceManager.getResourceStatus(resHash));

    expectResourceUnloaded(resHash, EResourceType_Effect, DeviceMock::FakeShaderDeviceHandle);
    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
    ASSERT_TRUE(resourceManager.getRendererResourceRegistry().containsResource(resHash));

    barrier.set_value();

    constexpr std::chrono::seconds timeoutTime{ 2u };
    const auto startTime = std::chrono::steady_clock::now();
    while (resourceManager.getRendererResourceRegistry().containsResource(resHash)
        && std::chrono::steady_clock::now() - startTime < timeoutTime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 5u });
        resourceManager.uploadAndUnloadPendingResources();
    }
    ASSERT_FALSE(resourceManager.getRendererResourceRegistry().containsResource(resHash));
}

TEST_F(ARendererResourceManager, DoesNotUnloadEffectThatGetsUnreferencedAndReReferencedWhileCompiling)
{
    auto effectRes = std::make_unique<const EffectResource>("", "", "", EDrawMode::NUMBER_OF_ELEMENTS, EffectInputInformationVector{}, EffectInputInformationVector{}, "", ResourceCacheFlag_DoNotCache);
    const ResourceContentHash resHash = effectRes->getHash();

    // request some resources
    referenceResource(resHash, fakeSceneId);
    resourceManager.provideResourceData(std::move(effectRes));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    //simulate shader was not uploaded by ResourceUploader (not found in bin shader cache) so it gets uploaded by AsyncEffectUploader
    EXPECT_CALL(*resUploader, uploadResource(Ref(platform.renderBackendMock), _, _)).WillOnce(Invoke([resHash](auto&, const auto& rd, auto&) {
        EXPECT_EQ(resHash, rd.hash);
        EXPECT_EQ(EResourceType_Effect, rd.type);

        return std::optional<DeviceResourceHandle>{};
        }));

    //block call to upload shader to simulate scene getting unreferenced while AsyncEffectUploader is still uploading
    std::promise<void> barrier;
    EXPECT_CALL(platform.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).WillOnce(Invoke([&](const auto&) {
        barrier.get_future().get();
        return std::make_unique<const GPUResource>(1u, 2u);
        }));
    expectDeviceFlushOnWindows();
    EXPECT_CALL(platform.renderBackendMock.deviceMock, registerShader(_));
    EXPECT_CALL(*resUploader, storeShaderInBinaryShaderCache(_, _, _, _));

    resourceManager.uploadAndUnloadPendingResources();
    ASSERT_EQ(EResourceStatus::ScheduledForUpload, resourceManager.getResourceStatus(resHash));

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
    resourceManager.uploadAndUnloadPendingResources();

    const SceneId fakeSceneId2{ 432u };
    ASSERT_NE(fakeSceneId, fakeSceneId2);
    referenceResource(resHash, fakeSceneId2);
    resourceManager.uploadAndUnloadPendingResources();

    barrier.set_value();

    constexpr std::chrono::seconds timeoutTime{ 2u };
    const auto startTime = std::chrono::steady_clock::now();
    while (EResourceStatus::Uploaded != resourceManager.getResourceStatus(resHash)
        && std::chrono::steady_clock::now() - startTime < timeoutTime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 5u });
        resourceManager.uploadAndUnloadPendingResources();
    }
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resHash));
    const auto& effectSceneUsage = resourceManager.getRendererResourceRegistry().getResourceDescriptor(resHash).sceneUsage;
    ASSERT_EQ(1u, effectSceneUsage.size());
    EXPECT_EQ(fakeSceneId2, effectSceneUsage[0]);

    expectResourceUnloaded(resHash, EResourceType_Effect, DeviceMock::FakeShaderDeviceHandle);
    unreferenceResource(resHash, fakeSceneId2);
}

TEST_F(ARendererResourceManager, UploadInvalidShaderResultsInBrokenResource)
{
    auto effectRes = std::make_unique<const EffectResource>("", "", "", EDrawMode::NUMBER_OF_ELEMENTS, EffectInputInformationVector{}, EffectInputInformationVector{}, "", ResourceCacheFlag_DoNotCache);
    const ResourceContentHash resHash = effectRes->getHash();

    // request some resources
    referenceResource(resHash, fakeSceneId);
    resourceManager.provideResourceData(std::move(effectRes));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    uploadShader(resHash, false);

    // resource must be broken
    EXPECT_EQ(EResourceStatus::Broken, resourceManager.getResourceStatus(resHash));
    Mock::VerifyAndClearExpectations(&platform.renderBackendMock);

    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());

    // not expecting any unload
    unreferenceResource(resHash, fakeSceneId);
}

class ARendererResourceManagerWithDisabledEffectDeletion : public ARendererResourceManager
{
public:
    ARendererResourceManagerWithDisabledEffectDeletion()
        : ARendererResourceManager(true)
    {
    }
};

TEST_F(ARendererResourceManagerWithDisabledEffectDeletion, DeletesEffectOnlyWhenDestructed_NotWhenUnrequested)
{
    const ResourceContentHash resource = MockResourceHash::EffectHash;

    // request some resources
    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    uploadShader(resource);
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));

    unreferenceResource(resource, fakeSceneId);

    // Make sure the effect was not deleted, before resource manager is destroyed
    Mock::VerifyAndClearExpectations(&platform.renderBackendMock.deviceMock);

    // delete resource
    expectResourceUnloaded(resource, EResourceType_Effect, DeviceMock::FakeShaderDeviceHandle);
}

TEST_F(ARendererResourceManager, unloadsAllSceneResources)
{
    // upload render buffer
    RenderTargetHandle targetHandle(1u);
    RenderBufferHandleVector bufferHandles;
    bufferHandles.push_back(RenderBufferHandle(1u));
    bufferHandles.push_back(RenderBufferHandle(5u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(800u, 600u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_ReadWrite, 0u));
    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthBuffer(800u, 600u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_ReadWrite, 0u);
    resourceManager.uploadRenderTargetBuffer(bufferHandles[0], fakeSceneId, colorBuffer);
    resourceManager.uploadRenderTargetBuffer(bufferHandles[1], fakeSceneId, depthBuffer);

    // upload render target
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_));
    resourceManager.uploadRenderTarget(targetHandle, bufferHandles, fakeSceneId);

    // upload blit pass
    const BlitPassHandle blitPassHandle(100u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_)).Times(2u);
    resourceManager.uploadBlitPassRenderTargets(blitPassHandle, bufferHandles[0], bufferHandles[1], fakeSceneId);

    //upload index data buffer
    const DataBufferHandle indexDataBufferHandle(123u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateIndexBuffer(_, _));
    resourceManager.uploadDataBuffer(indexDataBufferHandle, EDataBufferType::IndexBuffer, EDataType::Float, 10u, fakeSceneId);

    //upload vertex data buffer
    const DataBufferHandle vertexDataBufferHandle(777u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateVertexBuffer(_));
    resourceManager.uploadDataBuffer(vertexDataBufferHandle, EDataBufferType::VertexBuffer, EDataType::Float, 10u, fakeSceneId);

    //upload texture buffer
    const TextureBufferHandle textureBufferHandle(666u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateTexture2D(_, _, _, _, _, _));
    resourceManager.uploadTextureBuffer(textureBufferHandle, 1u, 2u, ETextureFormat::RGBA8, 1u, fakeSceneId);

    //upload vertex array
    VertexArrayInfo vaInfo;
    vaInfo.shader = DeviceMock::FakeShaderDeviceHandle;
    vaInfo.indexBuffer = DeviceMock::FakeIndexBufferDeviceHandle;
    vaInfo.vertexBuffers.push_back({ DeviceMock::FakeVertexBufferDeviceHandle,DataFieldHandle{2u}, 3u, 4u, EDataType::Vector2F, 5u, 6u });

    const RenderableHandle renderable{ 123u };
    EXPECT_CALL(platform.renderBackendMock.deviceMock, allocateVertexArray(Ref(vaInfo)));
    resourceManager.uploadVertexArray(renderable, vaInfo, fakeSceneId);

    // unload all scene resources
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(2);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_)).Times(3);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteIndexBuffer(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteVertexBuffer(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteTexture(_));
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteVertexArray(_));
    resourceManager.unloadAllSceneResourcesForScene(fakeSceneId);

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&platform.renderBackendMock);
}

TEST_F(ARendererResourceManager, canUnreferenceAllResourcesUsedByAScene)
{
    const SceneId fakeSceneId2(fakeSceneId.getValue() + 1u);
    const auto resource1 = MockResourceHash::VertArrayHash;
    const auto resource2 = MockResourceHash::IndexArrayHash;
    const auto resource3 = MockResourceHash::TextureHash;

    // request some resources
    referenceResource(resource1, fakeSceneId);
    referenceResource(resource2, fakeSceneId);
    referenceResource(resource2, fakeSceneId2);
    referenceResource(resource3, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource1));
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource2));
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource3));

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);

    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource2));

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId2);
}

TEST_F(ARendererResourceManager, canProvideResourceData)
{
    const auto managedRes = MockResourceHash::GetManagedResource(MockResourceHash::EffectHash);
    // resource must be referenced by scene before providing data
    referenceResource(MockResourceHash::EffectHash, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.provideResourceData(managedRes);
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
}

TEST_F(ARendererResourceManager, providingResourceDataMulitpleTimesIsNoop)
{
    const auto managedRes = MockResourceHash::GetManagedResource(MockResourceHash::EffectHash);
    // resource must be referenced by scene before providing data
    referenceResource(MockResourceHash::EffectHash, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.provideResourceData(managedRes);
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    // resource is already provided so providing data again is noop
    resourceManager.provideResourceData(managedRes);
    resourceManager.provideResourceData(managedRes);
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
}

TEST_F(ARendererResourceManager, providingResourceDataWhenAlreadyUploadedIsNoop)
{
    const auto managedRes = MockResourceHash::GetManagedResource(MockResourceHash::EffectHash);
    // resource must be referenced by scene before providing data
    referenceResource(MockResourceHash::EffectHash, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.provideResourceData(managedRes);
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    // upload the resource
    uploadShader(MockResourceHash::EffectHash);
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(MockResourceHash::EffectHash).isValid());

    // resource is already provided so providing data again is noop
    resourceManager.provideResourceData(managedRes);
    resourceManager.provideResourceData(managedRes);
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(MockResourceHash::EffectHash).isValid());

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
    expectResourceUnloaded(MockResourceHash::EffectHash, EResourceType_Effect, DeviceMock::FakeShaderDeviceHandle);
}

TEST_F(ARendererResourceManagerWithDisabledEffectDeletion, rereferencingResourceKeptInCacheIsNotUploadedAgain)
{
    const auto managedRes = MockResourceHash::GetManagedResource(MockResourceHash::EffectHash);
    // resource must be referenced by scene before providing data
    referenceResource(MockResourceHash::EffectHash, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.provideResourceData(managedRes);
    EXPECT_EQ(EResourceStatus::Provided, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    // upload the resource
    uploadShader(MockResourceHash::EffectHash);
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(MockResourceHash::EffectHash).isValid());

    unreferenceResource(MockResourceHash::EffectHash, fakeSceneId);
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    referenceResource(MockResourceHash::EffectHash, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));

    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
    expectResourceUnloaded(MockResourceHash::EffectHash, EResourceType_Effect, DeviceMock::FakeShaderDeviceHandle);
}

TEST_F(ARendererResourceManager, UnloadsAllRemainingOffscreenBuffersAndStreamBuffersAtDestruction)
{
    // 2 offscreen buffers
    constexpr OffscreenBufferHandle obHandle{ 1u };
    constexpr OffscreenBufferHandle obHandle2{ 2u };
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, uploadRenderBuffer(_, _, _, _, _, _)).Times(4u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, activateRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, colorMask(true, true, true, true)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f })).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, depthWrite(EDepthWrite::Enabled)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, scissorTest(EScissorTest::Disabled, RenderState::ScissorRegion{})).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, clear(_)).Times(2u);
    resourceManager.uploadOffscreenBuffer(obHandle, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer);
    resourceManager.uploadOffscreenBuffer(obHandle2, 1u, 1u, 0u, false, ERenderBufferType_DepthStencilBuffer);

    // 2 stream buffers
    constexpr StreamBufferHandle sbHandle{ 1u };
    constexpr StreamBufferHandle sbHandle2{ 2u };
    constexpr WaylandIviSurfaceId sbSrc1{ 11u };
    constexpr WaylandIviSurfaceId sbSrc2{ 12u };
    EXPECT_CALL(embeddedCompositingManager, refStream(sbSrc1));
    EXPECT_CALL(embeddedCompositingManager, refStream(sbSrc2));
    resourceManager.uploadStreamBuffer(sbHandle, sbSrc1);
    resourceManager.uploadStreamBuffer(sbHandle2, sbSrc2);

    // will destroy OBs directly on device
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderTarget(_)).Times(2u);
    EXPECT_CALL(platform.renderBackendMock.deviceMock, deleteRenderBuffer(_)).Times(4u);

    // will destroy SBs via EC manager
    EXPECT_CALL(embeddedCompositingManager, unrefStream(sbSrc1));
    EXPECT_CALL(embeddedCompositingManager, unrefStream(sbSrc2));
}

}
