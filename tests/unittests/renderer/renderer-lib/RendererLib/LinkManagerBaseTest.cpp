//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/TransformationLinkManager.h"
#include "internal/RendererLib/DataReferenceLinkManager.h"
#include "internal/RendererLib/TransformationLinkCachedScene.h"
#include "internal/RendererLib/TextureLinkManager.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "SceneLinksTestUtils.h"

using namespace testing;

namespace ramses::internal
{
    template <typename T>
    class ALinkManager : public ::testing::Test
    {
    public:
        ALinkManager()
            : rendererScenes(rendererEventCollector)
            , linkManager(rendererScenes)
            , providerSceneId(3u)
            , consumerSceneId(4u)
            , providerScene(rendererScenes.createScene(SceneInfo(providerSceneId)))
            , consumerScene(rendererScenes.createScene(SceneInfo(consumerSceneId)))
            , providerSceneAllocator(providerScene)
            , consumerSceneAllocator(consumerScene)
            , providerSlotHandle(55u)
            , consumerSlotHandle(66u)
        {
            createDataSlot<T>(providerSceneAllocator, providerSlotHandle, DataSlotId(33u), true);
            createDataSlot<T>(consumerSceneAllocator, consumerSlotHandle, DataSlotId(44u), false);
        }

    protected:
        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes;
        T linkManager;

        const SceneId providerSceneId;
        const SceneId consumerSceneId;
        IScene& providerScene;
        IScene& consumerScene;
        SceneAllocateHelper providerSceneAllocator;
        SceneAllocateHelper consumerSceneAllocator;
        const DataSlotHandle providerSlotHandle;
        const DataSlotHandle consumerSlotHandle;
    };

    using ManagerTypes = ::testing::Types <
        LinkManagerBase,
        TransformationLinkManager,
        DataReferenceLinkManager,
        TextureLinkManager
    >;

    TYPED_TEST_SUITE(ALinkManager, ManagerTypes);

    TYPED_TEST(ALinkManager, reportsNoDependenciesInitially)
    {
        EXPECT_TRUE(this->linkManager.getDependencyChecker().isEmpty());
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToConsumer(this->providerSceneId));
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToProvider(this->consumerSceneId));
    }

    TYPED_TEST(ALinkManager, canCreateLinkFromConsumerToProvider)
    {
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_TRUE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
        const SceneLink& link = this->linkManager.getSceneLinks().getLinkedProvider(this->consumerSceneId, this->consumerSlotHandle);
        EXPECT_EQ(this->providerSceneId, link.providerSceneId);
        EXPECT_EQ(this->consumerSceneId, link.consumerSceneId);
        EXPECT_EQ(this->providerSlotHandle, link.providerSlot);
        EXPECT_EQ(this->consumerSlotHandle, link.consumerSlot);

        EXPECT_TRUE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
        EXPECT_FALSE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->providerSceneId));
    }

    TYPED_TEST(ALinkManager, reportsNoLinksForConsumerSceneIfSceneLinksRemoved)
    {
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        this->linkManager.removeSceneLinks(this->consumerSceneId);
        EXPECT_TRUE(this->linkManager.getDependencyChecker().isEmpty());
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToConsumer(this->providerSceneId));
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToProvider(this->consumerSceneId));
    }

    TYPED_TEST(ALinkManager, reportsNoLinksForConsumerSceneIfProviderSceneDestroyed)
    {
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        this->linkManager.removeSceneLinks(this->providerSceneId);
        EXPECT_TRUE(this->linkManager.getDependencyChecker().isEmpty());
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToConsumer(this->providerSceneId));
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToProvider(this->consumerSceneId));
    }

    TYPED_TEST(ALinkManager, reportsNoLinksForSceneWithRemovedLink)
    {
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_TRUE(this->linkManager.removeDataLink(this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_FALSE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToConsumer(this->providerSceneId));
        EXPECT_FALSE(this->linkManager.getSceneLinks().hasAnyLinksToProvider(this->consumerSceneId));
    }

    TYPED_TEST(ALinkManager, canLinkUnlinkAndLinkAgain)
    {
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_TRUE(this->linkManager.removeDataLink(this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));

        const SceneLink& link = this->linkManager.getSceneLinks().getLinkedProvider(this->consumerSceneId, this->consumerSlotHandle);
        EXPECT_EQ(this->providerSceneId, link.providerSceneId);
        EXPECT_EQ(this->consumerSceneId, link.consumerSceneId);
        EXPECT_EQ(this->providerSlotHandle, link.providerSlot);
        EXPECT_EQ(this->consumerSlotHandle, link.consumerSlot);

        EXPECT_TRUE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
        EXPECT_FALSE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->providerSceneId));
    }

    TYPED_TEST(ALinkManager, createsTwoLinksSameProviderDifferentConsumers)
    {
        const DataSlotHandle consumerSlot2(43u);
        createDataSlot<TypeParam>(this->consumerSceneAllocator, consumerSlot2, DataSlotId(88u), false);

        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, consumerSlot2));

        const SceneLink& link = this->linkManager.getSceneLinks().getLinkedProvider(this->consumerSceneId, consumerSlot2);
        EXPECT_EQ(this->providerSceneId, link.providerSceneId);
        EXPECT_EQ(this->consumerSceneId, link.consumerSceneId);
        EXPECT_EQ(this->providerSlotHandle, link.providerSlot);
        EXPECT_EQ(consumerSlot2, link.consumerSlot);

        EXPECT_TRUE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->consumerSceneId));
        EXPECT_FALSE(this->linkManager.getDependencyChecker().hasDependencyAsConsumer(this->providerSceneId));
    }

    TYPED_TEST(ALinkManager, failsToCreateLinksCausingCyclicDependency)
    {
        const DataSlotHandle slotHandle(43u);
        createDataSlot<TypeParam>(this->consumerSceneAllocator, slotHandle, DataSlotId(88u), true);

        const DataSlotHandle slotHandle2(43u);
        createDataSlot<TypeParam>(this->providerSceneAllocator, slotHandle2, DataSlotId(89u), false);

        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_FALSE(this->linkManager.createDataLink(this->consumerSceneId, slotHandle, this->providerSceneId, slotHandle2));
    }

    TYPED_TEST(ALinkManager, failsToCreateLinkIfConsumerAlreadyLinked)
    {
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_FALSE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
    }

    TYPED_TEST(ALinkManager, failsToRemoveLinkIfConsumerNotLinked)
    {
        EXPECT_FALSE(this->linkManager.removeDataLink(this->consumerSceneId, this->consumerSlotHandle));
    }

    TYPED_TEST(ALinkManager, failsToRemoveLinkIfConsumerAlreadyUnlinked)
    {
        EXPECT_TRUE(this->linkManager.createDataLink(this->providerSceneId, this->providerSlotHandle, this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_TRUE(this->linkManager.removeDataLink(this->consumerSceneId, this->consumerSlotHandle));
        EXPECT_FALSE(this->linkManager.removeDataLink(this->consumerSceneId, this->consumerSlotHandle));
    }
}
