//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/RendererScenes.h"
#include "RendererEventCollector.h"
#include "SceneLinksTestUtils.h"

using namespace testing;
using namespace ramses_internal;

template <typename T>
class ASceneLinksManager : public ::testing::Test
{
public:
    ASceneLinksManager()
        : rendererScenes(rendererEventCollector)
        , sceneLinksManager(rendererScenes.getSceneLinksManager())
        , concreteLinkManager(GetConcreteLinkManager<T>(sceneLinksManager))
        , providerSceneId(3u)
        , consumerSceneId(4u)
        , providerScene(rendererScenes.createScene(SceneInfo(providerSceneId)))
        , consumerScene(rendererScenes.createScene(SceneInfo(consumerSceneId)))
        , providerSceneAllocator(providerScene)
        , consumerSceneAllocator(consumerScene)
        , providerId(33u)
        , consumerId(44u)
        , providerSlotHandle(5u)
        , consumerSlotHandle(6u)
    {
        createDataSlot<T>(providerSceneAllocator, providerSlotHandle, providerId, true);
        expectRendererEvent(ERendererEventType_SceneDataSlotProviderCreated, providerSceneId, providerId, SceneId(0u), DataSlotId(0u));

        createDataSlot<T>(consumerSceneAllocator, consumerSlotHandle, consumerId, false);
        expectRendererEvent(ERendererEventType_SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId);
    }

protected:
    void expectRendererEvent(ERendererEventType event, SceneId providerSId, DataSlotId pId, SceneId consumerSId, DataSlotId cId)
    {
        RendererEventVector events;
        rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(event, events.front().eventType);
        EXPECT_EQ(providerSId, events.front().providerSceneId);
        EXPECT_EQ(consumerSId, events.front().consumerSceneId);
        EXPECT_EQ(pId, events.front().providerdataId);
        EXPECT_EQ(cId, events.front().consumerdataId);
    }

    void expectRendererEvent(ERendererEventType event, SceneId consumerSId, DataSlotId cId)
    {
        RendererEventVector events;
        rendererEventCollector.dispatchEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(event, events.front().eventType);
        EXPECT_EQ(consumerSId, events.front().consumerSceneId);
        EXPECT_EQ(cId, events.front().consumerdataId);
    }

    void expectConsumerLinkedToProvider()
    {
        EXPECT_TRUE(this->concreteLinkManager.getSceneLinks().hasLinkedConsumers(this->providerSceneId, this->providerSlotHandle));
        SceneLinkVector links;
        this->concreteLinkManager.getSceneLinks().getLinkedConsumers(this->providerSceneId, this->providerSlotHandle, links);
        ASSERT_EQ(1u, links.size());
        EXPECT_EQ(this->providerSceneId, links.front().providerSceneId);
        EXPECT_EQ(this->providerSlotHandle, links.front().providerSlot);
        EXPECT_EQ(this->consumerSceneId, links.front().consumerSceneId);
        EXPECT_EQ(this->consumerSlotHandle, links.front().consumerSlot);
    }

    void expectRendererEventUnlinkedAndDestroyedSlot(bool slotIsProvider, SceneId sceneIdOfDestroyedSlot, DataSlotId destroyedSlotId, SceneId consumerSId, DataSlotId cId, SceneId consumerSceneId2 = SceneId(0u), DataSlotId consumerId2 = DataSlotId(0u))
    {
        RendererEventVector events;
        rendererEventCollector.dispatchEvents(events);

        const bool hasTwoLinksRemoved = (consumerId2.getValue() != 0u);
        if (hasTwoLinksRemoved)
        {
            ASSERT_EQ(3u, events.size());
        }
        else
        {
            ASSERT_EQ(2u, events.size());
        }

        UInt32 eventIdx = 0u;

        EXPECT_EQ(ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange, events[eventIdx].eventType);
        EXPECT_EQ(consumerSId, events[eventIdx].consumerSceneId);
        EXPECT_EQ(cId, events[eventIdx].consumerdataId);

        if (hasTwoLinksRemoved)
        {
            ++eventIdx;
            EXPECT_EQ(ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange, events[eventIdx].eventType);
            EXPECT_EQ(consumerSceneId2, events[eventIdx].consumerSceneId);
            EXPECT_EQ(consumerId2, events[eventIdx].consumerdataId);
        }

        ++eventIdx;
        if (slotIsProvider)
        {
            EXPECT_EQ(ERendererEventType_SceneDataSlotProviderDestroyed, events[eventIdx].eventType);
            EXPECT_EQ(sceneIdOfDestroyedSlot, events[eventIdx].providerSceneId);
            EXPECT_EQ(destroyedSlotId, events[eventIdx].providerdataId);
        }
        else
        {
            EXPECT_EQ(ERendererEventType_SceneDataSlotConsumerDestroyed, events[eventIdx].eventType);
            EXPECT_EQ(sceneIdOfDestroyedSlot, events[eventIdx].consumerSceneId);
            EXPECT_EQ(destroyedSlotId, events[eventIdx].consumerdataId);
        }
    }

    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    SceneLinksManager& sceneLinksManager;
    const T& concreteLinkManager;
    const SceneId providerSceneId;
    const SceneId consumerSceneId;
    IScene& providerScene;
    IScene& consumerScene;
    SceneAllocateHelper providerSceneAllocator;
    SceneAllocateHelper consumerSceneAllocator;
    const DataSlotId providerId;
    const DataSlotId consumerId;
    const DataSlotHandle providerSlotHandle;
    const DataSlotHandle consumerSlotHandle;
};

typedef ::testing::Types <
    TransformationLinkManager,
    DataReferenceLinkManager,
    TextureLinkManager
> ManagerTypes;

TYPED_TEST_CASE(ASceneLinksManager, ManagerTypes);

TYPED_TEST(ASceneLinksManager, reportsLinkCreationEvent)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectConsumerLinkedToProvider();
}

TYPED_TEST(ASceneLinksManager, reportsLinkRemovalEvent)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.removeDataLink(this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataUnlinked, this->consumerSceneId, this->consumerId);
}

TYPED_TEST(ASceneLinksManager, reportsProviderSlotRemoved)
{
    this->providerScene.releaseDataSlot(this->providerSlotHandle);
    this->expectRendererEvent(ERendererEventType_SceneDataSlotProviderDestroyed, this->providerSceneId, this->providerId, SceneId(0u), DataSlotId(0u));
}

TYPED_TEST(ASceneLinksManager, reportsConsumerSlotRemoved)
{
    this->consumerScene.releaseDataSlot(this->consumerSlotHandle);
    this->expectRendererEvent(ERendererEventType_SceneDataSlotConsumerDestroyed, SceneId(0u), DataSlotId(0u), this->consumerSceneId, this->consumerId);
}

TYPED_TEST(ASceneLinksManager, removesLinkForSceneWithRemovedProviderSlot)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->providerScene.releaseDataSlot(this->providerSlotHandle);
    this->expectRendererEventUnlinkedAndDestroyedSlot(true, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);

    EXPECT_FALSE(this->concreteLinkManager.getSceneLinks().hasLinkedConsumers(this->providerSceneId, this->providerSlotHandle));
    EXPECT_FALSE(this->concreteLinkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
}

TYPED_TEST(ASceneLinksManager, removesLinkForSceneWithRemovedConsumerSlot)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->consumerScene.releaseDataSlot(this->consumerSlotHandle);
    this->expectRendererEventUnlinkedAndDestroyedSlot(false, this->consumerSceneId, this->consumerId, this->consumerSceneId, this->consumerId);

    EXPECT_FALSE(this->concreteLinkManager.getSceneLinks().hasLinkedConsumers(this->providerSceneId, this->providerSlotHandle));
    EXPECT_FALSE(this->concreteLinkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
}

TYPED_TEST(ASceneLinksManager, failsToCreateSecondLinkToConsumer)
{
    const DataSlotId providerId2(999u);
    const DataSlotHandle slotHandle(43u);
    createDataSlot<TypeParam>(this->providerSceneAllocator, slotHandle, providerId2, true);
    this->expectRendererEvent(ERendererEventType_SceneDataSlotProviderCreated, this->providerSceneId, providerId2, SceneId(0u), DataSlotId(0u));

    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.createDataLink(this->providerSceneId, providerId2, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->providerSceneId, providerId2, this->consumerSceneId, this->consumerId);
}

TYPED_TEST(ASceneLinksManager, failsToCreateLinkForInvalidSceneId)
{
    this->sceneLinksManager.createDataLink(SceneId(0u), this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, SceneId(0u), this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, SceneId(0u), this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->providerSceneId, this->providerId, SceneId(0u), this->consumerId);
}

TYPED_TEST(ASceneLinksManager, failsToCreateLinkForInvalidDataSlotId)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, DataSlotId(), this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->providerSceneId, DataSlotId(), this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, DataSlotId());
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->providerSceneId, this->providerId, this->consumerSceneId, DataSlotId());
}

TYPED_TEST(ASceneLinksManager, failsToCreateLinkForSwappedDataSlotId)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->consumerId, this->consumerSceneId, this->providerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->providerSceneId, this->consumerId, this->consumerSceneId, this->providerId);
    this->sceneLinksManager.createDataLink(this->consumerSceneId, this->providerId, this->providerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->consumerSceneId, this->providerId, this->providerSceneId, this->consumerId);
}

TYPED_TEST(ASceneLinksManager, failsToCreateLinkForSwappedConsumerAndProvider)
{
    this->sceneLinksManager.createDataLink(this->consumerSceneId, this->consumerId, this->providerSceneId, this->providerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->consumerSceneId, this->consumerId, this->providerSceneId, this->providerId);
}

TYPED_TEST(ASceneLinksManager, failsToCreateLinkTwiceForSameConsumer)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
}

TYPED_TEST(ASceneLinksManager, failsToRemoveLinkForInvalidSceneId)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.removeDataLink(SceneId(0u), this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataUnlinkFailed, SceneId(0u), this->consumerId);
}

TYPED_TEST(ASceneLinksManager, failsToRemoveLinkForInvalidDataSlotId)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.removeDataLink(this->consumerSceneId, DataSlotId());
    this->expectRendererEvent(ERendererEventType_SceneDataUnlinkFailed, this->consumerSceneId, DataSlotId());
}

TYPED_TEST(ASceneLinksManager, failsToRemoveLinkWhenProvidingProviderInsteadOfConsumer)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.removeDataLink(this->providerSceneId, this->providerId);
    this->expectRendererEvent(ERendererEventType_SceneDataUnlinkFailed, this->providerSceneId, this->providerId);
}

TYPED_TEST(ASceneLinksManager, failsToRemoveLinkIfThereIsNoLink)
{
    this->sceneLinksManager.removeDataLink(this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataUnlinkFailed, this->consumerSceneId, this->consumerId);
}

TYPED_TEST(ASceneLinksManager, failsToCreateLinksCausingCyclicDependency)
{
    const DataSlotId providerId2(999u);
    const DataSlotHandle slotHandle(43u);
    createDataSlot<TypeParam>(this->consumerSceneAllocator, slotHandle, providerId2, true);
    this->expectRendererEvent(ERendererEventType_SceneDataSlotProviderCreated, this->consumerSceneId, providerId2, SceneId(0u), DataSlotId(0u));

    const DataSlotId consumerId2(999u);
    const DataSlotHandle slotHandle2(43u);
    createDataSlot<TypeParam>(this->providerSceneAllocator, slotHandle2, consumerId2, true);
    this->expectRendererEvent(ERendererEventType_SceneDataSlotProviderCreated, this->providerSceneId, providerId2, SceneId(0u), DataSlotId(0u));

    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.createDataLink(this->consumerSceneId, providerId2, this->providerSceneId, consumerId2);
    this->expectRendererEvent(ERendererEventType_SceneDataLinkFailed, this->consumerSceneId, providerId2, this->providerSceneId, consumerId2);
}

TYPED_TEST(ASceneLinksManager, canLinkUnlinkAndLinkAgain)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.removeDataLink(this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataUnlinked, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectConsumerLinkedToProvider();
}

TYPED_TEST(ASceneLinksManager, removesAllLinksToProviderOnProviderSlotDestruction)
{
    const DataSlotId consumerId2(999u);
    const DataSlotHandle slotHandle(43u);
    createDataSlot<TypeParam>(this->consumerSceneAllocator, slotHandle, consumerId2, false);
    this->expectRendererEvent(ERendererEventType_SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), this->consumerSceneId, consumerId2);

    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, consumerId2);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, consumerId2);

    this->providerScene.releaseDataSlot(this->providerSlotHandle);
    this->expectRendererEventUnlinkedAndDestroyedSlot(true, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId, this->consumerSceneId, consumerId2);

    EXPECT_FALSE(this->concreteLinkManager.getSceneLinks().hasLinkedConsumers(this->providerSceneId, this->providerSlotHandle));
    EXPECT_FALSE(this->concreteLinkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
}

typedef ASceneLinksManager<TextureLinkManager> SceneLinksTextureManager;

TEST_F(SceneLinksTextureManager, unlinksTextureLinksForUnmappedProviderScene)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);

    this->sceneLinksManager.handleSceneUnmapped(this->providerSceneId);
    EXPECT_FALSE(this->sceneLinksManager.getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(this->consumerSceneId));
    EXPECT_FALSE(this->sceneLinksManager.getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(this->consumerSceneId));
}

TEST_F(SceneLinksTextureManager, unlinksTextureLinksForUnmappedConsumerScene)
{
    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType_SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);

    this->sceneLinksManager.handleSceneUnmapped(this->consumerSceneId);
    EXPECT_FALSE(this->sceneLinksManager.getTextureLinkManager().getSceneLinks().hasAnyLinksToProvider(this->providerSceneId));
    EXPECT_FALSE(this->sceneLinksManager.getTextureLinkManager().getSceneLinks().hasAnyLinksToConsumer(this->providerSceneId));
}
