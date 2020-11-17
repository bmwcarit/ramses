//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/RendererStatistics.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/EDataBufferType.h"
#include "ResourceDeviceHandleAccessorMock.h"
#include "RendererResourceCacheMock.h"
#include "RendererResourceCacheFake.h"
#include "RenderBackendMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include "MockResourceHash.h"

namespace ramses_internal {
using namespace testing;

class ARendererResourceManager : public ::testing::Test
{
public:
    explicit ARendererResourceManager(bool disableEffectDeletion = false)
        : fakeSceneId(66u)
        , resUploader(stats)
        , frameTimer()
        , resourceManager(resUploader, renderer, embeddedCompositingManager, disableEffectDeletion, frameTimer, stats)
    {
    }

    ~ARendererResourceManager()
    {
        // no actual unload expected but clears internal lists
        resourceManager.unloadAllSceneResourcesForScene(fakeSceneId);
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

protected:
    StrictMock<RenderBackendStrictMock> renderer;
    StrictMock<RenderBackendStrictMock> additionalRenderer;
    StrictMock<EmbeddedCompositingManagerMock> embeddedCompositingManager;
    SceneId fakeSceneId;
    RendererStatistics stats;
    ResourceUploader resUploader;
    FrameTimer frameTimer;
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
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(resource).isValid());
    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());

    // delete resource
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.unreferenceResourcesForScene(fakeSceneId, { resource });
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
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

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(resource).isValid());

    resourceManager.unreferenceResourcesForScene(fakeSceneId, { resource });
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
}

TEST_F(ARendererResourceManager, referencingAnUnloadedResourceAgainShouldUploadItAgain)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;
    const auto managedRes = MockResourceHash::GetManagedResource(resource);

    // ref
    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(managedRes);
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingResources();

    // unref
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.unreferenceResourcesForScene(fakeSceneId, resources);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingResources();

    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());

    Mock::VerifyAndClearExpectations(&renderer);

    // ref again
    referenceResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus::Registered, resourceManager.getResourceStatus(resource));
    resourceManager.provideResourceData(managedRes);
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_CALL(renderer, getDevice()).Times(2);
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingResources();

    //general clean-up (expect needed because of strict mock)
    unreferenceResource(resource, fakeSceneId);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingResources();
}

TEST_F(ARendererResourceManager, deletesNoLongerNeededResourcesWhenSceneDestroyed)
{
    ResourceContentHash resource = MockResourceHash::VertArrayHash;

    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingResources();

    ResourceContentHashVector usedResources;
    usedResources.push_back(resource);
    resourceManager.unreferenceResourcesForScene(fakeSceneId, usedResources);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
}

TEST_F(ARendererResourceManager, callsEmbeddedCompositingManagerForUploadingAndUnloadingStreamTexture)
{
    const StreamTextureHandle handle(1);
    const WaylandIviSurfaceId source(2);
    EXPECT_CALL(embeddedCompositingManager, refStream(handle, source, fakeSceneId));
    resourceManager.uploadStreamTexture(handle, source, fakeSceneId);

    EXPECT_CALL(embeddedCompositingManager, unrefStream(handle, source, fakeSceneId));
    resourceManager.unloadStreamTexture(handle, fakeSceneId);
}

TEST_F(ARendererResourceManager, doesNotDeleteResourcesNeededByAnotherSceneWhenSceneDestroyed)
{
    ResourceContentHash vertResource = MockResourceHash::VertArrayHash;
    ResourceContentHash indexResource = MockResourceHash::IndexArrayHash;

    const SceneId fakeSceneId2(4u);

    //index array is also needed by scene 2
    referenceResource(vertResource, fakeSceneId);
    referenceResource(indexResource, fakeSceneId);
    referenceResource(indexResource, fakeSceneId2);

    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(vertResource));
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(indexResource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    EXPECT_CALL(renderer.deviceMock, allocateIndexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadIndexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingResources();

    resourceManager.unreferenceResourcesForScene(fakeSceneId, { vertResource, indexResource });
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(_)).Times(0); //not allowed to be deleted
    resourceManager.uploadAndUnloadPendingResources();

    Mock::VerifyAndClearExpectations(&renderer);

    //general clean-up (expect needed because of strict mock)
    unreferenceResource(indexResource, fakeSceneId2);
    EXPECT_CALL(renderer, getDevice());
    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(_));
    resourceManager.uploadAndUnloadPendingResources();
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadDataBuffer_IndexBuffer)
{
    const DataBufferHandle dataBuffer(1u);
    const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
    const EDataType dataType = EDataType::UInt32;
    const UInt32 sizeInBytes = 1024u;
    EXPECT_CALL(renderer.deviceMock, allocateIndexBuffer(dataType, sizeInBytes));
    resourceManager.uploadDataBuffer(dataBuffer, dataBufferType, dataType, sizeInBytes, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeIndexBufferDeviceHandle, resourceManager.getDataBufferDeviceHandle(dataBuffer, fakeSceneId));

    const Byte dummyData[10] = {};
    EXPECT_CALL(renderer.deviceMock, uploadIndexBufferData(DeviceMock::FakeIndexBufferDeviceHandle, dummyData, 7u));
    resourceManager.updateDataBuffer(dataBuffer, 7u, dummyData, fakeSceneId);

    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(DeviceMock::FakeIndexBufferDeviceHandle));
    resourceManager.unloadDataBuffer(dataBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadDataBuffer_VertexBuffer)
{
    const DataBufferHandle dataBuffer(1u);
    const EDataBufferType dataBufferType = EDataBufferType::VertexBuffer;
    const EDataType dataType = EDataType::UInt32;
    const UInt32 sizeInBytes = 1024u;
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(sizeInBytes));
    resourceManager.uploadDataBuffer(dataBuffer, dataBufferType, dataType, sizeInBytes, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeVertexBufferDeviceHandle, resourceManager.getDataBufferDeviceHandle(dataBuffer, fakeSceneId));

    const Byte dummyData[10] = {};
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(DeviceMock::FakeVertexBufferDeviceHandle, dummyData, 7u));
    resourceManager.updateDataBuffer(dataBuffer, 7u, dummyData, fakeSceneId);

    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(DeviceMock::FakeVertexBufferDeviceHandle));
    resourceManager.unloadDataBuffer(dataBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadTextureBuffer_WithOneMipLevel)
{
    InSequence seq;
    const TextureBufferHandle textureBuffer(1u);
    const UInt32 expectedSize = (10u * 20u) * 4u;
    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(10u, 20u, ETextureFormat::RGBA8, DefaultTextureSwizzleArray, 1u, expectedSize));
    resourceManager.uploadTextureBuffer(textureBuffer, 10u, 20u, ETextureFormat::RGBA8, 1u, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeTextureDeviceHandle, resourceManager.getTextureBufferDeviceHandle(textureBuffer, fakeSceneId));

    const Byte dummyTexture[10] = {};
    EXPECT_CALL(renderer.deviceMock, bindTexture(DeviceMock::FakeTextureDeviceHandle));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceMock::FakeTextureDeviceHandle, 0u, 2u, 3u, 0u, 4u, 5u, 1u, dummyTexture, 0u));
    resourceManager.updateTextureBuffer(textureBuffer, 0u, 2u, 3u, 4u, 5u, dummyTexture, fakeSceneId);

    EXPECT_CALL(renderer.deviceMock, deleteTexture(DeviceMock::FakeTextureDeviceHandle));
    resourceManager.unloadTextureBuffer(textureBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadTextureBuffer_WithSeveralMipLevels)
{
    InSequence seq;
    const TextureBufferHandle textureBuffer(1u);
    const UInt32 expectedSize = (10u * 20u + 5u * 10u)* 4u;
    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(10u, 20u, ETextureFormat::RGBA8, DefaultTextureSwizzleArray, 2u, expectedSize));
    resourceManager.uploadTextureBuffer(textureBuffer, 10u, 20u, ETextureFormat::RGBA8, 2u, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeTextureDeviceHandle, resourceManager.getTextureBufferDeviceHandle(textureBuffer, fakeSceneId));

    const Byte dummyTexture[10] = {};
    EXPECT_CALL(renderer.deviceMock, bindTexture(DeviceMock::FakeTextureDeviceHandle));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceMock::FakeTextureDeviceHandle, 1u, 2u, 3u, 0u, 4u, 5u, 1u, _, 0u));
    resourceManager.updateTextureBuffer(textureBuffer, 1u, 2u, 3u, 4u, 5u, dummyTexture, fakeSceneId);

    EXPECT_CALL(renderer.deviceMock, deleteTexture(DeviceMock::FakeTextureDeviceHandle));
    resourceManager.unloadTextureBuffer(textureBuffer, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUnloadTextureSampler)
{
    const TextureSamplerHandle textureSampler(1u);
    const EWrapMethod wrapU(EWrapMethod::Clamp);
    const EWrapMethod wrapV(EWrapMethod::Repeat);
    const EWrapMethod wrapR(EWrapMethod::RepeatMirrored);
    const ESamplingMethod minSampling(ESamplingMethod::Linear);
    const ESamplingMethod magSampling(ESamplingMethod::Linear_MipMapLinear);
    const UInt32 anisotropyLevel(2u);
    const TextureSamplerStates state(wrapU, wrapV, wrapR, minSampling, magSampling, anisotropyLevel);

    EXPECT_CALL(renderer.deviceMock, uploadTextureSampler(wrapU, wrapV, wrapR, minSampling, magSampling, anisotropyLevel));
    resourceManager.uploadTextureSampler(textureSampler, fakeSceneId, state);

    EXPECT_EQ(DeviceMock::FakeTextureSamplerDeviceHandle, resourceManager.getTextureSamplerDeviceHandle(textureSampler, fakeSceneId));

    EXPECT_CALL(renderer.deviceMock, deleteTextureSampler(DeviceMock::FakeTextureSamplerDeviceHandle));
    resourceManager.unloadTextureSampler(textureSampler, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUnloadRenderTargetBuffer)
{
    RenderBufferHandle bufferHandle(1u);
    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);

    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(colorBuffer)));
    resourceManager.uploadRenderTargetBuffer(bufferHandle, fakeSceneId, colorBuffer);

    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getRenderTargetBufferDeviceHandle(bufferHandle, fakeSceneId));

    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_));
    resourceManager.unloadRenderTargetBuffer(bufferHandle, fakeSceneId);
}

TEST_F(ARendererResourceManager, canUploadAndUnloadBlitPassRenderTargets)
{
    const BlitPassHandle blitPassHandle(100u);
    const RenderBufferHandle sourceRenderBufferHandle(101u);
    const RenderBufferHandle destinationRenderBufferHandle(102u);

    const RenderBuffer renderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);

    const DeviceResourceHandle sourceRenderBufferDeviceHandle(201u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(renderBuffer))).WillOnce(Return(sourceRenderBufferDeviceHandle));
    resourceManager.uploadRenderTargetBuffer(sourceRenderBufferHandle, fakeSceneId, renderBuffer);

    const DeviceResourceHandle destinationRenderBufferDeviceHandle(202u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(renderBuffer))).WillOnce(Return(destinationRenderBufferDeviceHandle));
    resourceManager.uploadRenderTargetBuffer(destinationRenderBufferHandle, fakeSceneId, renderBuffer);

    const DeviceResourceHandle blittingRenderTargetSrcDeviceHandle(203u);
    const DeviceResourceHandle blittingRenderTargetDstDeviceHandle(204u);
    DeviceHandleVector rbsSrc;
    rbsSrc.push_back(sourceRenderBufferDeviceHandle);
    DeviceHandleVector rbsDst;
    rbsDst.push_back(destinationRenderBufferDeviceHandle);
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(Eq(rbsSrc))).WillOnce(Return(blittingRenderTargetSrcDeviceHandle));
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(Eq(rbsDst))).WillOnce(Return(blittingRenderTargetDstDeviceHandle));
    resourceManager.uploadBlitPassRenderTargets(blitPassHandle, sourceRenderBufferHandle, destinationRenderBufferHandle, fakeSceneId);

    DeviceResourceHandle actualBlitPassRTSrc;
    DeviceResourceHandle actualBlitPassRTDst;
    resourceManager.getBlitPassRenderTargetsDeviceHandle(blitPassHandle, fakeSceneId, actualBlitPassRTSrc, actualBlitPassRTDst);
    EXPECT_EQ(blittingRenderTargetSrcDeviceHandle, actualBlitPassRTSrc);
    EXPECT_EQ(blittingRenderTargetDstDeviceHandle, actualBlitPassRTDst);

    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(actualBlitPassRTSrc));
    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(actualBlitPassRTDst));
    resourceManager.unloadBlitPassRenderTargets(blitPassHandle, fakeSceneId);

    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2);
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
        EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(colorBuffer)));
        EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(depthBuffer)));
        EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_));
    }

    resourceManager.uploadRenderTargetBuffer(bufferHandles[0], fakeSceneId, colorBuffer);
    resourceManager.uploadRenderTargetBuffer(bufferHandles[1], fakeSceneId, depthBuffer);

    resourceManager.uploadRenderTarget(targetHandle, bufferHandles, fakeSceneId);

    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getRenderTargetBufferDeviceHandle(bufferHandles[0], fakeSceneId));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getRenderTargetBufferDeviceHandle(bufferHandles[1], fakeSceneId));
    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getRenderTargetDeviceHandle(targetHandle, fakeSceneId));

    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2);
    resourceManager.unloadRenderTargetBuffer(bufferHandles[0], fakeSceneId);
    resourceManager.unloadRenderTargetBuffer(bufferHandles[1], fakeSceneId);
    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_));
    resourceManager.unloadRenderTarget(targetHandle, fakeSceneId);
}

TEST_F(ARendererResourceManager, GetsInvalidDeviceHandleForUnknownOffscreenBuffer)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
    EXPECT_FALSE(resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle).isValid());
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorAndDepthStencilAttached)
{
    const OffscreenBufferHandle bufferHandle(1u);
    const RenderBuffer colorOffscreenBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthOffscreenBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat::Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 0u);
    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(colorOffscreenBuffer)));
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(depthOffscreenBuffer)));
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(renderer.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(renderer.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(renderer.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(renderer.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, false);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2u);
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithColorAndDepthStencilAttachedWithMSAAenabled)
{
    const OffscreenBufferHandle bufferHandle(1u);
    const RenderBuffer colorOffscreenBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 4u);
    const RenderBuffer depthOffscreenBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat::Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 4u);
    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(colorOffscreenBuffer)));
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(depthOffscreenBuffer)));
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(renderer.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(renderer.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(renderer.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(renderer.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 4u, false);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2u);
}

TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithDoubleBuffering)
{
    const OffscreenBufferHandle bufferHandle(1u);
    const RenderBuffer colorOffscreenBuffer1(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer colorOffscreenBuffer2(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthOffscreenBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat::Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 0u);

    const DeviceResourceHandle colorBufferDeviceHandle1{ 7771u };
    const DeviceResourceHandle colorBufferDeviceHandle2{ 7778u };
    const DeviceResourceHandle depthBufferDeviceHandle{ 7796u };
    const DeviceResourceHandle offscreenBufferDeviceHandle1{ 7798u };
    const DeviceResourceHandle offscreenBufferDeviceHandle2{ 7799u };
    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(colorOffscreenBuffer1))).WillOnce(Return(colorBufferDeviceHandle1));
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(depthOffscreenBuffer))).WillOnce(Return(depthBufferDeviceHandle));
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(Eq(DeviceHandleVector({colorBufferDeviceHandle1, depthBufferDeviceHandle})))).WillOnce(Return(offscreenBufferDeviceHandle1));

    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(colorOffscreenBuffer2))).WillOnce(Return(colorBufferDeviceHandle2));
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(Eq(DeviceHandleVector({ colorBufferDeviceHandle2, depthBufferDeviceHandle })))).WillOnce(Return(offscreenBufferDeviceHandle2));

    EXPECT_CALL(renderer.deviceMock, activateRenderTarget(offscreenBufferDeviceHandle1));
    EXPECT_CALL(renderer.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(renderer.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(renderer.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(renderer.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(renderer.deviceMock, clear(_));

    EXPECT_CALL(renderer.deviceMock, activateRenderTarget(offscreenBufferDeviceHandle2));
    EXPECT_CALL(renderer.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(renderer.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(renderer.deviceMock, depthWrite(EDepthWrite::Enabled));
    EXPECT_CALL(renderer.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(renderer.deviceMock, clear(_));
    EXPECT_CALL(renderer.deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, true);

    EXPECT_EQ(offscreenBufferDeviceHandle1, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(colorBufferDeviceHandle1, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(offscreenBufferDeviceHandle1));

    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_)).Times(2);
    EXPECT_CALL(renderer.deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(3u);
}

TEST_F(ARendererResourceManager, CanUnloadOffscreenBuffer)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, activateRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, colorMask(true, true, true, true));
    EXPECT_CALL(renderer.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f }));
    EXPECT_CALL(renderer.deviceMock, depthWrite(EDepthWrite::Enabled));
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(renderer.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
    EXPECT_CALL(renderer.deviceMock, clear(_));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, false);

    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2u);
    resourceManager.unloadOffscreenBuffer(bufferHandle);

    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
}

TEST_F(ARendererResourceManager, CanUnloadDoubleBufferedOffscreenBuffer)
{
    const OffscreenBufferHandle bufferHandle(1u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(_)).Times(3u);
    EXPECT_CALL(renderer.deviceMock, activateRenderTarget(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, colorMask(true, true, true, true)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f })).Times(2u);
    EXPECT_CALL(renderer.deviceMock, depthWrite(EDepthWrite::Enabled)).Times(2u);
    RenderState::ScissorRegion scissorRegion{};
    EXPECT_CALL(renderer.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, clear(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, pairRenderTargetsForDoubleBuffering(_, _));
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, 0u, true);

    EXPECT_CALL(renderer.deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(3u);
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

TEST_F(ARendererResourceManager, UploadAndDeleteValidShader)
{
    const ResourceContentHash resource = MockResourceHash::EffectHash;

    // request some resources
    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    resourceManager.uploadAndUnloadPendingResources();

    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));

    unreferenceResource(resource, fakeSceneId);
    EXPECT_CALL(renderer.deviceMock, deleteShader(_));
    resourceManager.uploadAndUnloadPendingResources();
}

TEST_F(ARendererResourceManager, UploadInvalidShaderResultsInBrokenResource)
{
    const ResourceContentHash resource = MockResourceHash::EffectHash;

    // request some resources
    referenceResource(resource, fakeSceneId);
    resourceManager.provideResourceData(MockResourceHash::GetManagedResource(resource));
    EXPECT_TRUE(resourceManager.hasResourcesToBeUploaded());
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    resourceManager.uploadAndUnloadPendingResources();

    // resource must be broken
    EXPECT_EQ(EResourceStatus::Broken, resourceManager.getResourceStatus(resource));
    Mock::VerifyAndClearExpectations(&renderer);

    EXPECT_FALSE(resourceManager.hasResourcesToBeUploaded());

    // not expecting any unload
    unreferenceResource(resource, fakeSceneId);
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
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(resource));

    unreferenceResource(resource, fakeSceneId);

    // Make sure the effect was not deleted, before resource manager is destroyed
    Mock::VerifyAndClearExpectations(&renderer.deviceMock);

    // delete resource
    EXPECT_CALL(renderer.deviceMock, deleteShader(_));
}

TEST_F(ARendererResourceManager, unloadsAllSceneResources)
{
    // upload render buffer
    RenderTargetHandle targetHandle(1u);
    RenderBufferHandleVector bufferHandles;
    bufferHandles.push_back(RenderBufferHandle(1u));
    bufferHandles.push_back(RenderBufferHandle(5u));
    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthBuffer(800u, 600u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth24, ERenderBufferAccessMode_ReadWrite, 0u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(colorBuffer)));
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(Eq(depthBuffer)));
    resourceManager.uploadRenderTargetBuffer(bufferHandles[0], fakeSceneId, colorBuffer);
    resourceManager.uploadRenderTargetBuffer(bufferHandles[1], fakeSceneId, depthBuffer);

    // upload render target
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_));
    resourceManager.uploadRenderTarget(targetHandle, bufferHandles, fakeSceneId);

    // upload blit pass
    const BlitPassHandle blitPassHandle(100u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_)).Times(2u);
    resourceManager.uploadBlitPassRenderTargets(blitPassHandle, bufferHandles[0], bufferHandles[1], fakeSceneId);

    // upload stream texture
    const StreamTextureHandle streamTextureHandle(1);
    const WaylandIviSurfaceId source(2);
    EXPECT_CALL(embeddedCompositingManager, refStream(streamTextureHandle, source, fakeSceneId));
    resourceManager.uploadStreamTexture(streamTextureHandle, source, fakeSceneId);

    //upload index data buffer
    const DataBufferHandle indexDataBufferHandle(123u);
    EXPECT_CALL(renderer.deviceMock, allocateIndexBuffer(_, _));
    resourceManager.uploadDataBuffer(indexDataBufferHandle, EDataBufferType::IndexBuffer, EDataType::Float, 10u, fakeSceneId);

    //upload vertex data buffer
    const DataBufferHandle vertexDataBufferHandle(777u);
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_));
    resourceManager.uploadDataBuffer(vertexDataBufferHandle, EDataBufferType::VertexBuffer, EDataType::Float, 10u, fakeSceneId);

    //upload texture buffer
    const TextureBufferHandle textureBufferHandle(666u);
    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(_, _, _, _, _, _));
    resourceManager.uploadTextureBuffer(textureBufferHandle, 1u, 2u, ETextureFormat::RGBA8, 1u, fakeSceneId);

    // unload all scene resources
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2);
    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_)).Times(3);
    EXPECT_CALL(embeddedCompositingManager, unrefStream(streamTextureHandle, source, fakeSceneId));
    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, deleteTexture(_));
    resourceManager.unloadAllSceneResourcesForScene(fakeSceneId);

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
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
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    resourceManager.uploadAndUnloadPendingResources();
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(MockResourceHash::EffectHash).isValid());

    // resource is already provided so providing data again is noop
    resourceManager.provideResourceData(managedRes);
    resourceManager.provideResourceData(managedRes);
    EXPECT_EQ(EResourceStatus::Uploaded, resourceManager.getResourceStatus(MockResourceHash::EffectHash));
    EXPECT_TRUE(resourceManager.getResourceDeviceHandle(MockResourceHash::EffectHash).isValid());

    EXPECT_CALL(renderer.deviceMock, deleteShader(_));
    resourceManager.unreferenceAllResourcesForScene(fakeSceneId);
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
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    resourceManager.uploadAndUnloadPendingResources();
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
    EXPECT_CALL(renderer.deviceMock, deleteShader(_));
}

TEST_F(ARendererResourceManager, UnloadsAllRemainingOffscreenBuffersAndStreamBuffersAtDestruction)
{
    // 2 offscreen buffers
    constexpr OffscreenBufferHandle obHandle{ 1u };
    constexpr OffscreenBufferHandle obHandle2{ 2u };
    EXPECT_CALL(renderer.deviceMock, uploadRenderTarget(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, uploadRenderBuffer(_)).Times(4u);
    EXPECT_CALL(renderer.deviceMock, activateRenderTarget(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, colorMask(true, true, true, true)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, clearColor(Vector4{ 0.f, 0.f, 0.f, 1.f })).Times(2u);
    EXPECT_CALL(renderer.deviceMock, depthWrite(EDepthWrite::Enabled)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, scissorTest(EScissorTest::Disabled, RenderState::ScissorRegion{})).Times(2u);
    EXPECT_CALL(renderer.deviceMock, clear(_)).Times(2u);
    resourceManager.uploadOffscreenBuffer(obHandle, 1u, 1u, 0u, false);
    resourceManager.uploadOffscreenBuffer(obHandle2, 1u, 1u, 0u, false);

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
    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(4u);

    // will destroy SBs via EC manager
    EXPECT_CALL(embeddedCompositingManager, unrefStream(sbSrc1));
    EXPECT_CALL(embeddedCompositingManager, unrefStream(sbSrc2));
}

}
