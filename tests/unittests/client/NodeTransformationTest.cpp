//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses/client/MeshNode.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/PickableObject.h"

#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "TestEqualHelper.h"
#include "internal/Core/Math3d/Rotation.h"

namespace ramses::internal
{
    using namespace testing;

    template <typename T>
    class NodeTransformationTest : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_node = &this->template createObject<T>("node");
        }

        T* m_node{nullptr};
    };

    TYPED_TEST_SUITE(NodeTransformationTest, NodeTypes);

    TYPED_TEST(NodeTransformationTest, setTranslate)
    {
        vec3f initialTranslation(0.f, 0.f, 0.f);
        vec3f actualTranslation;
        EXPECT_TRUE(this->m_node->getTranslation(actualTranslation));
        EXPECT_EQ(initialTranslation, actualTranslation);

        vec3f translationVector(1.2f, 2.3f, 4.5f);
        EXPECT_TRUE(this->m_node->setTranslation(translationVector));
        EXPECT_TRUE(this->m_node->getTranslation(actualTranslation));
        EXPECT_EQ(translationVector, actualTranslation);
    }

    TYPED_TEST(NodeTransformationTest, translate)
    {
        vec3f initialTranslation(0.f, 0.f, 0.f);
        vec3f actualTranslation;
        EXPECT_TRUE(this->m_node->getTranslation(actualTranslation));
        EXPECT_EQ(initialTranslation, actualTranslation);

        vec3f translationVector(1.2f, 2.3f, 4.5f);
        EXPECT_TRUE(this->m_node->translate(translationVector));
        EXPECT_TRUE(this->m_node->translate(translationVector));
        EXPECT_TRUE(this->m_node->getTranslation(actualTranslation));
        EXPECT_EQ(2.f * translationVector, actualTranslation);
    }

    TYPED_TEST(NodeTransformationTest, setRotation)
    {
        vec3f initialRotation(0.f, 0.f, 0.f);
        vec3f actualRotation;
        EXPECT_TRUE(this->m_node->getRotation(actualRotation));
        EXPECT_EQ(ERotationType::Euler_XYZ, this->m_node->getRotationType());
        EXPECT_EQ(initialRotation, actualRotation);

        vec3f rotationVector_1(1.2f, 2.3f, 4.5f);
        EXPECT_TRUE(this->m_node->setRotation(rotationVector_1, ERotationType::Euler_ZYX));
        EXPECT_TRUE(this->m_node->getRotation(actualRotation));
        EXPECT_EQ(rotationVector_1, actualRotation);
        EXPECT_EQ(ERotationType::Euler_ZYX, this->m_node->getRotationType());

        vec3f rotationVector_2(2.2f, 3.3f, 5.5f);
        EXPECT_TRUE(this->m_node->setRotation(rotationVector_2, ERotationType::Euler_ZYZ));
        EXPECT_TRUE(this->m_node->getRotation(actualRotation));
        EXPECT_EQ(rotationVector_2, actualRotation);
        EXPECT_EQ(ERotationType::Euler_ZYZ, this->m_node->getRotationType());
    }

    TYPED_TEST(NodeTransformationTest, setScaling)
    {
        vec3f initialScaling(1.f, 1.f, 1.f);
        vec3f actualScale;
        EXPECT_TRUE(this->m_node->getScaling(actualScale));
        EXPECT_EQ(initialScaling, actualScale);

        vec3f scalingVector_1(1.2f, 2.3f, 4.5f);
        EXPECT_TRUE(this->m_node->setScaling(scalingVector_1));
        EXPECT_TRUE(this->m_node->getScaling(actualScale));
        EXPECT_EQ(scalingVector_1, actualScale);

        vec3f scalingVector_2(2.2f, 3.3f, 5.5f);
        EXPECT_TRUE(this->m_node->setScaling(scalingVector_2));
        EXPECT_TRUE(this->m_node->getScaling(actualScale));
        EXPECT_EQ(scalingVector_2, actualScale);
    }

    TYPED_TEST(NodeTransformationTest, scale)
    {
        vec3f initialScaling(1.f, 1.f, 1.f);
        vec3f actualScale;
        EXPECT_TRUE(this->m_node->getScaling(actualScale));
        EXPECT_EQ(initialScaling, actualScale);

        vec3f scalingVector_1(4.f, 6.f, 8.f);
        EXPECT_TRUE(this->m_node->scale(scalingVector_1));
        EXPECT_TRUE(this->m_node->getScaling(actualScale));
        EXPECT_EQ(scalingVector_1, actualScale);

        vec3f scalingVector_2(0.5f, 0.5f, 0.5f);
        EXPECT_TRUE(this->m_node->scale(scalingVector_2));

        vec3f resultVector(2.f, 3.f, 4.f);
        EXPECT_TRUE(this->m_node->getScaling(actualScale));
        EXPECT_EQ(resultVector, actualScale);
    }

    TYPED_TEST(NodeTransformationTest, rotateMixConventions)
    {
        /*
                     node
                (10, 0, 0, Euler_ZYX)
                /              \
             chlid0           child1
         (0, 20, 0, Euler_XYZ)    (0, 20, 30, Euler_ZYZ)
                |
            grandChild
          (0, 0, 30, Euler_XZY)
        */
        TypeParam* child0 = &this->template createObject<TypeParam>("child0 node");
        TypeParam* child1 = &this->template createObject<TypeParam>("child1 node");
        TypeParam* grandChild = &this->template createObject<TypeParam>("grand child node");
        this->m_node->addChild(*child0);
        this->m_node->addChild(*child1);
        child0->addChild(*grandChild);

        EXPECT_TRUE(this->m_node->setRotation({10.f, 0.f, 0.f}, ERotationType::Euler_ZYX));
        EXPECT_TRUE(child0->setRotation({0.f, 20.f, 0.f}, ERotationType::Euler_XYZ));
        EXPECT_TRUE(child1->setRotation({0.f, 20.f, 30.f}, ERotationType::Euler_ZYZ));
        EXPECT_TRUE(grandChild->setRotation({0.f, 0.f, 30.f}, ERotationType::Euler_XZY));

        //expected matrices for rotation after transformation chain is applied
        const auto expectedNodeRotationMatrix           = ramses::internal::Math3d::Rotation({ 10.f , 0.f , 0.f, 1.f  }, ERotationType::Euler_ZYX);
        const auto expectedChild0RorationMatrix         = ramses::internal::Math3d::Rotation({ 10.f , 20.f, 0.f, 1.f  }, ERotationType::Euler_ZYX);
        const auto expectedChild1RorationMatrix         = ramses::internal::Math3d::Rotation({ 10.f , 20.f, 30.f, 1.f }, ERotationType::Euler_ZYX);
        const auto expectedGrandChildRorationMatrix     = ramses::internal::Math3d::Rotation({ 10.f , 20.f, 30.f, 1.f }, ERotationType::Euler_ZYX);

        matrix44f resultNodeMatrix;
        matrix44f resultChild0Matrix;
        matrix44f resultChild1Matrix;
        matrix44f resultGrandChildMatrix;
        this->m_node->getModelMatrix(resultNodeMatrix);
        child0      ->getModelMatrix(resultChild0Matrix);
        child1      ->getModelMatrix(resultChild1Matrix);
        grandChild  ->getModelMatrix(resultGrandChildMatrix);

        ramses::internal::expectMatrixFloatEqual(expectedNodeRotationMatrix       , resultNodeMatrix);
        ramses::internal::expectMatrixFloatEqual(expectedChild0RorationMatrix     , resultChild0Matrix);
        ramses::internal::expectMatrixFloatEqual(expectedChild1RorationMatrix     , resultChild1Matrix);
        ramses::internal::expectMatrixFloatEqual(expectedGrandChildRorationMatrix , resultGrandChildMatrix);
    }

    template <typename T>
    class NodeTransformationTestWithPublishedScene : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        void SetUp() override
        {
            const ramses::internal::IScene& iscene = this->m_scene.impl().getIScene();
            ramses::internal::SceneInfo info(iscene.getSceneId(), iscene.getName());
            EXPECT_CALL(this->sceneActionsCollector, handleNewSceneAvailable(info, _));
            EXPECT_CALL(this->sceneActionsCollector, handleInitializeScene(info, _));
            EXPECT_TRUE(m_scene.publish(EScenePublicationMode::LocalOnly));

            m_node = &this->template createObject<T>("node");
        }

        void TearDown() override
        {
            EXPECT_CALL(this->sceneActionsCollector, handleSceneBecameUnavailable(ramses::internal::SceneId(this->m_scene.impl().getSceneId().getValue()), _));
            EXPECT_TRUE(m_scene.unpublish());
        }

        T* m_node{nullptr};
    };

    TYPED_TEST_SUITE(NodeTransformationTestWithPublishedScene, NodeTypes);

    TYPED_TEST(NodeTransformationTestWithPublishedScene, setTranslateWithValuesEqualToCurrentValuesDoesNotCreateSceneActions)
    {
        vec3f translationVector(1.2f, 2.3f, 4.5f);
        EXPECT_CALL(this->sceneActionsCollector, handleSceneUpdate_rvr(ramses::internal::SceneId(this->m_scene.getSceneId().getValue()), _, _));
        EXPECT_TRUE(this->m_node->setTranslation(translationVector));
        this->m_scene.flush();
        EXPECT_LE(1u, this->sceneActionsCollector.getNumberOfActions());

        Mock::VerifyAndClearExpectations(this);
        this->sceneActionsCollector.resetCollecting();

        EXPECT_TRUE(this->m_node->setTranslation(translationVector));
        this->m_scene.flush();
        EXPECT_EQ(0u, this->sceneActionsCollector.getNumberOfActions());  // flush empty and optimized away
    }

    TYPED_TEST(NodeTransformationTestWithPublishedScene, setRotationWithValuesEqualToCurrentValuesDoesNotCreateSceneActions)
    {
        vec3f rotationVector(1.2f, 2.3f, 4.5f);
        EXPECT_CALL(this->sceneActionsCollector, handleSceneUpdate_rvr(ramses::internal::SceneId(this->m_scene.getSceneId().getValue()), _, _));
        EXPECT_TRUE(this->m_node->setRotation(rotationVector, ERotationType::Euler_YXZ));
        this->m_scene.flush();
        EXPECT_LE(1u, this->sceneActionsCollector.getNumberOfActions());

        Mock::VerifyAndClearExpectations(this);
        this->sceneActionsCollector.resetCollecting();

        EXPECT_TRUE(this->m_node->setRotation(rotationVector, ERotationType::Euler_YXZ));
        this->m_scene.flush();
        EXPECT_EQ(0u, this->sceneActionsCollector.getNumberOfActions());  // flush empty and optimized away
    }

    TYPED_TEST(NodeTransformationTestWithPublishedScene, setScalingWithValuesEqualToCurrentValuesDoesNotCreateSceneActions)
    {
        vec3f scalingVector(1.2f, 2.3f, 4.5f);
        EXPECT_CALL(this->sceneActionsCollector, handleSceneUpdate_rvr(ramses::internal::SceneId(this->m_scene.getSceneId().getValue()), _, _));
        EXPECT_TRUE(this->m_node->setScaling(scalingVector));
        this->m_scene.flush();
        EXPECT_LE(1u, this->sceneActionsCollector.getNumberOfActions());

        Mock::VerifyAndClearExpectations(this);
        this->sceneActionsCollector.resetCollecting();

        EXPECT_TRUE(this->m_node->setScaling(scalingVector));
        this->m_scene.flush();
        EXPECT_EQ(0u, this->sceneActionsCollector.getNumberOfActions());  // flush empty and optimized away
    }
}
