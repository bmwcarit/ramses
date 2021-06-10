//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestSceneHelper.h"
#include "RendererLib/ResourceCachedScene.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/RendererScenes.h"
#include "RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "Utils/ThreadLocalLog.h"
#include <array>

namespace ramses_internal
{
    class AResourceCachedScene : public ::testing::Test
    {
    public:
        explicit AResourceCachedScene(bool indexArrayAvailable = true)
            : rendererScenes(rendererEventCollector)
            , scene(rendererScenes.createScene(SceneInfo()))
            , sceneAllocator(scene)
            , sceneHelper(scene, indexArrayAvailable)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);

            sceneAllocator.allocateDataBuffer(EDataBufferType::VertexBuffer, EDataType::Vector3Buffer, sizeof(Float) * 3, verticesDataBuffer);
            sceneAllocator.allocateTextureBuffer(ETextureFormat::R8, { { 1u, 1u } }, textureBuffer);

            ON_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(resourceNotUploadedToDevice)).WillByDefault(Return(DeviceResourceHandle::Invalid()));
            ON_CALL(sceneHelper.resourceManager, getDataBufferDeviceHandle(verticesDataBuffer, _)).WillByDefault(Return(vertexDataBufferDeviceHandle));
            ON_CALL(sceneHelper.resourceManager, getVertexArrayDeviceHandle(_, _)).WillByDefault(Return(vertexArrayDeviceHandle));
            ON_CALL(sceneHelper.resourceManager, getTextureBufferDeviceHandle(textureBuffer, _)).WillByDefault(Return(textureBufferDeviceHandle));
        }

    protected:
        void expectRenderableResourcesClean(RenderableHandle renderable, bool mustHaveIndexBuffer = true)
        {
            const bool dirtiness = scene.renderableResourcesDirty(renderable);
            EXPECT_FALSE(dirtiness);

            // if not dirty, check that all device handles are correct
            if (!dirtiness)
            {
                EXPECT_FALSE(scene.isRenderableVertexArrayDirty(renderable));

                const DeviceResourceHandle FakeShaderDeviceHandle = DeviceMock::FakeShaderDeviceHandle;
                const DeviceResourceHandle FakeTextureDeviceHandle = DeviceMock::FakeTextureDeviceHandle;
                const DeviceResourceHandle FakeRenderTargetTextureDeviceHandle = DeviceMock::FakeRenderBufferDeviceHandle;

                EXPECT_EQ(FakeShaderDeviceHandle, scene.getRenderableEffectDeviceHandle(renderable));

                EXPECT_EQ(vertexArrayDeviceHandle, scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()].deviceHandle);
                if (mustHaveIndexBuffer)
                {
                    EXPECT_TRUE(scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()].usesIndexArray);
                }

                const DataInstanceHandle uniformData = scene.getRenderable(renderable).dataInstances[ERenderableDataSlotType_Uniforms];
                const TextureSamplerHandle textureSampler = scene.getDataTextureSamplerHandle(uniformData, sceneHelper.samplerField);
                if (scene.getTextureSampler(textureSampler).contentType != TextureSampler::ContentType::RenderBuffer)
                {
                    EXPECT_EQ(FakeTextureDeviceHandle, scene.getCachedHandlesForTextureSamplers()[textureSampler.asMemoryHandle()]);
                }
                else
                {
                    EXPECT_EQ(FakeRenderTargetTextureDeviceHandle, scene.getCachedHandlesForTextureSamplers()[textureSampler.asMemoryHandle()]);
                }
            }
        }

        void expectRenderableResourcesDirty(RenderableHandle renderable)
        {
            EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        }

        void updateRenderableResources(bool withStreamTexture = false)
        {
            if (withStreamTexture)
            {
                EXPECT_CALL(sceneHelper.embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillOnce(Return(DeviceResourceHandle::Invalid()));
            }
            scene.updateRenderableResources(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
            scene.updateRenderableVertexArrays(sceneHelper.resourceManager, {});
        }

        void updateResourcesAndExpectFallbackTextureHandle(const TextureSamplerHandle textureSampler, const DeviceResourceHandle deviceHandleFromEmbeddedCompositingManager)
        {
            EXPECT_CALL(sceneHelper.embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillOnce(Return(deviceHandleFromEmbeddedCompositingManager));
            updateRenderableResources();
            scene.updateRenderableVertexArrays(sceneHelper.resourceManager, {});

            //expect correct device handle
            EXPECT_EQ(DeviceMock::FakeTextureDeviceHandle, scene.getCachedHandlesForTextureSamplers()[textureSampler.asMemoryHandle()]);
        }

        void updateResourcesAndExpectCompositedTextureHandle(const TextureSamplerHandle textureSampler, const DeviceResourceHandle deviceHandleFromEmbeddedCompositingManager, const RenderableVector& updatedRenderables)
        {
            EXPECT_CALL(sceneHelper.embeddedCompositingManager, getCompositedTextureDeviceHandleForStreamTexture(_)).WillOnce(Return(deviceHandleFromEmbeddedCompositingManager));
            scene.updateRenderableResources(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
            scene.updateRenderableVertexArrays(sceneHelper.resourceManager, updatedRenderables);

            EXPECT_EQ(deviceHandleFromEmbeddedCompositingManager, scene.getCachedHandlesForTextureSamplers()[textureSampler.asMemoryHandle()]);
        }

        void updateRenderableResourcesAndVertexArray(const RenderableVector& renderables)
        {
            scene.updateRenderableResources(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
            scene.updateRenderableVertexArrays(sceneHelper.resourceManager, renderables);
        }

        TextureSamplerHandle setResourcesAndSamplerWithStreamTexture(const RenderableHandle renderable, WaylandIviSurfaceId sourceId, ResourceContentHash fallbackTexture)
        {
            sceneAllocator.allocateStreamTexture(sourceId, fallbackTexture, sceneHelper.streamTexture);
            const TextureSamplerHandle samplerHandle = sceneHelper.createTextureSampler(sceneHelper.streamTexture);
            sceneHelper.createAndAssignUniformDataInstance(renderable, samplerHandle);
            sceneHelper.createAndAssignVertexDataInstance(renderable);
            sceneHelper.setResourcesToRenderable(renderable);

            return samplerHandle;
        }

    protected:
        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes;
        ResourceCachedScene& scene;
        SceneAllocateHelper sceneAllocator;
        TestSceneHelper sceneHelper;
        const ResourceContentHash resourceNotUploadedToDevice = ResourceContentHash(1234u, 9999u);
        const DeviceResourceHandle deviceHandleNotKnownToResourceManager = DeviceResourceHandle(4445u);
        const DataBufferHandle verticesDataBuffer{ 7u };
        const TextureBufferHandle textureBuffer{ 8u };
        const DeviceResourceHandle vertexDataBufferDeviceHandle{ 4446u };
        const DeviceResourceHandle vertexArrayDeviceHandle{ 4445u };
        const DeviceResourceHandle textureBufferDeviceHandle{ 4447u };
    };

    class AResourceCachedSceneWithoutIndexArrayAvailable : public AResourceCachedScene
    {
    public:
        AResourceCachedSceneWithoutIndexArrayAvailable() : AResourceCachedScene(false) {}
    };

    TEST_F(AResourceCachedScene, EmptyRenderableIsMarkedAsDirty)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        updateRenderableResources();

        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedScene, RenderableWithAllResourcesMarkedAsClean)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResourcesAndVertexArray({ renderable });

        expectRenderableResourcesClean(renderable);
    }

    TEST_F(AResourceCachedScene, RenderableWithNoGeometryAndNoUniformsMarkedAsDirty)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedScene, RenderableWithNoVerticesMarkedAsDirty)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, true);

        updateRenderableResources();

        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedSceneWithoutIndexArrayAvailable, RenderableWithIndicesNotAvailableMarkedAsDirty)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResources();

        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedScene, RenderableWithNoIndicesButNotUsedMarkedAsClean)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, true, false);

        updateRenderableResourcesAndVertexArray({ renderable });

        expectRenderableResourcesClean(renderable, false);
    }

    TEST_F(AResourceCachedScene, RenderableWithInvalidTextureSamplerHandleMarkedAsDirty)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, TextureSamplerHandle::Invalid());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResources();

        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedScene, CanExplicitlyMarkDirtyRenderableUsingSampler)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSamplerWithFakeTexture();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setRenderableResourcesDirtyByTextureSampler(sampler);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, CanExplicitlyMarkDirtyAllRenderablesUsingSampler)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const RenderableHandle renderable2 = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSamplerWithFakeTexture();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignUniformDataInstance(renderable2, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.createAndAssignVertexDataInstance(renderable2);
        sceneHelper.setResourcesToRenderable(renderable);
        sceneHelper.setResourcesToRenderable(renderable2);
        updateRenderableResourcesAndVertexArray({ renderable, renderable2 });
        expectRenderableResourcesClean(renderable);
        expectRenderableResourcesClean(renderable2);

        scene.setRenderableResourcesDirtyByTextureSampler(sampler);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable2));
    }

    TEST_F(AResourceCachedScene, RenderableWithAllResourcesAvailableButTextureResourceMarkedAsDirty)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSampler(resourceNotUploadedToDevice);
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedScene, RenderableWithAllResourcesAndRenderBufferMarkedClean)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createRenderTarget();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSampler(sceneHelper.renderTargetColorBuffer);
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);
    }

    TEST_F(AResourceCachedScene, MarksRenderableCleanAfterGeometryResourcesBecameValid)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, false);

        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);

        sceneHelper.setResourcesToRenderable(renderable, true, true);

        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);
    }

    TEST_F(AResourceCachedScene, MarksRenderableCleanAfterUniformResourcesBecameValid)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSampler(resourceNotUploadedToDevice));
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);

        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfNoGeometryDataInstanceAssigned)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());

        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfNoUniformDataInstanceAssigned)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignVertexDataInstance(renderable);

        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfGeometryDataInstanceIsUpdated)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const DataInstanceHandle vertexData = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, vertexData);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfUniformDataInstanceIsUpdated)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        const DataInstanceHandle uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());

        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Uniforms, uniformData);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfDataInstanceFieldIsUpdated)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const DataInstanceHandle vertexData = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setDataResource(vertexData, sceneHelper.vertAttribField, MockResourceHash::VertArrayHash2, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfDataTextureSamplerIsUpdated)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        const TextureSamplerHandle sampler = sceneHelper.createTextureSamplerWithFakeTexture();
        const DataInstanceHandle uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.setResourcesToRenderable(renderable);

        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setDataTextureSamplerHandle(uniformData, sceneHelper.samplerField, sampler);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, EmptyRenderableVectorIsNotDirty)
    {
        const RenderableVector handles;
        EXPECT_FALSE(scene.renderableResourcesDirty(handles));
    }

    TEST_F(AResourceCachedScene, ReportsNotDirtyWithSingleNotDirtyElementInRenderableVector)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });

        expectRenderableResourcesClean(renderable);
        EXPECT_FALSE(scene.renderableResourcesDirty(RenderableVector{ renderable }));
    }

    TEST_F(AResourceCachedScene, ReportsNotDirtyWithMultipleNotDirtyElementsInRenderableVector)
    {
        const RenderableHandle renderable1 = sceneHelper.createRenderable();
        const RenderableHandle renderable2 = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable1, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignUniformDataInstance(renderable2, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable1);
        sceneHelper.createAndAssignVertexDataInstance(renderable2);
        sceneHelper.setResourcesToRenderable(renderable1);
        sceneHelper.setResourcesToRenderable(renderable2);
        updateRenderableResourcesAndVertexArray({renderable1, renderable2});

        expectRenderableResourcesClean(renderable1);
        expectRenderableResourcesClean(renderable2);
        EXPECT_FALSE(scene.renderableResourcesDirty({ renderable1, renderable2 }));
    }

    TEST_F(AResourceCachedScene, ReportsDirtyWithMultipleDirtyElementsInRenderableVector)
    {
        EXPECT_TRUE(scene.renderableResourcesDirty({ sceneHelper.createRenderable(), sceneHelper.createRenderable() }));
    }

    TEST_F(AResourceCachedScene, ReportsDirtyWithMixedElementsInRenderableVector)
    {
        const RenderableHandle renderable1 = sceneHelper.createRenderable();
        const RenderableHandle renderable2 = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable1, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignUniformDataInstance(renderable2, sceneHelper.createTextureSampler(resourceNotUploadedToDevice));
        sceneHelper.createAndAssignVertexDataInstance(renderable1);
        sceneHelper.createAndAssignVertexDataInstance(renderable2);
        sceneHelper.setResourcesToRenderable(renderable1);
        sceneHelper.setResourcesToRenderable(renderable2);
        updateRenderableResourcesAndVertexArray({ renderable1, renderable2 });

        expectRenderableResourcesClean(renderable1);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable2));
        EXPECT_TRUE(scene.renderableResourcesDirty({ renderable1, renderable2 }));
    }

    TEST_F(AResourceCachedScene, updatesCacheForNewRenderTarget)
    {
        sceneHelper.createRenderTarget();
        updateRenderableResources();

        const DeviceResourceHandle deviceHandle = scene.getCachedHandlesForRenderTargets()[sceneHelper.renderTarget.asMemoryHandle()];
        EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, deviceHandle);
    }

    TEST_F(AResourceCachedScene, releasedAndRecreateRenderableWithSameHandleResetsItsDirtiness)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        const NodeHandle existingNode = scene.getRenderable(renderable).node;
        scene.releaseRenderable(renderable);
        scene.allocateRenderable(existingNode, renderable);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, releasedAndRecreateDataInstanceSetsVertexArrayDirty)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle geomDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        const DataLayoutHandle layoutHandle = scene.getLayoutOfDataInstance(geomDataInstance);
        scene.releaseDataInstance(geomDataInstance);
        scene.allocateDataInstance(layoutHandle, geomDataInstance);

        scene.updateRenderableResources(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, releasedAndRecreateTextureSamplerWithSameHandleResetsItsCache)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = sceneHelper.createTextureSamplerWithFakeTexture();
        sceneHelper.createAndAssignUniformDataInstance(renderable, textureSampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.releaseTextureSampler(textureSampler);
        scene.allocateTextureSampler({ {}, ResourceContentHash(1,2) }, textureSampler);

        EXPECT_FALSE(scene.getCachedHandlesForTextureSamplers()[textureSampler.asMemoryHandle()].isValid());
    }

    TEST_F(AResourceCachedScene, releasedAndRecreateRenderTargetWithSameHandleResetsItsCache)
    {
        sceneHelper.createRenderTarget();
        updateRenderableResources();

        DeviceResourceHandle deviceHandle = scene.getCachedHandlesForRenderTargets()[sceneHelper.renderTarget.asMemoryHandle()];
        EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, deviceHandle);

        scene.releaseRenderTarget(sceneHelper.renderTarget);
        scene.releaseRenderBuffer(sceneHelper.renderTargetColorBuffer);
        sceneHelper.createRenderTarget();
        deviceHandle = scene.getCachedHandlesForRenderTargets()[sceneHelper.renderTarget.asMemoryHandle()];
        EXPECT_FALSE(deviceHandle.isValid());

        updateRenderableResources();
        deviceHandle = scene.getCachedHandlesForRenderTargets()[sceneHelper.renderTarget.asMemoryHandle()];
        EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, deviceHandle);
    }

    TEST_F(AResourceCachedScene, releasedAndRecreateBlitPassWithSameHandleResetsItsCache)
    {
        const BlitPassHandle blitPass = sceneHelper.createBlitPassWithDummyRenderBuffers();
        updateRenderableResources();
        const UInt indexToCache = blitPass.asMemoryHandle() * 2u;
        const DeviceResourceHandle deviceHandleSrc = scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache];
        const DeviceResourceHandle deviceHandleDst = scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache + 1u];
        EXPECT_EQ(DeviceMock::FakeBlitPassRenderTargetDeviceHandle, deviceHandleSrc);
        EXPECT_EQ(DeviceMock::FakeBlitPassRenderTargetDeviceHandle, deviceHandleDst);

        const RenderBufferHandle srcRB = scene.getBlitPass(blitPass).sourceRenderBuffer;
        const RenderBufferHandle dstRB = scene.getBlitPass(blitPass).destinationRenderBuffer;
        scene.releaseBlitPass(blitPass);
        scene.allocateBlitPass(srcRB, dstRB, blitPass);

        EXPECT_FALSE(scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache].isValid());
        EXPECT_FALSE(scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache + 1u].isValid());
    }

    TEST_F(AResourceCachedScene, ResettingResourceCacheMarksAllHandlesInvalid)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const DataInstanceHandle uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.createRenderTarget();
        const BlitPassHandle blitPass = sceneHelper.createBlitPassWithDummyRenderBuffers();
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.resetResourceCache();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));

        EXPECT_FALSE(scene.getRenderableEffectDeviceHandle(renderable).isValid());
        EXPECT_FALSE(scene.getCachedHandlesForRenderTargets()[sceneHelper.renderTarget.asMemoryHandle()].isValid());
        EXPECT_FALSE(scene.getCachedHandlesForBlitPassRenderTargets()[blitPass.asMemoryHandle()].isValid());
        EXPECT_FALSE(scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()].deviceHandle.isValid());

        const TextureSamplerHandle textureSampler = scene.getDataTextureSamplerHandle(uniformData, sceneHelper.samplerField);
        EXPECT_FALSE(scene.getCachedHandlesForTextureSamplers()[textureSampler.asMemoryHandle()].isValid());
    }

    TEST_F(AResourceCachedScene, ResettingResourceCacheDoesNotAffectReleasedRenderables)
    {
        const RenderableHandle renderable1 = sceneHelper.createRenderable();
        const RenderableHandle renderable2 = sceneHelper.createRenderable();
        const DataInstanceHandle uniformData1 = sceneHelper.createAndAssignUniformDataInstance(renderable1, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignUniformDataInstance(renderable2, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexData1 = sceneHelper.createAndAssignVertexDataInstance(renderable1);
        sceneHelper.createAndAssignVertexDataInstance(renderable2);
        sceneHelper.setResourcesToRenderable(renderable1);
        sceneHelper.setResourcesToRenderable(renderable2);

        updateRenderableResources();
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable1));
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable2));

        scene.releaseDataInstance(uniformData1);
        scene.releaseDataInstance(vertexData1);
        scene.releaseRenderable(renderable1);

        scene.resetResourceCache();
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable1));
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable2));

        updateRenderableResources();
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable1));
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable2));
    }

    TEST_F(AResourceCachedScene, ResizesBlitPassesDeviceHandleCache)
    {
        sceneAllocator.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u), BlitPassHandle(15u));
        const UInt32 expectedBlitCacheSize = scene.getBlitPassCount() * 2u;
        EXPECT_EQ(expectedBlitCacheSize, scene.getCachedHandlesForBlitPassRenderTargets().size());
    }

    TEST_F(AResourceCachedScene, updatesCacheForNewBlitPasses)
    {
        const BlitPassHandle blitPass = sceneHelper.createBlitPassWithDummyRenderBuffers();

        updateRenderableResources();

        const UInt indexToCache = blitPass.asMemoryHandle() * 2u;
        const DeviceResourceHandle deviceHandleSrc = scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache];
        const DeviceResourceHandle deviceHandleDst = scene.getCachedHandlesForBlitPassRenderTargets()[indexToCache + 1u];
        EXPECT_EQ(DeviceMock::FakeBlitPassRenderTargetDeviceHandle, deviceHandleSrc);
        EXPECT_EQ(DeviceMock::FakeBlitPassRenderTargetDeviceHandle, deviceHandleDst);
    }

    //Stream texture tests for dirtiness of renderables
    TEST_F(AResourceCachedScene, CanExplicitlyMarkDirtyRenderableUsingStreamTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);
        updateRenderableResources(true);
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        scene.setRenderableResourcesDirtyByStreamTexture(sceneHelper.streamTexture);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, RenderableWithStreamTextureMarkedDirty_BySettingForceFallbackImage)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);
        updateRenderableResources(true);
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, RenderableWithStreamTextureMarkedDirty_ByUnsettingForceFallbackImage)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);
        updateRenderableResources(true);
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        //set force fallback, update and expect clean
        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateRenderableResources(true);
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        //unset
        scene.setForceFallbackImage(sceneHelper.streamTexture, false);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, RenderableWithStreamTextureMarkedDirty_DueToMissingFallbackTextureAndStreamSourceNotAvailable)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        setResourcesAndSamplerWithStreamTexture(renderable, {}, resourceNotUploadedToDevice);
        updateRenderableResources(true);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, RenderableWithStreamTextureMarkedDirty_DueToMissingFallbackTextureAndForceFallbackIsSet)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, resourceNotUploadedToDevice);

        updateResourcesAndExpectCompositedTextureHandle(textureSampler, DeviceMock::FakeTextureDeviceHandle, {renderable});
        expectRenderableResourcesClean(renderable);

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateRenderableResources(true);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    //Stream texture tests for getting correct device handles
    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_StreamSourceAvailableAndFallbackNotAvailable_UsesCompositedTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, ResourceContentHash::Invalid());

        updateResourcesAndExpectCompositedTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager, { renderable });
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_StreamSourceAvailableAndFallbackAvailable_UsesCompositedTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        updateResourcesAndExpectCompositedTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager, { renderable });
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_StreamSourceNotAvailableAndFallbackAvailable_UsesFallbackTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, DeviceResourceHandle::Invalid());
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_StreamSourceBecomesUnavailable_UsesFallbackTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        updateResourcesAndExpectCompositedTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager, { renderable });

        scene.setRenderableResourcesDirtyByStreamTexture(sceneHelper.streamTexture);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, DeviceResourceHandle::Invalid());
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_ForceFallbackIsSet_StreamSourceAvailableAndFallbackAvailable_UsesFallbackTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager);
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_ForceFallbackIsSet_StreamSourceNotAvailableAndFallbackAvailable_UsesFallbackTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, DeviceResourceHandle::Invalid());
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_ForceFallbackIsSet_AfterSourceAlreadyAvailable_UsesFallbackTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        updateResourcesAndExpectCompositedTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager, { renderable });

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager);
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_ForceFallbackIsSet_AfterSourceNotAvailable_UsesFallbackTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        updateResourcesAndExpectFallbackTextureHandle(textureSampler, DeviceResourceHandle::Invalid());

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, DeviceResourceHandle::Invalid()); //no change
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_ForceFallbackIsUnset_SourceAvailable_UsesCompositedTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager);

        scene.setForceFallbackImage(sceneHelper.streamTexture, false);
        updateResourcesAndExpectCompositedTextureHandle(textureSampler, deviceHandleNotKnownToResourceManager, { renderable });
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForSamplerWithStreamTexture_ForceFallbackIsUnset_SourceNotAvailable_UsesFallbackTexture)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle textureSampler = setResourcesAndSamplerWithStreamTexture(renderable, {}, MockResourceHash::TextureHash);

        scene.setForceFallbackImage(sceneHelper.streamTexture, true);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, DeviceResourceHandle::Invalid());

        scene.setForceFallbackImage(sceneHelper.streamTexture, false);
        updateResourcesAndExpectFallbackTextureHandle(textureSampler, DeviceResourceHandle::Invalid()); //no change
    }

    //vertex arrays
    TEST_F(AResourceCachedScene, CanGetDeviceHandleForVertexArray)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });

        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        EXPECT_FALSE(scene.isRenderableVertexArrayDirty(renderable));
        const auto& vertexArrayEntry = scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()];
        EXPECT_EQ(vertexArrayDeviceHandle, vertexArrayEntry.deviceHandle);
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForVertexArray_AfterSwitchingFromNonExistingDataBufferResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, true);
        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);

        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        const auto& vertexArrayEntry = scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()];
        EXPECT_EQ(vertexArrayDeviceHandle, vertexArrayEntry.deviceHandle);
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForVertexArray_AfterSwitchingFromAnExistingDataBufferResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        const auto& vertexArrayEntry = scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()];
        EXPECT_EQ(vertexArrayDeviceHandle, vertexArrayEntry.deviceHandle);
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfDataBufferIsChanged)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, true);
        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        const DataBufferHandle dummyDataBuffer(1234u);
        ASSERT_NE(dummyDataBuffer, verticesDataBuffer);
        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), dummyDataBuffer, 0u, 0u, 0u);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));

        scene.updateRenderableVertexArrays(sceneHelper.resourceManager, {});
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForVertexArray_DoesNotChangeHandleIfContentOfDataBufferIsUpdated)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, true);
        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);

        scene.updateRenderableResources(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));

        scene.updateRenderableVertexArrays(sceneHelper.resourceManager, { renderable });
        EXPECT_FALSE(scene.isRenderableVertexArrayDirty(renderable));

        scene.updateDataBuffer(verticesDataBuffer, sizeof(Float) * 3, 0u, reinterpret_cast<Byte*>(std::array<Float, 3>().data()));
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        EXPECT_FALSE(scene.isRenderableVertexArrayDirty(renderable));

        updateRenderableResources();
        const auto& vertexArrayEntry = scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()];
        EXPECT_EQ(vertexArrayDeviceHandle, vertexArrayEntry.deviceHandle);
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyAfterSwitchingFromDataBufferToNonExistingResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, true);
        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, resourceNotUploadedToDevice, DataBufferHandle::Invalid(), 0u, 0u, 0u);

        scene.updateRenderableResources(sceneHelper.resourceManager, sceneHelper.embeddedCompositingManager);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));
    }

    TEST_F(AResourceCachedScene, MarkVertexArrayDirtyAfterStartVertexIsUpdated)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, true);
        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setRenderableStartVertex(renderable, 10u);
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));
    }

    TEST_F(AResourceCachedScene, ExpectValidVertexArrayHandleAfterSwitchingFromDataBufferToExistingResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable, false, true);
        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, MockResourceHash::VertArrayHash, DataBufferHandle::Invalid(), 0u, 0u, 0u);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        const auto& vertexArrayEntry = scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()];
        EXPECT_EQ(vertexArrayDeviceHandle, vertexArrayEntry.deviceHandle);
    }

    TEST_F(AResourceCachedScene, MarksVertexArrayDirtyIfRenderableReleased)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.releaseRenderable(renderable);
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));

        //next call to update cleans up dirty VAO
        updateRenderableResourcesAndVertexArray({ renderable });
        EXPECT_FALSE(scene.isRenderableVertexArrayDirty(renderable));
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForVertexArray_IfUniformDataInstanceUpdaterAfterGeomDataInstance)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const DataInstanceHandle vertexDataInstance = sceneHelper.createAndAssignVertexDataInstance(renderable);

        sceneHelper.setResourcesToRenderable(renderable, false);
        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));

        scene.setDataResource(vertexDataInstance, sceneHelper.vertAttribField, ResourceContentHash::Invalid(), verticesDataBuffer, 0u, 0u, 0u);
        updateRenderableResources();
        //stays dirty
        expectRenderableResourcesDirty(renderable);
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));

        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        const auto& vertexArrayEntry = scene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()];
        EXPECT_EQ(vertexArrayDeviceHandle, vertexArrayEntry.deviceHandle);
    }

    //texture buffers
    TEST_F(AResourceCachedScene, CanGetDeviceHandleForTextureBuffer)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSampler(textureBuffer);
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResources();

        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        const DeviceResourceHandle deviceHandle = scene.getCachedHandlesForTextureSamplers()[sampler.asMemoryHandle()];
        EXPECT_EQ(textureBufferDeviceHandle, deviceHandle);
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForTextureBuffer_AfterSwitchingFromNonExistingResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle samplerInvalid = sceneHelper.createTextureSampler(resourceNotUploadedToDevice);
        const TextureSamplerHandle samplerTexBuff = sceneHelper.createTextureSampler(textureBuffer);
        const auto uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, samplerInvalid);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResources();
        expectRenderableResourcesDirty(renderable);

        scene.setDataTextureSamplerHandle(uniformData, sceneHelper.samplerField, samplerTexBuff);
        updateRenderableResources();

        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        const DeviceResourceHandle deviceHandle = scene.getCachedHandlesForTextureSamplers()[samplerTexBuff.asMemoryHandle()];
        EXPECT_EQ(textureBufferDeviceHandle, deviceHandle);
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForTextureBuffer_AfterSwitchingFromAnExistingResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle samplerTexRes = sceneHelper.createTextureSamplerWithFakeTexture();
        const TextureSamplerHandle samplerTexBuff = sceneHelper.createTextureSampler(textureBuffer);
        const auto uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, samplerTexRes);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        scene.setDataTextureSamplerHandle(uniformData, sceneHelper.samplerField, samplerTexBuff);
        updateRenderableResources();

        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        const DeviceResourceHandle deviceHandle = scene.getCachedHandlesForTextureSamplers()[samplerTexBuff.asMemoryHandle()];
        EXPECT_EQ(textureBufferDeviceHandle, deviceHandle);
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyIfTextureBufferIsChanged)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSampler(textureBuffer);
        const auto uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        const TextureBufferHandle dummyTextureBuffer(1234u);
        ASSERT_NE(dummyTextureBuffer, textureBuffer);
        const auto sampler2 = sceneHelper.createTextureSampler(dummyTextureBuffer);
        scene.setDataTextureSamplerHandle(uniformData, sceneHelper.samplerField, sampler2);
        scene.updateRenderablesResourcesDirtiness();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, CanGetDeviceHandleForTextureBuffer_DoesNotChangeHandleIfContentOfTextureBufferIsUpdated)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSampler(textureBuffer);
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        scene.updateTextureBuffer(textureBuffer, 0u, 0u, 0u, 1u, 1u, std::array<Byte, 1>().data());
        updateRenderableResources();

        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));
        const DeviceResourceHandle deviceHandle = scene.getCachedHandlesForTextureSamplers()[sampler.asMemoryHandle()];
        EXPECT_EQ(textureBufferDeviceHandle, deviceHandle);
    }

    TEST_F(AResourceCachedScene, MarksRenderableDirtyAfterSwitchingFromTextureBufferToNonExistingResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSampler(textureBuffer);
        const auto uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResources();
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        const TextureSamplerHandle samplerInvalid = sceneHelper.createTextureSampler(resourceNotUploadedToDevice);
        scene.setDataTextureSamplerHandle(uniformData, sceneHelper.samplerField, samplerInvalid);
        updateRenderableResources();
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, GetsCorrectDeviceHandleAfterSwitchingFromTextureBufferToExistingResource)
    {
        const RenderableHandle renderable = sceneHelper.createRenderable();
        const TextureSamplerHandle sampler = sceneHelper.createTextureSampler(textureBuffer);
        const auto uniformData = sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        EXPECT_FALSE(scene.renderableResourcesDirty(renderable));

        const TextureSamplerHandle samplerRes = sceneHelper.createTextureSamplerWithFakeTexture();
        scene.setDataTextureSamplerHandle(uniformData, sceneHelper.samplerField, samplerRes);
        updateRenderableResources();

        expectRenderableResourcesClean(renderable);
        const DeviceResourceHandle deviceHandle = scene.getCachedHandlesForTextureSamplers()[samplerRes.asMemoryHandle()];
        EXPECT_EQ(DeviceMock::FakeTextureDeviceHandle, deviceHandle);
    }

    TEST_F(AResourceCachedScene, willForceUpdateCacheIfDataInstanceAssignedToAnotherRenderable)
    {
        const auto renderable = sceneHelper.createRenderable();
        const auto sampler = sceneHelper.createTextureSamplerWithFakeTexture();
        const auto uniforms = sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        const auto geom = sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);
        scene.releaseRenderable(renderable);

        // Resource cache is currently not used by any renderable and the resources can potentially be unloaded.
        // Cache must be updated if another renderable starts using the same resources.

        const auto renderable2 = sceneHelper.createRenderable();
        scene.setRenderableDataInstance(renderable2, ERenderableDataSlotType_Geometry, geom);
        scene.setRenderableDataInstance(renderable2, ERenderableDataSlotType_Uniforms, uniforms);
        expectRenderableResourcesDirty(renderable2);

        // test upcoming calls explicitly
        Mock::VerifyAndClearExpectations(&sceneHelper.resourceManager);
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::EffectHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::TextureHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::IndexArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::VertArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getVertexArrayDeviceHandle(renderable2, scene.getSceneId()));
        updateRenderableResourcesAndVertexArray({ renderable2 });
        expectRenderableResourcesClean(renderable2);
    }

    TEST_F(AResourceCachedScene, willNotUpdateCacheIfRenderableVisibilityOff)
    {
        const auto renderable = sceneHelper.createRenderable();
        const auto sampler = sceneHelper.createTextureSamplerWithFakeTexture();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);

        // no caching for OFF renderable
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(_)).Times(0);
        updateRenderableResources();
        // renderable stays dirty
        expectRenderableResourcesDirty(renderable);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
    }

    TEST_F(AResourceCachedScene, willForceUpdateCacheIfRenderableVisibilityModeChangesFromOffToVisible)
    {
        const auto renderable = sceneHelper.createRenderable();
        const auto sampler = sceneHelper.createTextureSamplerWithFakeTexture();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        // set off, no change
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        expectRenderableResourcesClean(renderable);

        // Resource cache is currently not used by any active renderable and the resources can potentially be unloaded.
        // Cache must be updated if renderable using the same resources gets visible again.

        // set visible will trigger update of resources
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        expectRenderableResourcesDirty(renderable);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        EXPECT_TRUE(scene.isRenderableVertexArrayDirty(renderable));

        // test upcoming calls explicitly
        Mock::VerifyAndClearExpectations(&sceneHelper.resourceManager);
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::EffectHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::TextureHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::IndexArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::VertArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getVertexArrayDeviceHandle(renderable, scene.getSceneId()));
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);
    }

    TEST_F(AResourceCachedScene, willForceUpdateCacheIfRenderableVisibilityModeChangesFromOffToInvisibleOrVisible)
    {
        const auto renderable = sceneHelper.createRenderable();
        const auto sampler = sceneHelper.createTextureSamplerWithFakeTexture();
        sceneHelper.createAndAssignUniformDataInstance(renderable, sampler);
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);

        // test upcoming calls explicitly
        Mock::VerifyAndClearExpectations(&sceneHelper.resourceManager);

        // no caching for OFF renderable
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(_)).Times(0);
        updateRenderableResources();
        // renderable stays dirty
        expectRenderableResourcesDirty(renderable);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));

        // set invisible will trigger update of resources
        scene.setRenderableVisibility(renderable, EVisibilityMode::Invisible);
        expectRenderableResourcesDirty(renderable);
        EXPECT_TRUE(scene.renderableResourcesDirty(renderable));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::EffectHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::TextureHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::IndexArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::VertArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getVertexArrayDeviceHandle(renderable, scene.getSceneId()));
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);

        // back to OFF
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        updateRenderableResources();

        // set visible will trigger update of resources
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        expectRenderableResourcesDirty(renderable);
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::EffectHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::TextureHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::IndexArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getResourceDeviceHandle(MockResourceHash::VertArrayHash));
        EXPECT_CALL(sceneHelper.resourceManager, getVertexArrayDeviceHandle(renderable, scene.getSceneId()));
        updateRenderableResourcesAndVertexArray({ renderable });
        expectRenderableResourcesClean(renderable);
    }

    TEST_F(AResourceCachedScene, vaoDirtyOnRenderableAllocationAndRelease)
    {
        const auto renderable = sceneHelper.createRenderable();
        EXPECT_TRUE(scene.hasDirtyVertexArrays());
        EXPECT_TRUE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        EXPECT_FALSE(scene.hasDirtyVertexArrays());
        EXPECT_FALSE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        scene.releaseRenderable(renderable);
        EXPECT_TRUE(scene.hasDirtyVertexArrays());
        EXPECT_TRUE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);
    }

    TEST_F(AResourceCachedScene, vaoDirtyOnRenderableVisibilityChangeFromOff)
    {
        const auto renderable = sceneHelper.createRenderable();
        EXPECT_TRUE(scene.hasDirtyVertexArrays());
        EXPECT_TRUE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        sceneHelper.createAndAssignUniformDataInstance(renderable, sceneHelper.createTextureSamplerWithFakeTexture());
        sceneHelper.createAndAssignVertexDataInstance(renderable);
        sceneHelper.setResourcesToRenderable(renderable);
        updateRenderableResourcesAndVertexArray({ renderable });
        EXPECT_FALSE(scene.hasDirtyVertexArrays());
        EXPECT_FALSE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        // no change
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        EXPECT_FALSE(scene.hasDirtyVertexArrays());
        EXPECT_FALSE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        // dirty
        scene.setRenderableVisibility(renderable, EVisibilityMode::Invisible);
        EXPECT_TRUE(scene.hasDirtyVertexArrays());
        EXPECT_TRUE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);
        updateRenderableResourcesAndVertexArray({ renderable });
        EXPECT_FALSE(scene.hasDirtyVertexArrays());
        EXPECT_FALSE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        // no change
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        EXPECT_FALSE(scene.hasDirtyVertexArrays());
        EXPECT_FALSE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        // no change
        scene.setRenderableVisibility(renderable, EVisibilityMode::Invisible);
        EXPECT_FALSE(scene.hasDirtyVertexArrays());
        EXPECT_FALSE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        // no change
        scene.setRenderableVisibility(renderable, EVisibilityMode::Off);
        EXPECT_FALSE(scene.hasDirtyVertexArrays());
        EXPECT_FALSE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);

        // dirty
        scene.setRenderableVisibility(renderable, EVisibilityMode::Visible);
        EXPECT_TRUE(scene.hasDirtyVertexArrays());
        EXPECT_TRUE(scene.getVertexArraysDirtinessFlags()[renderable.asMemoryHandle()]);
    }
}
