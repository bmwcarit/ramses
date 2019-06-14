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
#include "ResourceProviderMock.h"
#include "RendererResourceCacheMock.h"
#include "RendererResourceCacheFake.h"
#include "RenderBackendMock.h"
#include "EmbeddedCompositingManagerMock.h"

namespace ramses_internal {
using namespace testing;

class ARendererResourceManager : public ::testing::Test
{
public:
    ARendererResourceManager(bool disableEffectDeletion = false)
        : fakeSceneId(66u)
        , resUploader(stats)
        , frameTimer()
        , resourceManager(resourceProvider, resUploader, renderer, embeddedCompositingManager, RequesterID(1), disableEffectDeletion, frameTimer, stats)
    {
    }

    ~ARendererResourceManager()
    {
        // no actual unload expected but clears internal lists
        resourceManager.unloadAllSceneResourcesForScene(fakeSceneId);
    }

    void requestResource(ResourceContentHash hash, SceneId sceneId, bool expectRequest = true)
    {
        ResourceContentHashVector list;
        list.push_back(hash);

        resourceManager.referenceClientResourcesForScene(sceneId, list);
        if (expectRequest)
        {
            EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(list, resourceManager.getRequesterID(), sceneId));
        }
        resourceManager.requestAndUnrequestPendingClientResources();
    }

    void unrequestResource(ResourceContentHash hash, SceneId sceneId)
    {
        ResourceContentHashVector list;
        list.push_back(hash);

        resourceManager.unreferenceClientResourcesForScene(sceneId, list);
        resourceManager.requestAndUnrequestPendingClientResources();
    }

    void rendererSceneUpdaterFlowWithCache(IRendererResourceCache* cache)
    {
        // This is the flow from RendererSceneUpdater
        resourceManager.getRequestedResourcesAlreadyInCache(cache);
        resourceManager.requestAndUnrequestPendingClientResources();
        resourceManager.processArrivedClientResources(cache);
    }

protected:
    StrictMock<ResourceProviderMock> resourceProvider;
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
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    // request some resources
    requestResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getClientResourceDeviceHandle(resource).isValid());

    // check if resources have arrived -> we also have an object now
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_EQ(EResourceStatus_Provided, resourceManager.getClientResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getClientResourceDeviceHandle(resource).isValid());

    // upload the resource
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingClientResources();
    EXPECT_EQ(EResourceStatus_Uploaded, resourceManager.getClientResourceStatus(resource));
    EXPECT_TRUE(resourceManager.getClientResourceDeviceHandle(resource).isValid());

    // delete resource
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, resources);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingClientResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
}

TEST_F(ARendererResourceManager, SplitsResourceRequestsByProvidingScene)
{
    const SceneId fakeSceneId2(fakeSceneId.getValue() + 1);

    const ResourceContentHashVector hashs1(1, ResourceProviderMock::FakeVertArrayHash);
    const ResourceContentHashVector hashs2(1, ResourceProviderMock::FakeIndexArrayHash);

    resourceManager.referenceClientResourcesForScene(fakeSceneId, hashs1);
    resourceManager.referenceClientResourcesForScene(fakeSceneId2, hashs2);

    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(hashs1, resourceManager.getRequesterID(), fakeSceneId));
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(hashs2, resourceManager.getRequesterID(), fakeSceneId2));
    resourceManager.requestAndUnrequestPendingClientResources();

    EXPECT_CALL(resourceProvider, cancelResourceRequest(_, _)).Times(2);
    unrequestResource(ResourceProviderMock::FakeVertArrayHash, fakeSceneId);
    unrequestResource(ResourceProviderMock::FakeIndexArrayHash, fakeSceneId2);
}

TEST_F(ARendererResourceManager, ResourceIsRequestedByAllScenesUsingIt)
{
    const SceneId fakeSceneId2(fakeSceneId.getValue() + 1);

    const ResourceContentHashVector hashs(1, ResourceProviderMock::FakeVertArrayHash);

    resourceManager.referenceClientResourcesForScene(fakeSceneId, hashs);
    resourceManager.referenceClientResourcesForScene(fakeSceneId2, hashs);

    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(hashs, resourceManager.getRequesterID(), fakeSceneId));
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(hashs, resourceManager.getRequesterID(), fakeSceneId2));
    resourceManager.requestAndUnrequestPendingClientResources();

    EXPECT_CALL(resourceProvider, cancelResourceRequest(_, _)).Times(1);
    unrequestResource(ResourceProviderMock::FakeVertArrayHash, fakeSceneId);
    unrequestResource(ResourceProviderMock::FakeVertArrayHash, fakeSceneId2);
}

TEST_F(ARendererResourceManager, RequestedResourceHasCorrectStatus)
{
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    requestResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getClientResourceDeviceHandle(resource).isValid());

    EXPECT_CALL(resourceProvider, cancelResourceRequest(resource, _));
    unrequestResource(resource, fakeSceneId);
}

TEST_F(ARendererResourceManager, RequestedAndArrivedResourceHasCorrectStatus)
{
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    requestResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));
    EXPECT_FALSE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_FALSE(resourceManager.getClientResourceDeviceHandle(resource).isValid());

    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_EQ(EResourceStatus_Provided, resourceManager.getClientResourceStatus(resource));
    EXPECT_FALSE(resourceManager.getClientResourceDeviceHandle(resource).isValid());

    unrequestResource(resource, fakeSceneId);
}

TEST_F(ARendererResourceManager, unrequestingResourceThatWasNotUploadedYetWillNotReportItAsPendingForUploadAnymore)
{
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    requestResource(resource, fakeSceneId);
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_EQ(EResourceStatus_Provided, resourceManager.getClientResourceStatus(resource));

    unrequestResource(resource, fakeSceneId);
    EXPECT_FALSE(resourceManager.hasClientResourcesToBeUploaded());
}

TEST_F(ARendererResourceManager, DeletesResourceUsedBySingleSceneWhenUnrequested)
{
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    // request some resource
    requestResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));

    // check if resource has arrived -> we also have an object now
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingClientResources();
    EXPECT_EQ(EResourceStatus_Uploaded, resourceManager.getClientResourceStatus(resource));
    EXPECT_TRUE(resourceManager.getClientResourceDeviceHandle(resource).isValid());

    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, resources);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingClientResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
}

TEST_F(ARendererResourceManager, requestingAnUnloadedResourceAgainShouldUploadItAgain)
{
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    // request
    requestResource(resource, fakeSceneId);
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingClientResources();

    // unrequest
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, resources);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingClientResources();

    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_FALSE(resourceManager.hasClientResourcesToBeUploaded());

    Mock::VerifyAndClearExpectations(&renderer);

    // request again
    requestResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_CALL(renderer, getDevice()).Times(2);
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingClientResources();

    //general clean-up (expect needed because of strict mock)
    unrequestResource(resource, fakeSceneId);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingClientResources();
}

TEST_F(ARendererResourceManager, deletesNoLongerNeededResourcesWhenSceneDestroyed)
{
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    requestResource(resource, fakeSceneId);
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingClientResources();

    ResourceContentHashVector usedResources;
    usedResources.push_back(resource);
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, usedResources);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    resourceManager.uploadAndUnloadPendingClientResources();

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
}

TEST_F(ARendererResourceManager, callsEmbeddedCompositingManagerForUploadingAndUnloadingStreamTexture)
{
    const StreamTextureHandle handle(1);
    const StreamTextureSourceId source(2);
    EXPECT_CALL(embeddedCompositingManager, uploadStreamTexture(handle, source, fakeSceneId));
    resourceManager.uploadStreamTexture(handle, source, fakeSceneId);

    EXPECT_CALL(embeddedCompositingManager, deleteStreamTexture(handle, source, fakeSceneId));
    resourceManager.unloadStreamTexture(handle, fakeSceneId);
}

TEST_F(ARendererResourceManager, doesNotDeleteResourcesNeededByAnotherSceneWhenSceneDestroyed)
{
    ResourceContentHash vertResource = ResourceProviderMock::FakeVertArrayHash;
    ResourceContentHash indexResource = ResourceProviderMock::FakeIndexArrayHash;

    const SceneId fakeSceneId2(4u);

    //index array is also needed by scene 2
    requestResource(vertResource, fakeSceneId);
    requestResource(indexResource, fakeSceneId);
    requestResource(indexResource, fakeSceneId2, false);

    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(_, _, _));
    EXPECT_CALL(renderer.deviceMock, allocateIndexBuffer(_, _));
    EXPECT_CALL(renderer.deviceMock, uploadIndexBufferData(_, _, _));
    resourceManager.uploadAndUnloadPendingClientResources();

    ResourceContentHashVector usedResources;
    usedResources.push_back(vertResource);
    usedResources.push_back(indexResource);
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, usedResources);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(_)).Times(0); //not allowed to be deleted
    resourceManager.uploadAndUnloadPendingClientResources();

    Mock::VerifyAndClearExpectations(&renderer);

    //general clean-up (expect needed because of strict mock)
    unrequestResource(indexResource, fakeSceneId2);
    EXPECT_CALL(renderer, getDevice());
    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(_));
    resourceManager.uploadAndUnloadPendingClientResources();
}

TEST_F(ARendererResourceManager, canUploadAndUpdateAndUnloadDataBuffer_IndexBuffer)
{
    const DataBufferHandle dataBuffer(1u);
    const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
    const EDataType dataType = EDataType_UInt32;
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
    const EDataType dataType = EDataType_UInt32;
    const UInt32 sizeInBytes = 1024u;
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(dataType, sizeInBytes));
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
    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(10u, 20u, ETextureFormat_RGBA8, 1u, expectedSize));
    resourceManager.uploadTextureBuffer(textureBuffer, 10u, 20u, ETextureFormat_RGBA8, 1u, fakeSceneId);

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
    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(10u, 20u, ETextureFormat_RGBA8, 2u, expectedSize));
    resourceManager.uploadTextureBuffer(textureBuffer, 10u, 20u, ETextureFormat_RGBA8, 2u, fakeSceneId);

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
    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);

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

    const RenderBuffer renderBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);

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

    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthBuffer(800u, 600u, ERenderBufferType_DepthBuffer, ETextureFormat_Depth24, ERenderBufferAccessMode_ReadWrite, 0u);

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
    const RenderBuffer colorOffscreenBuffer(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthOffscreenBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat_Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 0u);
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
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, false);

    EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, resourceManager.getOffscreenBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, resourceManager.getOffscreenBufferColorBufferDeviceHandle(bufferHandle));
    EXPECT_EQ(bufferHandle, resourceManager.getOffscreenBufferHandle(DeviceMock::FakeRenderTargetDeviceHandle));

    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_));
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2u);
}


TEST_F(ARendererResourceManager, UploadsOffscreenBufferWithDoubleBuffering)
{
    const OffscreenBufferHandle bufferHandle(1u);
    const RenderBuffer colorOffscreenBuffer1(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer colorOffscreenBuffer2(1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthOffscreenBuffer(1u, 1u, ERenderBufferType_DepthStencilBuffer, ETextureFormat_Depth24_Stencil8, ERenderBufferAccessMode_WriteOnly, 0u);

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
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, true);

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
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, false);

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
    resourceManager.uploadOffscreenBuffer(bufferHandle, 1u, 1u, true);

    EXPECT_CALL(renderer.deviceMock, unpairRenderTargets(_)).Times(1u);
    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_)).Times(2u);
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(3u);
    resourceManager.unloadOffscreenBuffer(bufferHandle);

    EXPECT_FALSE(resourceManager.getOffscreenBufferDeviceHandle(bufferHandle).isValid());
}

TEST_F(ARendererResourceManager, UploadAndDeleteValidShader)
{
    const ResourceContentHash resource = ResourceProviderMock::FakeEffectHash;

    // request some resources
    requestResource(resource, fakeSceneId);
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    resourceManager.uploadAndUnloadPendingClientResources();

    EXPECT_EQ(EResourceStatus_Uploaded, resourceManager.getClientResourceStatus(resource));

    unrequestResource(resource, fakeSceneId);
    EXPECT_CALL(renderer.deviceMock, deleteShader(_));
    resourceManager.uploadAndUnloadPendingClientResources();
}

TEST_F(ARendererResourceManager, UploadInvalidShaderResultsInBrokenResource)
{
    const ResourceContentHash resource = ResourceProviderMock::FakeEffectHash;

    // request some resources
    requestResource(resource, fakeSceneId);
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillRepeatedly(Return(DeviceResourceHandle::Invalid()));
    resourceManager.uploadAndUnloadPendingClientResources();

    // resource must be broken
    EXPECT_EQ(EResourceStatus_Broken, resourceManager.getClientResourceStatus(resource));
    Mock::VerifyAndClearExpectations(&renderer);

    EXPECT_FALSE(resourceManager.hasClientResourcesToBeUploaded());

    // not expecting any unload
    unrequestResource(resource, fakeSceneId);
}

TEST_F(ARendererResourceManager, UnrequestingResourceThatDidNotArriveDoesNotDeleteItFromGPU)
{
    ResourceContentHash resource = ResourceProviderMock::FakeVertArrayHash;

    requestResource(resource, fakeSceneId);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));

    ResourceContentHashVector resources;
    resources.push_back(resource);

    EXPECT_CALL(resourceProvider, cancelResourceRequest(resource, resourceManager.getRequesterID()));
    unrequestResource(resource, fakeSceneId);

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
}

TEST_F(ARendererResourceManager, reportsNoArrivedResourcesIfRequestedUnavailableResource)
{
    const ResourceContentHash resource(0x00ff00ff, 0);
    requestResource(resource, fakeSceneId);
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_FALSE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));

    ResourceContentHashVector resources;
    resources.push_back(resource);
    EXPECT_CALL(resourceProvider, cancelResourceRequest(resource, _));
    unrequestResource(resource, fakeSceneId);
    EXPECT_FALSE(resourceManager.hasClientResourcesToBeUploaded());
}

TEST_F(ARendererResourceManager, rerequestsResourceIfUnavailableForSeveralUpdateIterations)
{
    const ResourceContentHash resource(0x00ff00ff, 0);
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.referenceClientResourcesForScene(fakeSceneId, resources);

    InSequence seq;
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(resources, resourceManager.getRequesterID(), fakeSceneId));
    resourceManager.requestAndUnrequestPendingClientResources();

    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(resources, resourceManager.getRequesterID(), fakeSceneId));
    for (UInt32 i = 0u; i < 60; ++i)
    {
        resourceManager.requestAndUnrequestPendingClientResources();
    }
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));

    EXPECT_CALL(resourceProvider, cancelResourceRequest(resource, _));
    unrequestResource(resource, fakeSceneId);
}

TEST_F(ARendererResourceManager, doesNotRerequestsResourceAfterItArrived)
{
    const ResourceContentHash resource = ResourceProviderMock::FakeIndexArrayHash;
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.referenceClientResourcesForScene(fakeSceneId, resources);

    InSequence seq;

    resourceProvider.setIndexArrayAvailability(false);
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(resources, resourceManager.getRequesterID(), fakeSceneId));
    resourceManager.requestAndUnrequestPendingClientResources();
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));

    resourceProvider.setIndexArrayAvailability(true);
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(resources, resourceManager.getRequesterID(), fakeSceneId));
    for (UInt32 i = 0u; i < 60u; ++i)
    {
        resourceManager.requestAndUnrequestPendingClientResources();
    }
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_EQ(EResourceStatus_Provided, resourceManager.getClientResourceStatus(resource));

    // no more requests after it was provided
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(resources, resourceManager.getRequesterID(), fakeSceneId)).Times(0);
    for (UInt32 i = 0u; i < 60u; ++i)
    {
        resourceManager.requestAndUnrequestPendingClientResources();
    }

    unrequestResource(resource, fakeSceneId);
}

TEST_F(ARendererResourceManager, doesNotRerequestsResourceAfterItIsUnrequested)
{
    const ResourceContentHash resource(0x00ff00ff, 0);
    ResourceContentHashVector resources;
    resources.push_back(resource);
    resourceManager.referenceClientResourcesForScene(fakeSceneId, resources);

    InSequence seq;

    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(resources, resourceManager.getRequesterID(), fakeSceneId));
    resourceManager.requestAndUnrequestPendingClientResources();
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource));

    EXPECT_CALL(resourceProvider, cancelResourceRequest(resource, _));
    unrequestResource(resource, fakeSceneId);

    // no more requests after it was unrequested
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(resources, resourceManager.getRequesterID(), fakeSceneId)).Times(0);
    for (UInt32 i = 0u; i < 10u; ++i)
    {
        resourceManager.requestAndUnrequestPendingClientResources();
    }
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
    const ResourceContentHash resource = ResourceProviderMock::FakeEffectHash;

    // request some resources
    requestResource(resource, fakeSceneId);
    resourceManager.processArrivedClientResources(nullptr);
    EXPECT_TRUE(resourceManager.hasClientResourcesToBeUploaded());
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    resourceManager.uploadAndUnloadPendingClientResources();
    EXPECT_EQ(EResourceStatus_Uploaded, resourceManager.getClientResourceStatus(resource));

    unrequestResource(resource, fakeSceneId);

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
    const RenderBuffer colorBuffer(800u, 600u, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u);
    const RenderBuffer depthBuffer(800u, 600u, ERenderBufferType_DepthBuffer, ETextureFormat_Depth24, ERenderBufferAccessMode_ReadWrite, 0u);
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
    const StreamTextureSourceId source(2);
    EXPECT_CALL(embeddedCompositingManager, uploadStreamTexture(streamTextureHandle, source, fakeSceneId));
    resourceManager.uploadStreamTexture(streamTextureHandle, source, fakeSceneId);

    //upload index data buffer
    const DataBufferHandle indexDataBufferHandle(123u);
    EXPECT_CALL(renderer.deviceMock, allocateIndexBuffer(_, _));
    resourceManager.uploadDataBuffer(indexDataBufferHandle, EDataBufferType::IndexBuffer, EDataType_Float, 10u, fakeSceneId);

    //upload vertex data buffer
    const DataBufferHandle vertexDataBufferHandle(777u);
    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(_, _));
    resourceManager.uploadDataBuffer(vertexDataBufferHandle, EDataBufferType::VertexBuffer, EDataType_Float, 10u, fakeSceneId);

    //upload texture buffer
    const TextureBufferHandle textureBufferHandle(666u);
    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(_, _, _, _, _));
    resourceManager.uploadTextureBuffer(textureBufferHandle, 1u, 2u, ETextureFormat_RGBA8, 1u, fakeSceneId);

    // unload all scene resources
    EXPECT_CALL(renderer.deviceMock, deleteRenderBuffer(_)).Times(2);
    EXPECT_CALL(renderer.deviceMock, deleteRenderTarget(_)).Times(3);
    EXPECT_CALL(embeddedCompositingManager, deleteStreamTexture(streamTextureHandle, source, fakeSceneId));
    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(_));
    EXPECT_CALL(renderer.deviceMock, deleteTexture(_));
    resourceManager.unloadAllSceneResourcesForScene(fakeSceneId);

    // Make sure the resource was deleted before the resourceManager gets out of scope
    // and deletes it automatically
    Mock::VerifyAndClearExpectations(&renderer);
}

TEST_F(ARendererResourceManager, hasResourceCacheWithoutItemAndDoesNotWantToCacheIt)
{
    InSequence seq;
    StrictMock<RendererResourceCacheMock> resourceCache;

    ResourceContentHashVector resList;
    resList.push_back(ResourceProviderMock::FakeTextureHash);
    resList.push_back(ResourceProviderMock::FakeTextureHash2);
    resourceManager.referenceClientResourcesForScene(fakeSceneId, resList);

    ON_CALL(resourceCache, hasResource(_, _)).WillByDefault(Return(false));
    EXPECT_CALL(resourceCache, hasResource(_, _)).Times(static_cast<UInt32>(resList.size()));
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(_, _, _)).Times(1);

    ON_CALL(resourceCache, shouldResourceBeCached(_, _, _, _)).WillByDefault(Return(false));
    EXPECT_CALL(resourceCache, shouldResourceBeCached(_, _, _, _)).Times(static_cast<UInt32>(resList.size()));

    rendererSceneUpdaterFlowWithCache(&resourceCache);
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, resList);
}

TEST_F(ARendererResourceManager, hasResourceCacheWithoutItemAndWantsToCacheIt)
{
    InSequence seq;
    StrictMock<RendererResourceCacheMock> resourceCache;

    ResourceContentHashVector resList;
    resList.push_back(ResourceProviderMock::FakeTextureHash);
    resList.push_back(ResourceProviderMock::FakeTextureHash2);
    resourceManager.referenceClientResourcesForScene(fakeSceneId, resList);

    ON_CALL(resourceCache, hasResource(_, _)).WillByDefault(Return(false));
    EXPECT_CALL(resourceCache, hasResource(_, _)).Times(2);
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(_, _, _)).Times(1);

    ON_CALL(resourceCache, shouldResourceBeCached(_, _, _, _)).WillByDefault(Return(true));

    EXPECT_CALL(resourceCache, shouldResourceBeCached(_, _, _, _)).Times(1);
    EXPECT_CALL(resourceCache, storeResource(_, _, _, _, _)).Times(1);
    EXPECT_CALL(resourceCache, shouldResourceBeCached(_, _, _, _)).Times(1);
    EXPECT_CALL(resourceCache, storeResource(_, _, _, _, _)).Times(1);

    rendererSceneUpdaterFlowWithCache(&resourceCache);
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, resList);
}

TEST_F(ARendererResourceManager, canStoreAndRetrieveFromResourceCache)
{
    InSequence seq;
    StrictMock<RendererResourceCacheFake> resourceCache;

    ResourceContentHashVector resList;
    resList.push_back(ResourceProviderMock::FakeTextureHash);
    resourceManager.referenceClientResourcesForScene(fakeSceneId, resList);

    EXPECT_CALL(resourceCache, hasResource(_, _)).WillOnce(Return(false));
    EXPECT_CALL(resourceProvider, requestResourceAsyncronouslyFromFramework(_, _, _)).Times(1);
    EXPECT_CALL(resourceCache, shouldResourceBeCached(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(resourceCache, storeResource(_, _, _, _, _)).Times(1);

    rendererSceneUpdaterFlowWithCache(&resourceCache);

    // Clear internal state of resource manager, so it will access the cache again
    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, resList);
    resourceManager.requestAndUnrequestPendingClientResources();
    resourceManager.referenceClientResourcesForScene(fakeSceneId, resList);

    // The resource is now cached, so it can now be retrieved
    EXPECT_CALL(resourceCache, hasResource(_, _)).WillOnce(Return(true));
    EXPECT_CALL(resourceCache, getResourceData(_, _, _)).Times(1);

    rendererSceneUpdaterFlowWithCache(&resourceCache);

    resourceManager.unreferenceClientResourcesForScene(fakeSceneId, resList);
}

TEST_F(ARendererResourceManager, canUnreferenceAllResourcesUsedByAScene)
{
    const SceneId fakeSceneId2(fakeSceneId.getValue() + 1u);
    const auto resource1 = ResourceProviderMock::FakeVertArrayHash;
    const auto resource2 = ResourceProviderMock::FakeIndexArrayHash;
    const auto resource3 = ResourceProviderMock::FakeTextureHash;

    // request some resources
    requestResource(resource1, fakeSceneId);
    requestResource(resource2, fakeSceneId);
    requestResource(resource2, fakeSceneId2, false); // second request for same resource, different scene
    requestResource(resource3, fakeSceneId);
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource1));
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource2));
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource3));

    resourceManager.unreferenceAllClientResourcesForScene(fakeSceneId);

    EXPECT_CALL(resourceProvider, cancelResourceRequest(resource1, _));
    EXPECT_CALL(resourceProvider, cancelResourceRequest(resource3, _));
    resourceManager.requestAndUnrequestPendingClientResources();
    EXPECT_EQ(EResourceStatus_Requested, resourceManager.getClientResourceStatus(resource2));

    resourceManager.unreferenceAllClientResourcesForScene(fakeSceneId2);
}
}
