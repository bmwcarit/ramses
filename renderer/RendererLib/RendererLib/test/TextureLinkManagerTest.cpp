//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "SceneAPI/TextureSampler.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/ResourceCachedScene.h"
#include "RendererEventCollector.h"
#include "TestSceneHelper.h"
#include "SceneAllocateHelper.h"
#include "MockResourceHash.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal {
using namespace testing;

// Tests also TextureLinkCachedScene class

class ATextureLinkManager : public ::testing::Test
{
public:
    ATextureLinkManager()
        : rendererScenes(rendererEventCollector)
        , sceneLinksManager(rendererScenes.getSceneLinksManager())
        , textureLinkManager(sceneLinksManager.getTextureLinkManager())
        , providerScene(rendererScenes.createScene(SceneInfo(providerSceneId)))
        , consumerScene(rendererScenes.createScene(SceneInfo(consumerSceneId)))
        , providerSceneAllocator(providerScene)
        , consumerSceneAllocator(consumerScene)
        , sceneHelper(consumerScene)
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);

        consumerSceneAllocator.allocateTextureSampler({ {}, consumerTextureHash }, sampler);
        consumerSceneAllocator.allocateTextureSampler({ {}, consumerTextureHash }, sampler2);
        consumerSceneAllocator.allocateTextureSampler({ {}, consumerTextureHash }, sampler3);
        consumerSceneAllocator.allocateTextureSampler({ {}, TextureSampler::ContentType::ExternalTexture, {}, InvalidMemoryHandle }, samplerExternal);

        providerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureProvider, providerId, NodeHandle(), DataInstanceHandle::Invalid(), providerTextureHash, TextureSamplerHandle() }, providerSlotHandle);
        expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, providerSceneId, providerId, SceneId(0u), DataSlotId(0u));
        consumerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureConsumer, consumerId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler }, consumerSlotHandle);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId);
        consumerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureConsumer, consumerId2, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler2 }, consumerSlotHandle2);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);
        consumerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureConsumer, consumerId3, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), sampler3 }, consumerSlotHandle3);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId3);

        consumerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureConsumer, consumerExternalId, NodeHandle(), DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), samplerExternal }, consumerExternalSlotHandle);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerExternalId);
        providerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureProvider, providerExternalId, NodeHandle(), DataInstanceHandle::Invalid(), {}, TextureSamplerHandle() }, providerExternalSlotHandle);
        expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, providerSceneId, providerExternalId, SceneId(0u), DataSlotId(0u));

        renderable = createRenderableWithResourcesAndMakeClean(sampler);
        // create a renderable with sampler2 and sampler3 as well so it is used for device handle caching
        createRenderableWithResourcesAndMakeClean(sampler2);
        createRenderableWithResourcesAndMakeClean(sampler3);

        renderableWithExternalTexture =  createRenderableWithExternalTextureSampler(samplerExternal);
    }

protected:
    RendererEvent expectRendererEvent(ERendererEventType type)
    {
        RendererEventVector events;
        RendererEventVector dummy;
        rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
        EXPECT_EQ(1u, events.size());
        if (events.size() == 1u)
        {
            RendererEvent event = events.front();
            EXPECT_EQ(type, event.eventType);
            return event;
        }
        return {};
    }

    void expectRendererEvent(ERendererEventType type, SceneId providerSId, DataSlotId pId, SceneId consumerSId, DataSlotId cId)
    {
        const RendererEvent event = expectRendererEvent(type);
        EXPECT_EQ(providerSId, event.providerSceneId);
        EXPECT_EQ(consumerSId, event.consumerSceneId);
        EXPECT_EQ(pId, event.providerdataId);
        EXPECT_EQ(cId, event.consumerdataId);
    }

    void expectRendererEvent(ERendererEventType type, SceneId consumerSId, DataSlotId cId, SceneId providerSId)
    {
        const RendererEvent event = expectRendererEvent(type);
        EXPECT_EQ(consumerSId, event.consumerSceneId);
        EXPECT_EQ(cId, event.consumerdataId);
        EXPECT_EQ(providerSId, event.providerSceneId);
    }

    void expectRendererEvent(ERendererEventType type, OffscreenBufferHandle buffer, SceneId consumerSId, DataSlotId cId)
    {
        const RendererEvent event = expectRendererEvent(type);
        EXPECT_EQ(buffer, event.offscreenBuffer);
        EXPECT_FALSE(event.streamBuffer.isValid());
        EXPECT_FALSE(event.externalBuffer.isValid());
        EXPECT_EQ(consumerSId, event.consumerSceneId);
        EXPECT_EQ(cId, event.consumerdataId);
    }

    void expectRendererEvent(ERendererEventType type, StreamBufferHandle buffer, SceneId consumerSId, DataSlotId cId)
    {
        const RendererEvent event = expectRendererEvent(type);
        EXPECT_EQ(buffer, event.streamBuffer);
        EXPECT_FALSE(event.offscreenBuffer.isValid());
        EXPECT_FALSE(event.externalBuffer.isValid());
        EXPECT_EQ(consumerSId, event.consumerSceneId);
        EXPECT_EQ(cId, event.consumerdataId);
    }

    void expectRendererEvent(ERendererEventType type, ExternalBufferHandle buffer, SceneId consumerSId, DataSlotId cId)
    {
        const RendererEvent event = expectRendererEvent(type);
        EXPECT_EQ(buffer, event.externalBuffer);
        EXPECT_FALSE(event.offscreenBuffer.isValid());
        EXPECT_FALSE(event.streamBuffer.isValid());
        EXPECT_EQ(consumerSId, event.consumerSceneId);
        EXPECT_EQ(cId, event.consumerdataId);
    }

    void updateConsumerSceneResourcesCache()
    {
        consumerScene.updateRenderableResources(sceneHelper.resourceManager);
    }

    RenderableHandle createRenderableWithResourcesAndMakeClean(TextureSamplerHandle samplerHandle)
    {
        const RenderableHandle renderableHandle = sceneHelper.createRenderable();
        const DataInstanceHandle uniformData = consumerSceneAllocator.allocateDataInstance(sceneHelper.testUniformLayout);
        sceneHelper.m_scene.setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, uniformData);
        sceneHelper.m_scene.setDataTextureSamplerHandle(uniformData, sceneHelper.samplerField, samplerHandle);
        sceneHelper.createAndAssignVertexDataInstance(renderableHandle);
        sceneHelper.setResourcesToRenderable(renderableHandle);
        updateConsumerSceneResourcesCache();
        EXPECT_FALSE(consumerScene.renderableResourcesDirty(renderableHandle));
        return renderableHandle;
    }

    RenderableHandle createRenderableWithExternalTextureSampler(TextureSamplerHandle samplerHandle)
    {
        const RenderableHandle renderableHandle = sceneHelper.createRenderable();
        DataFieldInfoVector uniformDataFields(1u);
        constexpr DataFieldHandle sampleDataField{ 0u };
        uniformDataFields[sampleDataField.asMemoryHandle()] = DataFieldInfo(EDataType::TextureSamplerExternal);
        const auto dataLayout = consumerSceneAllocator.allocateDataLayout(uniformDataFields, MockResourceHash::EffectHash);
        const auto uniformDataInstance = consumerSceneAllocator.allocateDataInstance(dataLayout);
        sceneHelper.m_scene.setRenderableDataInstance(renderableHandle, ERenderableDataSlotType_Uniforms, uniformDataInstance);
        sceneHelper.m_scene.setDataTextureSamplerHandle(uniformDataInstance, sampleDataField, samplerHandle);
        sceneHelper.createAndAssignVertexDataInstance(renderableHandle);
        sceneHelper.setResourcesToRenderable(renderableHandle);

        updateConsumerSceneResourcesCache();
        return renderableHandle;
    }

    void expectNoTextureLink()
    {
        EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
        EXPECT_EQ(TextureSampler::ContentType::ClientTexture, consumerScene.getTextureSampler(sampler).contentType);
        EXPECT_EQ(consumerTextureHash, consumerScene.getTextureSampler(sampler).textureResource);
    }

    void expectTextureLink(ResourceContentHash hash)
    {
        EXPECT_TRUE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
        EXPECT_EQ(hash, textureLinkManager.getLinkedTexture(consumerSceneId, sampler));
        EXPECT_EQ(TextureSampler::ContentType::ClientTexture, consumerScene.getTextureSampler(sampler).contentType);
        EXPECT_EQ(hash, consumerScene.getTextureSampler(sampler).textureResource);
    }

    void expectNoBufferLink(TextureSamplerHandle samplerHandle)
    {
        EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, samplerHandle));
        EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, samplerHandle));
        EXPECT_FALSE(textureLinkManager.hasLinkedExternalBuffer(consumerSceneId, samplerHandle));
        updateConsumerSceneResourcesCache();
        if (consumerScene.getTextureSampler(samplerHandle).contentType == TextureSampler::ContentType::ExternalTexture)
        {
            // sampler uses empty external textrue
            EXPECT_EQ(DeviceMock::FakeEmptyExternalTextureDeviceHandle, consumerScene.getCachedHandlesForTextureSamplers()[samplerHandle.asMemoryHandle()]);
        }
        else
        {
            // sampler uses fallback texture
            EXPECT_EQ(DeviceMock::FakeTextureDeviceHandle, consumerScene.getCachedHandlesForTextureSamplers()[samplerHandle.asMemoryHandle()]);
        }
    }

    void updateCacheAndExpectDeviceHandles(TextureSamplerHandle obSampler, TextureSamplerHandle sbSampler, TextureSamplerHandle ebSampler)
    {
        if (obSampler.isValid())
            EXPECT_CALL(sceneHelper.resourceManager, getOffscreenBufferColorBufferDeviceHandle(providerOffscreenBuffer));
        if (sbSampler.isValid())
            EXPECT_CALL(sceneHelper.resourceManager, getStreamBufferDeviceHandle(providerStreamBuffer));
        if(ebSampler.isValid())
            EXPECT_CALL(sceneHelper.resourceManager, getExternalBufferDeviceHandle(providerExternalBuffer)).WillOnce(Return(DeviceMock::FakeExternalTextureDeviceHandle));

        updateConsumerSceneResourcesCache();

        if (obSampler.isValid())
        {
            EXPECT_EQ(DeviceMock::FakeRenderBufferDeviceHandle, consumerScene.getCachedHandlesForTextureSamplers()[obSampler.asMemoryHandle()]);
        }
        if (sbSampler.isValid())
        {
            EXPECT_EQ(DeviceMock::FakeRenderTargetDeviceHandle, consumerScene.getCachedHandlesForTextureSamplers()[sbSampler.asMemoryHandle()]);
        }

        if (ebSampler.isValid())
        {
            EXPECT_EQ(DeviceMock::FakeExternalTextureDeviceHandle, consumerScene.getCachedHandlesForTextureSamplers()[ebSampler.asMemoryHandle()]);
        }
    }

    void expectOffscreenBufferLink(TextureSamplerHandle samplerHandle, bool withResourceCacheUpdate = true)
    {
        EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, samplerHandle));
        EXPECT_FALSE(textureLinkManager.hasLinkedExternalBuffer(consumerSceneId, samplerHandle));
        EXPECT_TRUE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, samplerHandle));
        EXPECT_EQ(providerOffscreenBuffer, textureLinkManager.getLinkedOffscreenBuffer(consumerSceneId, samplerHandle));
        EXPECT_EQ(TextureSampler::ContentType::OffscreenBuffer, consumerScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(providerOffscreenBuffer.asMemoryHandle(), consumerScene.getTextureSampler(samplerHandle).contentHandle);

        if (withResourceCacheUpdate)
            updateCacheAndExpectDeviceHandles(samplerHandle, {}, {});
    }

    void expectStreamBufferLink(TextureSamplerHandle samplerHandle, bool withResourceCacheUpdate = true)
    {
        EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, samplerHandle));
        EXPECT_FALSE(textureLinkManager.hasLinkedExternalBuffer(consumerSceneId, samplerHandle));
        EXPECT_TRUE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, samplerHandle));
        EXPECT_EQ(providerStreamBuffer, textureLinkManager.getLinkedStreamBuffer(consumerSceneId, samplerHandle));
        EXPECT_EQ(TextureSampler::ContentType::StreamBuffer, consumerScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(providerStreamBuffer.asMemoryHandle(), consumerScene.getTextureSampler(samplerHandle).contentHandle);

        if (withResourceCacheUpdate)
            updateCacheAndExpectDeviceHandles({}, samplerHandle, {});
    }

    void expectExternalBufferLink(TextureSamplerHandle samplerHandle, bool withResourceCacheUpdate = true)
    {
        EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, samplerHandle));
        EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, samplerHandle));
        EXPECT_TRUE(textureLinkManager.hasLinkedExternalBuffer(consumerSceneId, samplerHandle));
        EXPECT_EQ(providerExternalBuffer, textureLinkManager.getLinkedExternalBuffer(consumerSceneId, samplerHandle));
        EXPECT_EQ(TextureSampler::ContentType::ExternalTexture, consumerScene.getTextureSampler(samplerHandle).contentType);
        EXPECT_EQ(providerExternalBuffer.asMemoryHandle(), consumerScene.getTextureSampler(samplerHandle).contentHandle);

        if (withResourceCacheUpdate)
            updateCacheAndExpectDeviceHandles({}, {}, samplerHandle);
    }

    void expectRenderableDirtinessState(RenderableHandle handle, bool dirty)
    {
        consumerScene.updateRenderablesResourcesDirtiness();
        EXPECT_EQ(dirty, consumerScene.renderableResourcesDirty(handle));
    }

    void setFallbackTextureToConsumerSampler(TextureSamplerHandle samplerHandle, ResourceContentHash resource, RenderBufferHandle renderBufferHandle)
    {
        DataSlotId dataSlotId;
        DataSlotHandle dataSlot;
        for (DataSlotHandle d(0u); d < consumerScene.getDataSlotCount(); ++d)
        {
            if (consumerScene.isDataSlotAllocated(d) && consumerScene.getDataSlot(d).attachedTextureSampler == samplerHandle)
            {
                dataSlot = d;
                dataSlotId = consumerScene.getDataSlot(d).id;
            }
        }
        if (dataSlot.isValid())
        {
            consumerScene.releaseDataSlot(dataSlot);
            expectRendererEvent(ERendererEventType::SceneDataSlotConsumerDestroyed);
        }

        if (resource.isValid())
            sceneHelper.recreateSamplerWithDifferentContent(samplerHandle, resource);
        else if (renderBufferHandle.isValid())
            sceneHelper.recreateSamplerWithDifferentContent(samplerHandle, renderBufferHandle);

        if (dataSlot.isValid())
        {
            consumerScene.allocateDataSlot({ EDataSlotType_TextureConsumer, dataSlotId, {}, {}, {}, samplerHandle }, dataSlot);
            expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated);
        }
    }

    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    SceneLinksManager& sceneLinksManager;
    const TextureLinkManager& textureLinkManager;
    const SceneId providerSceneId{ 3u };
    const SceneId consumerSceneId{ 4u };
    IScene& providerScene;
    ResourceCachedScene& consumerScene;
    SceneAllocateHelper providerSceneAllocator;
    SceneAllocateHelper consumerSceneAllocator;

    TestSceneHelper sceneHelper;
    RenderableHandle renderable;
    RenderableHandle renderableWithExternalTexture;

    const OffscreenBufferHandle providerOffscreenBuffer{ 333u };
    const StreamBufferHandle providerStreamBuffer{ 444u };
    const ExternalBufferHandle providerExternalBuffer{ 555u };
    const DataSlotHandle providerSlotHandle{ 55u };
    const DataSlotHandle providerExternalSlotHandle{ 56u };
    const DataSlotHandle consumerSlotHandle{ 66u };
    const DataSlotHandle consumerSlotHandle2{ 77u };
    const DataSlotHandle consumerSlotHandle3{ 88u };
    const DataSlotHandle consumerExternalSlotHandle{ 89u };
    const DataSlotId providerId{ 33u };
    const DataSlotId providerExternalId{ 34u };
    const DataSlotId consumerId{ 44u };
    const DataSlotId consumerId2{ 45u };
    const DataSlotId consumerId3{ 46u };
    const DataSlotId consumerExternalId{ 47u };

    const TextureSamplerHandle sampler{ 3u };
    const TextureSamplerHandle sampler2{ 5u };
    const TextureSamplerHandle sampler3{ 7u };
    const TextureSamplerHandle samplerExternal{ 71u };
    const ResourceContentHash providerTextureHash{ MockResourceHash::TextureHash2 };
    const ResourceContentHash consumerTextureHash{ MockResourceHash::TextureHash };
};

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerInitially)
{
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsLinkForSamplerWhenLinked)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    expectTextureLink(providerTextureHash);
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndUnlinked)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId, providerSceneId);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndProviderSceneRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    rendererScenes.destroyScene(providerSceneId);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSceneRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    rendererScenes.destroyScene(consumerSceneId);
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndProviderSlotRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    providerScene.releaseDataSlot(providerSlotHandle);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSlotRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    consumerScene.releaseDataSlot(consumerSlotHandle);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndProviderSceneUnmapped)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    sceneLinksManager.handleSceneUnmapped(providerSceneId);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSceneUnmapped)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    sceneLinksManager.handleSceneUnmapped(consumerSceneId);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenLinked)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenUnlinked)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenConsumerSceneRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.handleSceneRemoved(consumerSceneId);
    expectRenderableDirtinessState(renderable, true);
    expectNoTextureLink();
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenConsumerSceneUnmapped)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.handleSceneUnmapped(consumerSceneId);
    expectRenderableDirtinessState(renderable, true);
    expectNoTextureLink();
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenProviderSceneRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.handleSceneRemoved(providerSceneId);
    expectRenderableDirtinessState(renderable, true);
    expectNoTextureLink();
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenProviderSceneUnmapped)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.handleSceneUnmapped(providerSceneId);
    expectRenderableDirtinessState(renderable, true);
    expectNoTextureLink();
}

TEST_F(ATextureLinkManager, doesNotMarkRenderableUsingSamplerDirtyWhenUnlinkedWithoutBeingLinked)
{
    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinkFailed, consumerSceneId, consumerId, SceneId::Invalid());
    expectRenderableDirtinessState(renderable, false);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenLinkedAndProviderSceneRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    rendererScenes.destroyScene(providerSceneId);
    expectRenderableDirtinessState(renderable, true);
    expectNoTextureLink();
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenLinkedAndProviderSlotRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    providerScene.releaseDataSlot(providerSlotHandle);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenLinkedAndConsumerSlotRemoved)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    consumerScene.releaseDataSlot(consumerSlotHandle);
    expectRenderableDirtinessState(renderable, true);
}

// buffer links tests
TEST_F(ATextureLinkManager, reportsLinkForSamplerWhenLinkedBuffer_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);
    expectOffscreenBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsLinkForSamplerWhenLinkedBuffer_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);
    expectStreamBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsLinkForSamplerWhenLinkedBuffer_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectExternalBufferLink(samplerExternal);
    EXPECT_FALSE(consumerScene.renderableResourcesDirty(renderableWithExternalTexture));
}

TEST_F(ATextureLinkManager, reportsLinkFailedWhenLinkingToUnknownConsumerSlot_OB)
{
    constexpr DataSlotId unknownDataId{ 131313u };
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, unknownDataId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinkFailed, providerOffscreenBuffer, consumerSceneId, unknownDataId);
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsLinkFailedWhenLinkingToUnknownConsumerSlot_SB)
{
    constexpr DataSlotId unknownDataId{ 131313u };
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, unknownDataId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinkFailed, providerStreamBuffer, consumerSceneId, unknownDataId);
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsLinkFailedWhenLinkingToUnknownConsumerSlot_EB)
{
    constexpr DataSlotId unknownDataId{ 131313u };
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, unknownDataId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinkFailed, providerExternalBuffer, consumerSceneId, unknownDataId);
    expectNoBufferLink(samplerExternal);
}

TEST_F(ATextureLinkManager, reportsLinkFailedWhenLinkingToConsumerSlotWithWrongType_OB)
{
    // attempt to link to a provider type slot
    constexpr DataSlotId providerDataId{ 131313u };
    consumerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureProvider, providerDataId, NodeHandle(), DataInstanceHandle::Invalid(), providerTextureHash, TextureSamplerHandle() });
    expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, consumerSceneId, providerDataId, SceneId(0u), DataSlotId(0u));

    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, providerDataId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinkFailed, providerOffscreenBuffer, consumerSceneId, providerDataId);
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsLinkFailedWhenLinkingToConsumerSlotWithWrongType_SB)
{
    // attempt to link to a provider type slot
    constexpr DataSlotId providerDataId{ 131313u };
    consumerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureProvider, providerDataId, NodeHandle(), DataInstanceHandle::Invalid(), providerTextureHash, TextureSamplerHandle() });
    expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, consumerSceneId, providerDataId, SceneId(0u), DataSlotId(0u));

    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, providerDataId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinkFailed, providerStreamBuffer, consumerSceneId, providerDataId);
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsLinkFailedWhenLinkingToConsumerSlotWithWrongType_EB)
{
    // attempt to link to a provider type slot
    constexpr DataSlotId providerDataId{ 131313u };
    consumerSceneAllocator.allocateDataSlot({ EDataSlotType_TextureProvider, providerDataId, NodeHandle(), DataInstanceHandle::Invalid(), providerTextureHash, TextureSamplerHandle() });
    expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, consumerSceneId, providerDataId, SceneId(0u), DataSlotId(0u));

    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, providerDataId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinkFailed, providerExternalBuffer, consumerSceneId, providerDataId);
    expectNoBufferLink(samplerExternal);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndUnlinked_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId, SceneId::Invalid());
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndUnlinked_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId, SceneId::Invalid());
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndUnlinked_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerExternalBuffer, consumerSceneId, consumerExternalId);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerExternalId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerExternalId, SceneId::Invalid());
    expectNoBufferLink(samplerExternal);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSceneRemoved_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);

    rendererScenes.destroyScene(consumerSceneId);
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSceneRemoved_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);

    rendererScenes.destroyScene(consumerSceneId);
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSceneRemoved_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerExternalBuffer, consumerSceneId, consumerExternalId);

    rendererScenes.destroyScene(consumerSceneId);
    EXPECT_FALSE(textureLinkManager.hasLinkedExternalBuffer(consumerSceneId, samplerExternal));
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSlotRemoved_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);
    expectOffscreenBufferLink(sampler);

    consumerScene.releaseDataSlot(consumerSlotHandle);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSlotRemoved_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);
    expectStreamBufferLink(sampler);

    consumerScene.releaseDataSlot(consumerSlotHandle);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSlotRemoved_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectExternalBufferLink(samplerExternal);

    consumerScene.releaseDataSlot(consumerExternalSlotHandle);
    expectNoBufferLink(samplerExternal);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSceneUnmapped_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);

    sceneLinksManager.handleSceneUnmapped(consumerSceneId);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSceneUnmapped_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);

    sceneLinksManager.handleSceneUnmapped(consumerSceneId);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndConsumerSceneUnmapped_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerExternalBuffer, consumerSceneId, consumerExternalId);

    sceneLinksManager.handleSceneUnmapped(consumerSceneId);
    expectNoBufferLink(samplerExternal);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndBufferRemoved_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);

    sceneLinksManager.handleBufferDestroyed(providerOffscreenBuffer);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndBufferRemoved_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);

    sceneLinksManager.handleBufferDestroyedOrSourceUnavailable(providerStreamBuffer);
    expectNoTextureLink();
    expectNoBufferLink(sampler);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenBufferLinkedAndBufferRemoved_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerExternalBuffer, consumerSceneId, consumerExternalId);

    sceneLinksManager.handleBufferDestroyed(providerExternalBuffer);
    expectNoBufferLink(samplerExternal);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinked_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    consumerScene.updateRenderablesResourcesDirtiness();
    EXPECT_TRUE(consumerScene.renderableResourcesDirty(renderable));
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinked_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    consumerScene.updateRenderablesResourcesDirtiness();
    EXPECT_TRUE(consumerScene.renderableResourcesDirty(renderable));
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinked_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    consumerScene.updateRenderablesResourcesDirtiness();
    EXPECT_TRUE(consumerScene.renderableResourcesDirty(renderableWithExternalTexture));
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferUnlinked_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferUnlinked_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferUnlinked_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerExternalId);
    expectRenderableDirtinessState(renderable, false);
}

TEST_F(ATextureLinkManager, doesNotMarkRenderableUsingSamplerDirtyWhenUnlinkedWithoutBeingBufferLinked)
{
    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinkFailed, consumerSceneId, consumerId, SceneId::Invalid());
    expectRenderableDirtinessState(renderable, false);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinkedAndConsumerSlotRemoved_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    consumerScene.releaseDataSlot(consumerSlotHandle);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinkedAndConsumerSlotRemoved_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    consumerScene.releaseDataSlot(consumerSlotHandle);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinkedAndConsumerSlotRemoved_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    updateConsumerSceneResourcesCache();
    //expectRenderableDirtinessState(renderableWithExternalTexture, false);

    consumerScene.releaseDataSlot(consumerExternalSlotHandle);
    expectRenderableDirtinessState(renderableWithExternalTexture, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinkedAndBufferRemoved_OB)
{
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.handleBufferDestroyed(providerOffscreenBuffer);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinkedAndBufferRemoved_SB)
{
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    sceneLinksManager.handleBufferDestroyedOrSourceUnavailable(providerStreamBuffer);
    expectRenderableDirtinessState(renderable, true);
}

TEST_F(ATextureLinkManager, marksRenderableUsingSamplerDirtyWhenBufferLinkedAndBufferRemoved_EB)
{
    sceneLinksManager.createBufferLink(providerExternalBuffer, consumerSceneId, consumerExternalId);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderableWithExternalTexture, false);

    sceneLinksManager.handleBufferDestroyed(providerExternalBuffer);
    expectRenderableDirtinessState(renderableWithExternalTexture, true);
}

TEST_F(ATextureLinkManager, reportsLinkAndMarksRenderableUsingSamplerDirtyWhenProviderChangesTextureAssignedToSlot)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectTextureLink(providerTextureHash);
    updateConsumerSceneResourcesCache();
    expectRenderableDirtinessState(renderable, false);

    const ResourceContentHash newTexture(123u, 456u);
    providerScene.setDataSlotTexture(providerSlotHandle, newTexture);
    expectTextureLink(newTexture);
    expectRenderableDirtinessState(renderable, true);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectNoTextureLink();
}

// combined buffer and texture links tests
TEST_F(ATextureLinkManager, canCreateTextureLinkAndBufferLink)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    expectTextureLink(providerTextureHash);

    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectOffscreenBufferLink(sampler2);

    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);
    expectStreamBufferLink(sampler3);
}

TEST_F(ATextureLinkManager, differentTypesOfLinksOverwriteEachOther)
{
    // OB
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);
    expectOffscreenBufferLink(sampler);
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));

    // OB -> tex
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    expectTextureLink(providerTextureHash);
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));

    // tex -> OB
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);
    expectOffscreenBufferLink(sampler);
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));

    // OB -> SB
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);
    expectStreamBufferLink(sampler);
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler));

    // SB -> tex
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    expectTextureLink(providerTextureHash);
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));

    // tex -> SB
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId);
    expectStreamBufferLink(sampler);
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler));

    // SB -> OB
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId);
    expectOffscreenBufferLink(sampler);
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler));
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndProviderSceneRemoved_KeepsBufferLink)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);

    rendererScenes.destroyScene(providerSceneId);
    expectNoTextureLink();
    expectOffscreenBufferLink(sampler2, false);
    expectStreamBufferLink(sampler3, false);
    updateCacheAndExpectDeviceHandles(sampler2, sampler3, {});
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndBufferRemoved_KeepsTextureLink_OB)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);

    sceneLinksManager.handleBufferDestroyed(providerOffscreenBuffer);
    expectTextureLink(providerTextureHash);
    expectNoBufferLink(sampler2);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndBufferRemoved_KeepsTextureLink_SB)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId2);

    sceneLinksManager.handleBufferDestroyedOrSourceUnavailable(providerStreamBuffer);
    expectTextureLink(providerTextureHash);
    expectNoBufferLink(sampler2);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSceneRemoved_BufferAndTextureLinks)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);

    rendererScenes.destroyScene(consumerSceneId);
    EXPECT_FALSE(textureLinkManager.hasLinkedTexture(consumerSceneId, sampler));
    EXPECT_FALSE(textureLinkManager.hasLinkedOffscreenBuffer(consumerSceneId, sampler2));
    EXPECT_FALSE(textureLinkManager.hasLinkedStreamBuffer(consumerSceneId, sampler3));
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndProviderSlotRemoved_KeepsBufferLink)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);

    providerScene.releaseDataSlot(providerSlotHandle);
    expectNoTextureLink();
    expectOffscreenBufferLink(sampler2, false);
    expectStreamBufferLink(sampler3, false);
    updateCacheAndExpectDeviceHandles(sampler2, sampler3, {});
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSlotRemoved_KeepsBufferLink)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);

    consumerScene.releaseDataSlot(consumerSlotHandle);
    expectNoTextureLink();
    expectOffscreenBufferLink(sampler2, false);
    expectStreamBufferLink(sampler3, false);
    updateCacheAndExpectDeviceHandles(sampler2, sampler3, {});
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSlotRemoved_KeepsTextureLink_OB)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);

    consumerScene.releaseDataSlot(consumerSlotHandle2);
    expectTextureLink(providerTextureHash);
    expectNoBufferLink(sampler2);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSlotRemoved_KeepsTextureLink_SB)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId2);

    consumerScene.releaseDataSlot(consumerSlotHandle2);
    expectTextureLink(providerTextureHash);
    expectNoBufferLink(sampler2);
}

TEST_F(ATextureLinkManager, reportsNoLinkForSamplerWhenLinkedAndConsumerSceneUnmapped_BufferAndTextureLinks)
{
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);

    sceneLinksManager.handleSceneUnmapped(consumerSceneId);
    expectNoTextureLink();
    expectNoBufferLink(sampler2);
    expectNoBufferLink(sampler3);
}

TEST_F(ATextureLinkManager, canCreateTextureLinkAndBufferLinkIfPreviouslyUsingRenderTarget)
{
    const RenderBufferHandle renderBuffer(14u);
    setFallbackTextureToConsumerSampler(sampler, {}, renderBuffer);
    setFallbackTextureToConsumerSampler(sampler2, {}, renderBuffer);
    setFallbackTextureToConsumerSampler(sampler3, {}, renderBuffer);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);

    expectTextureLink(providerTextureHash);
    expectOffscreenBufferLink(sampler2, false);
    expectStreamBufferLink(sampler3, false);
    updateCacheAndExpectDeviceHandles(sampler2, sampler3, {});
}

TEST_F(ATextureLinkManager, unlinkingTextureAndBufferFallsBackToPreviouslySetRenderTarget)
{
    const RenderBufferHandle renderBuffer(14u);
    setFallbackTextureToConsumerSampler(sampler, {}, renderBuffer);
    setFallbackTextureToConsumerSampler(sampler2, {}, renderBuffer);
    setFallbackTextureToConsumerSampler(sampler3, {}, renderBuffer);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createBufferLink(providerOffscreenBuffer, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerOffscreenBuffer, consumerSceneId, consumerId2);
    sceneLinksManager.createBufferLink(providerStreamBuffer, consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataBufferLinked, providerStreamBuffer, consumerSceneId, consumerId3);

    expectTextureLink(providerTextureHash);
    expectOffscreenBufferLink(sampler2, false);
    expectStreamBufferLink(sampler3, false);
    updateCacheAndExpectDeviceHandles(sampler2, sampler3, {});

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId, providerSceneId);
    sceneLinksManager.removeDataLink(consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId2, SceneId::Invalid());
    sceneLinksManager.removeDataLink(consumerSceneId, consumerId3);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId3, SceneId::Invalid());

    EXPECT_EQ(TextureSampler::ContentType::RenderBuffer, consumerScene.getTextureSampler(sampler).contentType);
    EXPECT_EQ(TextureSampler::ContentType::RenderBuffer, consumerScene.getTextureSampler(sampler2).contentType);
    EXPECT_EQ(TextureSampler::ContentType::RenderBuffer, consumerScene.getTextureSampler(sampler3).contentType);
    EXPECT_EQ(renderBuffer.asMemoryHandle(), consumerScene.getTextureSampler(sampler).contentHandle);
    EXPECT_EQ(renderBuffer.asMemoryHandle(), consumerScene.getTextureSampler(sampler2).contentHandle);
    EXPECT_EQ(renderBuffer.asMemoryHandle(), consumerScene.getTextureSampler(sampler3).contentHandle);
}
}
