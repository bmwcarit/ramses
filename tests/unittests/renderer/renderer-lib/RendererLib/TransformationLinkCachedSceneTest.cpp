//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/TransformationLinkCachedScene.h"
#include "internal/RendererLib/SceneLinksManager.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "TestEqualHelper.h"
#include "SceneAllocateHelper.h"
#include "internal/Core/Utils/ThreadLocalLog.h"
#include "internal/Core/Math3d/Rotation.h"
#include "glm/gtx/transform.hpp"

using namespace testing;

namespace ramses::internal
{
    class ATransformationLinkCachedScene : public testing::Test
    {
    public:
        ATransformationLinkCachedScene()
            : rendererScenes(rendererEventCollector)
            , sceneLinksManager(rendererScenes.getSceneLinksManager())
            , providerScene(rendererScenes.createScene(SceneInfo(SceneId(0u))))
            , consumer1Scene(rendererScenes.createScene(SceneInfo(SceneId(1u))))
            , consumer2Scene(rendererScenes.createScene(SceneInfo(SceneId(2u))))
            , consumer2ndLevelScene(rendererScenes.createScene(SceneInfo(SceneId(3u))))
            , consumerWithoutTransformsScene(rendererScenes.createScene(SceneInfo(SceneId(4u))))
            , providerSceneAllocator(providerScene)
            , consumer1SceneAllocator(consumer1Scene)
            , consumer2SceneAllocator(consumer2Scene)
            , consumer2ndLevelSceneAllocator(consumer2ndLevelScene)
            , consumerWithoutTransformsSceneAllocator(consumerWithoutTransformsScene)
            , providerSceneRootNode(1u)
            , providerSceneNode(2u)
            , providerSceneRootTransform(3u)
            , providerSceneTransform(4u)
            , consumer1SceneRootNode(5u)
            , consumer1SceneNode(6u)
            , consumer1SceneNode2(7u)
            , consumer1SceneRootTransform(8u)
            , consumer1SceneTransform(9u)
            , consumer1SceneTransform2(10u)
            , consumer2SceneRoot(11u)
            , consumer2SceneRootTransform(12u)
            , consumer2SceneNode(15u)
            , consumer2SceneTransform(16u)
            , consumer2ndLevelSceneNode(13u)
            , consumer2ndLevelSceneTransform(14u)
            , consumerWithoutTransformsRoot(20u)
            , consumerWithoutTransformsNode1(21u)
            , consumerWithoutTransformsNode2(22u)
            , consumerWithoutTransformsTransform2(23u)
            , consumerWithoutTransformsNode3(24u)
            , providerSceneRootTranslation(1, 2, 3)
            , providerSceneNodeTranslation(4, 5, 6)
            , consumer1SceneRootTranslation(7, 8, 9)
            , consumer1SceneNodeTranslation(11, 12, 13)
            , consumer1SceneNode2Translation(21, 22, 23)
            , consumer2SceneRootTranslation(31, 32, 33)
            , consumer2SceneNodeTranslation(51, 52, 53)
            , consumer2ndLevelSceneNodeTranslation(41, 42, 43)
            , consumerWithoutTransformsTransform2Translation(61, 62, 63)
            , providerSceneProviderSlotHandle(17u)
            , consumer1SceneConsumerSlotHandle(18u)
            , consumerWithoutTransformsSceneConsumerSlotHandle(19u)
            , providerSceneProviderId(1u)
            , consumer1SceneConsumerId(2u)
            , consumer1SceneProviderId(3u)
            , consumer2SceneConsumerId(4u)
            , consumer2ndLevelSceneConsumerId(5u)
            , consumerWithoutTransformsSceneConsumerId(6u)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
            setupNodesAndDataSlots();
        }

    protected:
        void setupNodesAndDataSlots()
        {
            // provider scene
            providerSceneAllocator.allocateNode(0u, providerSceneRootNode);
            providerSceneAllocator.allocateNode(0u, providerSceneNode);
            providerSceneAllocator.allocateTransform(providerSceneRootNode, providerSceneRootTransform);
            providerSceneAllocator.allocateTransform(providerSceneNode, providerSceneTransform);

            providerScene.addChildToNode(providerSceneRootNode, providerSceneNode);

            // consumer 1 scene
            consumer1SceneAllocator.allocateNode(0u, consumer1SceneRootNode);
            consumer1SceneAllocator.allocateNode(0u, consumer1SceneNode);
            consumer1SceneAllocator.allocateNode(0u, consumer1SceneNode2);
            consumer1SceneAllocator.allocateTransform(consumer1SceneRootNode, consumer1SceneRootTransform);
            consumer1SceneAllocator.allocateTransform(consumer1SceneNode, consumer1SceneTransform);
            consumer1SceneAllocator.allocateTransform(consumer1SceneNode2, consumer1SceneTransform2);

            consumer1Scene.addChildToNode(consumer1SceneRootNode, consumer1SceneNode);
            consumer1Scene.addChildToNode(consumer1SceneNode, consumer1SceneNode2);

            // consumer 2 scene
            consumer2SceneAllocator.allocateNode(0u, consumer2SceneRoot);
            consumer2SceneAllocator.allocateNode(0u, consumer2SceneNode);
            consumer2SceneAllocator.allocateTransform(consumer2SceneRoot, consumer2SceneRootTransform);
            consumer2SceneAllocator.allocateTransform(consumer2SceneNode, consumer2SceneTransform);

            consumer2Scene.addChildToNode(consumer2SceneRoot, consumer2SceneNode);

            // second level consumer scene
            consumer2ndLevelSceneAllocator.allocateNode(0u, consumer2ndLevelSceneNode);
            consumer2ndLevelSceneAllocator.allocateTransform(consumer2ndLevelSceneNode, consumer2ndLevelSceneTransform);

            // consumer without transforms scene
            consumerWithoutTransformsSceneAllocator.allocateNode(0u, consumerWithoutTransformsRoot);
            consumerWithoutTransformsSceneAllocator.allocateNode(0u, consumerWithoutTransformsNode1);
            consumerWithoutTransformsSceneAllocator.allocateNode(0u, consumerWithoutTransformsNode2);
            consumerWithoutTransformsSceneAllocator.allocateNode(0u, consumerWithoutTransformsNode3);
            consumerWithoutTransformsScene.addChildToNode(consumerWithoutTransformsRoot, consumerWithoutTransformsNode1);
            consumerWithoutTransformsScene.addChildToNode(consumerWithoutTransformsRoot, consumerWithoutTransformsNode2);
            consumerWithoutTransformsScene.addChildToNode(consumerWithoutTransformsNode2, consumerWithoutTransformsNode3);
            consumerWithoutTransformsSceneAllocator.allocateTransform(consumerWithoutTransformsNode2, consumerWithoutTransformsTransform2);

            // set initial translations
            setAllTranslations();

            // create provider/consumer slots
            DataSlotHandle slotHandle;
            slotHandle = providerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationProvider, providerSceneProviderId, providerSceneNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, providerSceneProviderSlotHandle);
            slotHandle = consumer1SceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumer1SceneConsumerId, consumer1SceneNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumer1SceneConsumerSlotHandle);
            slotHandle = consumer1SceneAllocator.allocateDataSlot({ EDataSlotType::TransformationProvider, consumer1SceneProviderId, consumer1SceneNode2, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() });
            slotHandle = consumer2SceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumer2SceneConsumerId, consumer2SceneRoot, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() });
            slotHandle = consumer2ndLevelSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumer2ndLevelSceneConsumerId, consumer2ndLevelSceneNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() });
            slotHandle = consumerWithoutTransformsSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumerWithoutTransformsSceneConsumerId, consumerWithoutTransformsRoot, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumerWithoutTransformsSceneConsumerSlotHandle);
        }

        void setAllTranslations(bool reset = false)
        {
            if (reset)
            {
                providerScene.setTranslation(providerSceneRootTransform, { 0.f, 0.f, 0.f });
                providerScene.setTranslation(providerSceneTransform, { 0.f, 0.f, 0.f });
                consumer1Scene.setTranslation(consumer1SceneRootTransform, { 0.f, 0.f, 0.f });
                consumer1Scene.setTranslation(consumer1SceneTransform, { 0.f, 0.f, 0.f });
                consumer1Scene.setTranslation(consumer1SceneTransform2, { 0.f, 0.f, 0.f });
                consumer2Scene.setTranslation(consumer2SceneRootTransform, { 0.f, 0.f, 0.f });
                consumer2Scene.setTranslation(consumer2SceneTransform, { 0.f, 0.f, 0.f });
                consumer2ndLevelScene.setTranslation(consumer2ndLevelSceneTransform, { 0.f, 0.f, 0.f });
                consumerWithoutTransformsScene.setTranslation(consumerWithoutTransformsTransform2, { 0.f, 0.f, 0.f });

            }
            else
            {
                providerScene.setTranslation(providerSceneRootTransform, providerSceneRootTranslation);
                providerScene.setTranslation(providerSceneTransform, providerSceneNodeTranslation);
                consumer1Scene.setTranslation(consumer1SceneRootTransform, consumer1SceneRootTranslation);
                consumer1Scene.setTranslation(consumer1SceneTransform, consumer1SceneNodeTranslation);
                consumer1Scene.setTranslation(consumer1SceneTransform2, consumer1SceneNode2Translation);
                consumer2Scene.setTranslation(consumer2SceneRootTransform, consumer2SceneRootTranslation);
                consumer2Scene.setTranslation(consumer2SceneTransform, consumer2SceneNodeTranslation);
                consumer2ndLevelScene.setTranslation(consumer2ndLevelSceneTransform, consumer2ndLevelSceneNodeTranslation);
                consumerWithoutTransformsScene.setTranslation(consumerWithoutTransformsTransform2, consumerWithoutTransformsTransform2Translation);
            }
        }

        void linkConsumer1SceneToProviderScene()
        {
            sceneLinksManager.createDataLink(providerScene.getSceneId(), providerSceneProviderId, consumer1Scene.getSceneId(), consumer1SceneConsumerId);
        }

        void linkConsumer2SceneToProviderScene()
        {
            sceneLinksManager.createDataLink(providerScene.getSceneId(), providerSceneProviderId, consumer2Scene.getSceneId(), consumer2SceneConsumerId);
        }

        void linkConsumer2ndLevelSceneToConsumer1Scene()
        {
            sceneLinksManager.createDataLink(consumer1Scene.getSceneId(), consumer1SceneProviderId, consumer2ndLevelScene.getSceneId(), consumer2ndLevelSceneConsumerId);
        }

        void linkConsumerWithoutTransformsSceneToProviderScene()
        {
            sceneLinksManager.createDataLink(providerScene.getSceneId(), providerSceneProviderId, consumerWithoutTransformsScene.getSceneId(), consumerWithoutTransformsSceneConsumerId);
        }

        static void ExpectIdentityMatrix(const NodeHandle nodeToCheck, const TransformationLinkCachedScene& scene)
        {
            ExpectCorrectMatrix(nodeToCheck, glm::identity<glm::mat4>(), scene);
        }

        static void ExpectCorrectMatrix(const NodeHandle nodeToCheck, const glm::mat4& worldMatrix, const TransformationLinkCachedScene& scene)
        {
            expectMatrixFloatEqual(worldMatrix, scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, nodeToCheck));
            expectMatrixFloatEqual(glm::inverse(worldMatrix), scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, nodeToCheck));
        }

        RendererEventVector consumeSceneControlEvents()
        {
            RendererEventVector events;
            RendererEventVector dummy;
            rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
            return events;
        }

        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes;
        SceneLinksManager& sceneLinksManager;

        TransformationLinkCachedScene& providerScene;
        TransformationLinkCachedScene& consumer1Scene;
        TransformationLinkCachedScene& consumer2Scene;
        TransformationLinkCachedScene& consumer2ndLevelScene;
        TransformationLinkCachedScene& consumerWithoutTransformsScene;

        SceneAllocateHelper providerSceneAllocator;
        SceneAllocateHelper consumer1SceneAllocator;
        SceneAllocateHelper consumer2SceneAllocator;
        SceneAllocateHelper consumer2ndLevelSceneAllocator;
        SceneAllocateHelper consumerWithoutTransformsSceneAllocator;

        // provider scene with root and child, child is provider
        const NodeHandle      providerSceneRootNode;
        const NodeHandle      providerSceneNode;
        const TransformHandle providerSceneRootTransform;
        const TransformHandle providerSceneTransform;

        // consumer scene with root and child, child is consumer
        // second level child is provider for second level consumer
        const NodeHandle      consumer1SceneRootNode;
        const NodeHandle      consumer1SceneNode;
        const NodeHandle      consumer1SceneNode2;
        const TransformHandle consumer1SceneRootTransform;
        const TransformHandle consumer1SceneTransform;
        const TransformHandle consumer1SceneTransform2;

        // second consumer scene with single consumer
        const NodeHandle      consumer2SceneRoot;
        const TransformHandle consumer2SceneRootTransform;
        const NodeHandle      consumer2SceneNode;
        const TransformHandle consumer2SceneTransform;

        // second level consumer scene with single consumer
        const NodeHandle      consumer2ndLevelSceneNode;
        const TransformHandle consumer2ndLevelSceneTransform;

        // consumer scene with no transforms
        const NodeHandle      consumerWithoutTransformsRoot;
        const NodeHandle      consumerWithoutTransformsNode1;
        const NodeHandle      consumerWithoutTransformsNode2;
        const TransformHandle consumerWithoutTransformsTransform2;
        const NodeHandle      consumerWithoutTransformsNode3;

        const glm::vec3 providerSceneRootTranslation;
        const glm::vec3 providerSceneNodeTranslation;
        const glm::vec3 consumer1SceneRootTranslation;
        const glm::vec3 consumer1SceneNodeTranslation;
        const glm::vec3 consumer1SceneNode2Translation;
        const glm::vec3 consumer2SceneRootTranslation;
        const glm::vec3 consumer2SceneNodeTranslation;
        const glm::vec3 consumer2ndLevelSceneNodeTranslation;
        const glm::vec3 consumerWithoutTransformsTransform2Translation;

        const DataSlotHandle providerSceneProviderSlotHandle;
        const DataSlotHandle consumer1SceneConsumerSlotHandle;
        const DataSlotHandle consumerWithoutTransformsSceneConsumerSlotHandle;

        const DataSlotId providerSceneProviderId;
        const DataSlotId consumer1SceneConsumerId;
        const DataSlotId consumer1SceneProviderId;
        const DataSlotId consumer2SceneConsumerId;
        const DataSlotId consumer2ndLevelSceneConsumerId;
        const DataSlotId consumerWithoutTransformsSceneConsumerId;
    };

    TEST_F(ATransformationLinkCachedScene, resolvesSingleDataDependency)
    {
        linkConsumer1SceneToProviderScene();
        const glm::vec3 expectedTranslation = providerSceneRootTranslation + providerSceneNodeTranslation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, resolvesSingleDataDependencyForChildNodeOfConsumer)
    {
        linkConsumer1SceneToProviderScene();
        const glm::vec3 expectedTranslation = providerSceneRootTranslation + providerSceneNodeTranslation + consumer1SceneNode2Translation;
        ExpectCorrectMatrix(consumer1SceneNode2, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, resolvesSingleDataDependencyWithLinkToSceneWithRoot)
    {
        linkConsumer2SceneToProviderScene();
        const glm::vec3 expectedTranslation = providerSceneRootTranslation + providerSceneNodeTranslation;
        ExpectCorrectMatrix(consumer2SceneRoot, glm::translate(expectedTranslation), consumer2Scene);
    }

    TEST_F(ATransformationLinkCachedScene, doesNotAffectSceneWhenLinkingItsChildAsConsumer)
    {
        linkConsumer1SceneToProviderScene();
        const glm::vec3 expectedTranslation = consumer1SceneRootTranslation;
        ExpectCorrectMatrix(consumer1SceneRootNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, doesNotAffectSceneWithConsumerWhenLinkingItsChildAsProvider)
    {
        linkConsumer1SceneToProviderScene();
        linkConsumer2ndLevelSceneToConsumer1Scene();
        const glm::vec3 expectedTranslation = providerSceneRootTranslation + providerSceneNodeTranslation + consumer1SceneNode2Translation;
        ExpectCorrectMatrix(consumer2ndLevelSceneNode, glm::translate(expectedTranslation), consumer2ndLevelScene);
    }

    TEST_F(ATransformationLinkCachedScene, resolvesTransformationAcrossTwoLinks)
    {
        linkConsumer1SceneToProviderScene();
        linkConsumer2ndLevelSceneToConsumer1Scene();
        const glm::vec3 expectedTranslation1 = providerSceneRootTranslation + providerSceneNodeTranslation;
        const glm::vec3 expectedTranslation2 = providerSceneRootTranslation + providerSceneNodeTranslation + consumer1SceneNode2Translation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation1), consumer1Scene);
        ExpectCorrectMatrix(consumer1SceneNode2, glm::translate(expectedTranslation2), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, resolvesTransformationAndPropagatesToChild)
    {
        linkConsumer2SceneToProviderScene();
        const glm::vec3 expectedTranslation = providerSceneRootTranslation + providerSceneNodeTranslation + consumer2SceneNodeTranslation;
        ExpectCorrectMatrix(consumer2SceneNode, glm::translate(expectedTranslation), consumer2Scene);
    }

    TEST_F(ATransformationLinkCachedScene, resolvesTransformationAndPropagatesToSceneWithNoTransforms)
    {
        linkConsumerWithoutTransformsSceneToProviderScene();
        const glm::vec3 expectedTranslation1 = providerSceneRootTranslation + providerSceneNodeTranslation;
        ExpectCorrectMatrix(consumerWithoutTransformsNode1, glm::translate(expectedTranslation1), consumerWithoutTransformsScene);

        const glm::vec3 expectedTranslation2 = providerSceneRootTranslation + providerSceneNodeTranslation + consumerWithoutTransformsTransform2Translation;
        ExpectCorrectMatrix(consumerWithoutTransformsNode2, glm::translate(expectedTranslation2), consumerWithoutTransformsScene);
        ExpectCorrectMatrix(consumerWithoutTransformsNode3, glm::translate(expectedTranslation2), consumerWithoutTransformsScene);
    }

    TEST_F(ATransformationLinkCachedScene, resolvesTransformationAndPropagatesToSceneWithDifferentRotationConventions)
    {
        linkConsumer1SceneToProviderScene();

        //reset translations
        setAllTranslations(true);

        providerScene.setRotation(providerSceneTransform, { 10.f, 0.f, 0.f, 1.f }, ERotationType::Euler_XYZ); //only X-Axis

        consumer1Scene.setRotation(consumer1SceneTransform2, { 0.f, 10.f, 0.f, 1.f }, ERotationType::Euler_ZYX); //only Y-Axis
        ExpectCorrectMatrix(consumer1SceneNode2, Math3d::Rotation({ 10.f, 10.f, 0.f, 1.f }, ERotationType::Euler_ZYX), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, providerOverridesEulerWithQuaternion)
    {
        const glm::vec4 rProvider = {0.5f, 0.5f, 0.5f, 0.5f};
        const glm::vec4 rConsumer = {30.f, 0.f, 0.f, 1.f};

        // reset translations
        setAllTranslations(true);
        consumer1Scene.setRotation(consumer1SceneTransform, rConsumer, ERotationType::Euler_XYZ);
        providerScene.setRotation(providerSceneTransform, rProvider, ERotationType::Quaternion);

        linkConsumer1SceneToProviderScene();
        ExpectCorrectMatrix(consumer1SceneNode2, Math3d::Rotation(rProvider, ERotationType::Quaternion), consumer1Scene);

        sceneLinksManager.removeDataLink(consumer1Scene.getSceneId(), consumer1SceneConsumerId);
        ExpectCorrectMatrix(consumer1SceneNode2, Math3d::Rotation(rConsumer, ERotationType::Euler_XZY), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, fallsBackToOriginalTransformationWhenProviderSlotIsRemoved)
    {
        linkConsumer1SceneToProviderScene();
        providerScene.releaseDataSlot(providerSceneProviderSlotHandle);
        const glm::vec3 expectedTranslation = consumer1SceneRootTranslation + consumer1SceneNodeTranslation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, fallsBackToOriginalTransformationWhenConsumerSlotIsRemoved)
    {
        linkConsumer1SceneToProviderScene();
        consumer1Scene.releaseDataSlot(consumer1SceneConsumerSlotHandle);
        const glm::vec3 expectedTranslation = consumer1SceneRootTranslation + consumer1SceneNodeTranslation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, recomputesDirtyTransformationWhenProviderSlotIsRemoved)
    {
        linkConsumer1SceneToProviderScene();
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, consumer1SceneNode);
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, consumer1SceneNode);

        providerScene.releaseDataSlot(providerSceneProviderSlotHandle);

        const glm::vec3 expectedTranslation = consumer1SceneRootTranslation + consumer1SceneNodeTranslation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, recomputesDirtyTransformationWhenConsumerSlotIsRemoved)
    {
        linkConsumer1SceneToProviderScene();
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, consumer1SceneNode);
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, consumer1SceneNode);

        consumer1Scene.releaseDataSlot(consumer1SceneConsumerSlotHandle);

        const glm::vec3 expectedTranslation = consumer1SceneRootTranslation + consumer1SceneNodeTranslation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, recomputesDirtyTransformationWhenProviderTransformationChanges)
    {
        linkConsumer1SceneToProviderScene();
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, consumer1SceneNode);
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, consumer1SceneNode);

        const glm::vec3 newRootTranslation(1.f);
        providerScene.setTranslation(providerSceneRootTransform, newRootTranslation);

        const glm::vec3 expectedTranslation = newRootTranslation + providerSceneNodeTranslation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, recomputesDirtyTransformationWhenProviderRotationConventionChanges)
    {
        linkConsumer1SceneToProviderScene();

        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, consumer1SceneNode);
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, consumer1SceneNode);

        setAllTranslations(true);
        const glm::vec4 rotation{ 10.f, 20.f, 30.f, 1.f };
        providerScene.setRotation(providerSceneTransform, rotation, ERotationType::Euler_XYZ);

        ExpectCorrectMatrix(consumer1SceneNode, Math3d::Rotation(rotation, ERotationType::Euler_XYZ), consumer1Scene);

        //change rotationType
        providerScene.setRotation(providerSceneTransform, rotation, ERotationType::Euler_ZYX);
        ExpectCorrectMatrix(consumer1SceneNode, Math3d::Rotation(rotation, ERotationType::Euler_ZYX), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, recomputesDirtyTransformationWhenProviderSceneRemoved)
    {
        linkConsumer1SceneToProviderScene();
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_World, consumer1SceneNode);
        std::ignore = consumer1Scene.updateMatrixCacheWithLinks(ETransformationMatrixType_Object, consumer1SceneNode);

        rendererScenes.destroyScene(providerScene.getSceneId());

        const glm::vec3 expectedTranslation = consumer1SceneRootTranslation + consumer1SceneNodeTranslation;
        ExpectCorrectMatrix(consumer1SceneNode, glm::translate(expectedTranslation), consumer1Scene);
    }

    TEST_F(ATransformationLinkCachedScene, canreleaseDataSlotOnUnlinkedProvider)
    {
        providerScene.releaseDataSlot(providerSceneProviderSlotHandle);
        EXPECT_FALSE(providerScene.isDataSlotAllocated(providerSceneProviderSlotHandle));
    }

    TEST_F(ATransformationLinkCachedScene, canreleaseDataSlotOnUnlinkedConsumer)
    {
        consumer1Scene.releaseDataSlot(consumer1SceneConsumerSlotHandle);
        EXPECT_FALSE(consumer1Scene.isDataSlotAllocated(consumer1SceneConsumerSlotHandle));
    }

    TEST_F(ATransformationLinkCachedScene, canTriggerCorrectEventWhenAllocateTransformProviderSlot)
    {
        consumeSceneControlEvents();

        const DataSlotId dataSlotId(123u);
        const DataSlotHandle slotHandle(345u);
        providerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationProvider, dataSlotId, providerSceneNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
        const RendererEventVector events = consumeSceneControlEvents();

        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataSlotProviderCreated, events.front().eventType);
        EXPECT_EQ(providerScene.getSceneId(), events.front().providerSceneId);
        EXPECT_EQ(dataSlotId, events.front().providerdataId);
    }

    TEST_F(ATransformationLinkCachedScene, canTriggerCorrectEventWhenReleaseTransformProviderSlot)
    {
        consumeSceneControlEvents();

        providerScene.releaseDataSlot(providerSceneProviderSlotHandle);

        const RendererEventVector events = consumeSceneControlEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataSlotProviderDestroyed, events.front().eventType);
        EXPECT_EQ(providerScene.getSceneId(), events.front().providerSceneId);
        EXPECT_EQ(providerSceneProviderId, events.front().providerdataId);
    }

    TEST_F(ATransformationLinkCachedScene, canTriggerCorrectEventWhenAllocateTransformConsumerSlot)
    {
        consumeSceneControlEvents();

        const DataSlotId dataSlotId(123u);
        const DataSlotHandle slotHandle(345u);
        consumer1SceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, dataSlotId, consumer1SceneNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
        const RendererEventVector events = consumeSceneControlEvents();

        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataSlotConsumerCreated, events.front().eventType);
        EXPECT_EQ(consumer1Scene.getSceneId(), events.front().consumerSceneId);
        EXPECT_EQ(dataSlotId, events.front().consumerdataId);
    }

    TEST_F(ATransformationLinkCachedScene, canTriggerCorrectEventWhenReleaseTransformConsumerSlot)
    {
        consumeSceneControlEvents();

        consumer1Scene.releaseDataSlot(consumer1SceneConsumerSlotHandle);
        const RendererEventVector events = consumeSceneControlEvents();

        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataSlotConsumerDestroyed, events.front().eventType);
        EXPECT_EQ(consumer1Scene.getSceneId(), events.front().consumerSceneId);
        EXPECT_EQ(consumer1SceneConsumerId, events.front().consumerdataId);
    }
}
