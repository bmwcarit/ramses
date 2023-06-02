//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-utils.h"

#include "NodeImpl.h"

#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "Math3d/Rotation.h"
#include "glm/gtx/transform.hpp"

#include <string_view>

using namespace testing;

namespace ramses
{
    template <typename NodeType>
    class NodeTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        NodeType& createNode(std::string_view name)
        {
            return (this->template createObject<NodeType>(name));
        }
    };

    TYPED_TEST_SUITE(NodeTest, NodeTypes);

    TYPED_TEST(NodeTest, hasChild)
    {
        Node& parent = this->createNode("parent");
        EXPECT_EQ(0u, parent.getChildCount());
        EXPECT_FALSE(parent.hasChild());

        EXPECT_EQ(StatusOK, parent.addChild(this->createNode("child")));
        EXPECT_TRUE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, hasParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(nullptr, child.getParent());
        EXPECT_FALSE(child.hasParent());

        EXPECT_EQ(StatusOK, child.setParent(parent));
        EXPECT_TRUE(child.hasParent());
    }

    TYPED_TEST(NodeTest, shouldAddChildrenToNodeAndCountThemCorrectly)
    {
        Node& parent = this->createNode("parent");
        EXPECT_EQ(0u, parent.getChildCount());

        for (uint32_t i = 0; i < 10; i++)
        {
            EXPECT_EQ(StatusOK, parent.addChild(this->createNode("child")));
            EXPECT_EQ(i + 1, parent.getChildCount());
        }

        EXPECT_TRUE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, shouldNotAddNodeFromOneSceneAsChildToNodeInOtherScene)
    {
        Scene& otherScene = *this->client.createScene(sceneId_t(1234u));
        CreationHelper otherSceneCreationHelper(&otherScene, &this->client);
        Node& parent = *otherSceneCreationHelper.template createObjectOfType<TypeParam>("parent");
        EXPECT_EQ(0u, parent.getChildCount());

        EXPECT_NE(StatusOK, parent.addChild(this->createNode("child")));
        EXPECT_EQ(0u, parent.getChildCount());

        EXPECT_FALSE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, shouldAddChildToNodeAndGetParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        EXPECT_EQ(&parent, child.getParent());
    }

    TYPED_TEST(NodeTest, constGetChildAndGetParentBehaveAsNonConstVersion)
    {
        Node& parent = this->createNode("parent");
        const Node& constParent = parent;
        Node& child = this->createNode("child");
        const Node& constChild = child;

        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_EQ(constParent.getChild(0), parent.getChild(0));
        EXPECT_EQ(constParent.getParent(), parent.getParent());
        EXPECT_EQ(constChild.getParent(), child.getParent());
    }

    TYPED_TEST(NodeTest, shouldRemoveChildOnDestruction)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(child.getParent() != nullptr);

        this->m_scene.destroy(child);

        EXPECT_EQ(nullptr, parent.getChild(0));
        EXPECT_FALSE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, shouldRemoveParentOnItsDestruction)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(child.getParent() != nullptr);

        this->m_scene.destroy(parent);

        EXPECT_EQ(nullptr, child.getParent());
        EXPECT_FALSE(child.hasParent());
    }

    TYPED_TEST(NodeTest, shouldRemoveChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(child.getParent() != nullptr);

        EXPECT_EQ(StatusOK, parent.removeChild(child));

        EXPECT_EQ(nullptr, parent.getChild(0));
        EXPECT_EQ(nullptr, child.getParent());
    }

    TYPED_TEST(NodeTest, shouldAddParentToNodeAndGetChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, child.setParent(parent));
        EXPECT_EQ(&child, parent.getChild(0));
    }

    TYPED_TEST(NodeTest, shouldRemoveParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, child.setParent(parent));

        ASSERT_TRUE(child.getParent() != nullptr);
        ASSERT_TRUE(parent.getChild(0) != nullptr);

        EXPECT_EQ(StatusOK, child.removeParent());

        EXPECT_EQ(nullptr, child.getParent());
        EXPECT_EQ(nullptr, parent.getChild(0));
    }

    TYPED_TEST(NodeTest, confidenceTest_shouldRemoveAllChildren)
    {
        Node& parent = this->createNode("parent");
        Node& child1 = this->createNode("child1");
        Node& child2 = this->createNode("child2");
        Node& child3 = this->createNode("child3");
        EXPECT_EQ(StatusOK, parent.addChild(child1));
        EXPECT_EQ(StatusOK, parent.addChild(child2));
        EXPECT_EQ(StatusOK, parent.addChild(child3));

        ASSERT_TRUE(parent.getChild(0) != nullptr);
        ASSERT_TRUE(parent.getChild(1) != nullptr);
        ASSERT_TRUE(parent.getChild(2) != nullptr);

        EXPECT_EQ(StatusOK, parent.removeAllChildren());

        EXPECT_EQ(nullptr, parent.getChild(0));
        EXPECT_EQ(nullptr, parent.getChild(1));
        EXPECT_EQ(nullptr, parent.getChild(2));
        EXPECT_EQ(nullptr, child1.getParent());
        EXPECT_EQ(nullptr, child2.getParent());
        EXPECT_EQ(nullptr, child3.getParent());
        EXPECT_TRUE(child1.m_impl.isDirty());
        EXPECT_TRUE(child2.m_impl.isDirty());
        EXPECT_TRUE(child3.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, reportsErrorWhenRemovinNonExistingParent)
    {
        Node& nodeWithNoParent = this->createNode("node");
        EXPECT_NE(StatusOK, nodeWithNoParent.removeParent());
    }

    TYPED_TEST(NodeTest, shouldReplaceParentWithaddChild)
    {
        Node& parent1 = this->createNode("parent1");
        Node& parent2 = this->createNode("parent2");
        Node& child = this->createNode("child");

        EXPECT_EQ(StatusOK, parent1.addChild(child));
        EXPECT_EQ(StatusOK, parent2.addChild(child));
        EXPECT_EQ(0u, parent1.getChildCount());
        EXPECT_EQ(&parent2, child.getParent());
    }

    TYPED_TEST(NodeTest, shouldReplaceParentWithSetParent)
    {
        Node& parent1 = this->createNode("parent1");
        Node& parent2 = this->createNode("parent2");
        Node& child = this->createNode("child");

        EXPECT_EQ(StatusOK, child.setParent(parent1));
        EXPECT_EQ(StatusOK, child.setParent(parent2));
        EXPECT_EQ(0u, parent1.getChildCount());
        EXPECT_EQ(&parent2, child.getParent());
    }

    TYPED_TEST(NodeTest, shouldOrderChildrenAccordingToTheirAddingToTheirParent)
    {
        Node& parent = this->createNode("parent");
        Node& child1 = this->createNode("child1");
        Node& child3 = this->createNode("child3");
        Node& child2 = this->createNode("child2");
        Node& child4 = this->createNode("child4");
        ASSERT_EQ(StatusOK, parent.addChild(child1));
        ASSERT_EQ(StatusOK, parent.addChild(child2));
        ASSERT_EQ(StatusOK, parent.addChild(child3));
        ASSERT_EQ(StatusOK, parent.addChild(child4));

        // children -> 1,2,3,4

        EXPECT_EQ(&child1, parent.getChild(0));
        EXPECT_EQ(&child2, parent.getChild(1));
        EXPECT_EQ(&child3, parent.getChild(2));
        EXPECT_EQ(&child4, parent.getChild(3));
    }

    TYPED_TEST(NodeTest, shouldAdaptChildOrderWhenChildrenAreRemovedFromParent)
    {
        Node& child1 = this->createNode("child1");
        Node& child3 = this->createNode("child3");
        Node& parent = this->createNode("parent");
        Node& child2 = this->createNode("child2");
        Node& child4 = this->createNode("child4");
        ASSERT_EQ(StatusOK, parent.addChild(child1));
        ASSERT_EQ(StatusOK, parent.addChild(child2));
        ASSERT_EQ(StatusOK, parent.addChild(child3));
        ASSERT_EQ(StatusOK, parent.addChild(child4));

        // children -> 1,2,3,4 (tested in other test)

        parent.removeChild(child2);

        // children -> 1,3,4

        EXPECT_EQ(&child1, parent.getChild(0));
        EXPECT_EQ(&child3, parent.getChild(1));
        EXPECT_EQ(&child4, parent.getChild(2));
        EXPECT_EQ(nullptr, parent.getChild(3));

        parent.removeChild(child1);

        // children -> 3,4

        EXPECT_EQ(&child3, parent.getChild(0));
        EXPECT_EQ(&child4, parent.getChild(1));
        EXPECT_EQ(nullptr, parent.getChild(2));
        EXPECT_EQ(nullptr, parent.getChild(3));

        parent.removeChild(child4);

        // children -> 3

        EXPECT_EQ(&child3, parent.getChild(0));
        EXPECT_EQ(nullptr, parent.getChild(1));
        EXPECT_EQ(nullptr, parent.getChild(2));
        EXPECT_EQ(nullptr, parent.getChild(3));
    }

    TYPED_TEST(NodeTest, canBeConvertedToNode)
    {
        RamsesObject& obj = this->createNode("node");
        EXPECT_TRUE(RamsesUtils::TryConvert<Node>(obj) != nullptr);
        const RamsesObject& constObj = obj;
        EXPECT_TRUE(RamsesUtils::TryConvert<Node>(constObj) != nullptr);
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingParentToItself)
    {
        Node& node = this->createNode("node");
        EXPECT_NE(StatusOK, node.setParent(node));
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingParaentFromAnotherScene)
    {
        Scene& anotherScene = *this->client.createScene(sceneId_t(12u));

        CreationHelper otherSceneCreationHelper(&anotherScene, &this->client);
        Node& parent = *otherSceneCreationHelper.template createObjectOfType<TypeParam>("parent");

        Node& node = this->createNode("node");
        EXPECT_NE(StatusOK, node.setParent(parent));
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingChildToItself)
    {
        Node& node = this->createNode("node");
        EXPECT_NE(StatusOK, node.addChild(node));
    }

    TYPED_TEST(NodeTest, isNotDirtyInitially)
    {
        Node& node = this->createNode("node");
        EXPECT_FALSE(node.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, canBeMarkedDirty)
    {
        Node& node = this->createNode("node");
        node.m_impl.markDirty();
        EXPECT_TRUE(node.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenSettingParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, child.setParent(parent));
        EXPECT_TRUE(child.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenAddingToParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_TRUE(child.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenRemovedFromParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        this->m_scene.flush(); // to clear dirty state

        EXPECT_EQ(StatusOK, parent.removeChild(child));
        EXPECT_TRUE(child.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, staysCleanWhenSettingChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_FALSE(parent.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenParentDestroyed)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        this->m_scene.flush(); // to clear dirty state

        this->m_scene.destroy(parent);
        EXPECT_TRUE(child.m_impl.isDirty());
    }

    TYPED_TEST(NodeTest, staysCleanWhenRemovingNonExistingParent)
    {
        Node& nodeWithNoParent = this->createNode("node");
        EXPECT_NE(StatusOK, nodeWithNoParent.removeParent());
        EXPECT_FALSE(nodeWithNoParent.m_impl.isDirty());
    }

    void expectMatricesEqual(const glm::mat4 mat1, const glm::mat4 mat2)
    {
        for (uint32_t i = 0u; i < 16u; ++i)
        {
            EXPECT_FLOAT_EQ(mat1[i / 4][i % 4], mat2[i / 4][i % 4]);
        }
    }

    TYPED_TEST(NodeTest, getsIdentityModelMatrixInitially)
    {
        Node& node = this->createNode("node");
        matrix44f modelMat;
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(glm::identity<glm::mat4>(), modelMat);
    }

    TYPED_TEST(NodeTest, setsRotationVectorAndConvention)
    {
        Node& node = this->createNode("node");

        EXPECT_EQ(ramses::StatusOK, node.setRotation({1.f, 2.f, 3.f}, ERotationType::Euler_XZX));

        const auto transformHandle = node.m_impl.getTransformHandle();

        vec3f euler;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));
        EXPECT_EQ(euler.x, 1.f);
        EXPECT_EQ(euler.y, 2.f);
        EXPECT_EQ(euler.z, 3.f);
        EXPECT_EQ(ERotationType::Euler_XZX, node.getRotationType());
        EXPECT_EQ(glm::vec3(1.f, 2.f, 3.f), glm::vec3(this->m_scene.m_impl.getIScene().getRotation(transformHandle)));
        EXPECT_EQ(ramses_internal::ERotationType::Euler_XZX, this->m_scene.m_impl.getIScene().getRotationType(transformHandle));

        EXPECT_EQ(ramses::StatusOK, node.setRotation({11.f, 12.f, 13.f}, ERotationType::Euler_XYZ));

        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));
        EXPECT_EQ(euler.x, 11.f);
        EXPECT_EQ(euler.y, 12.f);
        EXPECT_EQ(euler.z, 13.f);
        EXPECT_EQ(ERotationType::Euler_XYZ, node.getRotationType());
        EXPECT_EQ(glm::vec3(11.f, 12.f, 13.f), glm::vec3(this->m_scene.m_impl.getIScene().getRotation(transformHandle)));
        EXPECT_EQ(ramses_internal::ERotationType::Euler_XYZ, this->m_scene.m_impl.getIScene().getRotationType(transformHandle));
    }

    TYPED_TEST(NodeTest, returnsErrorWhenSettingQuaternionConvention)
    {
        Node& node = this->createNode("node");

        EXPECT_NE(StatusOK, node.setRotation({10.f, 20.f, 30.f}, ERotationType::Quaternion));

        // default value is still set
        EXPECT_EQ(ERotationType::Euler_XYZ, node.getRotationType());

        quat quaternion;
        EXPECT_EQ(StatusOK, node.getRotation(quaternion));
        EXPECT_EQ(quaternion, glm::identity<quat>());

        vec3f euler;
        EXPECT_EQ(StatusOK, node.getRotation(euler));
        EXPECT_EQ(euler, vec3f{0.f});

        const auto transformHandle = node.m_impl.getTransformHandle();
        EXPECT_FALSE(transformHandle.isValid());
    }

    TYPED_TEST(NodeTest, setsRotationQuaternion)
    {
        Node& node = this->createNode("node");

        const quat q{0.830048f, -0.2907008f, 0.4666782f, -0.093407f};
        EXPECT_EQ(StatusOK, node.setRotation(q));

        const auto transformHandle = node.m_impl.getTransformHandle();

        quat qOut;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(qOut));
        EXPECT_EQ(qOut.x, q.x);
        EXPECT_EQ(qOut.y, q.y);
        EXPECT_EQ(qOut.z, q.z);
        EXPECT_EQ(qOut.w, q.w);
        EXPECT_EQ(glm::vec4(q.x, q.y, q.z, q.w), this->m_scene.m_impl.getIScene().getRotation(transformHandle));
        EXPECT_EQ(ramses_internal::ERotationType::Quaternion, this->m_scene.m_impl.getIScene().getRotationType(transformHandle));
    }

    TYPED_TEST(NodeTest, setsRotationQuaternionThenEuler)
    {
        Node& node = this->createNode("node");

        const quat q{0.830048f, -0.2907008f, 0.4666782f, -0.093407f};
        EXPECT_EQ(StatusOK, node.setRotation(q));
        quat qOut;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(qOut));

        const auto transformHandle = node.m_impl.getTransformHandle();
        EXPECT_EQ(ramses::StatusOK, node.setRotation({11.f, 12.f, 13.f}, ERotationType::Euler_XYZ));

        vec3f euler;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));
        EXPECT_EQ(ERotationType::Euler_XYZ, node.getRotationType());
        EXPECT_EQ(euler.x, 11.f);
        EXPECT_EQ(euler.y, 12.f);
        EXPECT_EQ(euler.z, 13.f);
        EXPECT_EQ(glm::vec3(11.f, 12.f, 13.f), glm::vec3(this->m_scene.m_impl.getIScene().getRotation(transformHandle)));
        EXPECT_EQ(ramses_internal::ERotationType::Euler_XYZ, this->m_scene.m_impl.getIScene().getRotationType(transformHandle));
        EXPECT_NE(ramses::StatusOK, node.getRotation(qOut));
    }

    TYPED_TEST(NodeTest, setsRotationEulerThenQuaternion)
    {
        Node& node = this->createNode("node");

        EXPECT_EQ(ramses::StatusOK, node.setRotation({1.f, 2.f, 3.f}, ERotationType::Euler_XZX));

        vec3f euler;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));
        EXPECT_EQ(ERotationType::Euler_XZX, node.getRotationType());

        const quat q{0.830048f, -0.2907008f, 0.4666782f, -0.093407f};
        EXPECT_EQ(StatusOK, node.setRotation(q));

        const auto transformHandle = node.m_impl.getTransformHandle();

        quat qOut;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(qOut));
        EXPECT_EQ(qOut.x, q.x);
        EXPECT_EQ(qOut.y, q.y);
        EXPECT_EQ(qOut.z, q.z);
        EXPECT_EQ(qOut.w, q.w);
        EXPECT_EQ(glm::vec4(q.x, q.y, q.z, q.w), this->m_scene.m_impl.getIScene().getRotation(transformHandle));
        EXPECT_EQ(ramses_internal::ERotationType::Quaternion, this->m_scene.m_impl.getIScene().getRotationType(transformHandle));

        EXPECT_NE(ramses::StatusOK, node.getRotation(euler));
        EXPECT_EQ(ERotationType::Quaternion, node.getRotationType());
    }

    TYPED_TEST(NodeTest, getRotationReturnsErrorIfQuaternion)
    {
        Node& node = this->createNode("node");
        quat  q{0.5f, 0.5f, 0.5f, -0.5f};
        EXPECT_EQ(StatusOK, node.setRotation(q));
        vec3f euler;
        EXPECT_NE(StatusOK, node.getRotation(euler));
        EXPECT_EQ(ERotationType::Quaternion, node.getRotationType());
    }

    TYPED_TEST(NodeTest, getRotationQuaternionReturnsErrorIfEuler)
    {
        Node& node = this->createNode("node");
        quat  q;
        EXPECT_EQ(StatusOK, node.setRotation({90.f, 0.f, 0.f}));
        EXPECT_NE(StatusOK, node.getRotation(q));
    }

    TYPED_TEST(NodeTest, getRotationReturnsDefaultValuesWithoutSetBefore)
    {
        Node& node = this->createNode("node");

        vec3f euler;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));
        EXPECT_EQ(euler, vec3f{0.f});
        EXPECT_EQ(ERotationType::Euler_XYZ, node.getRotationType());
    }


    TYPED_TEST(NodeTest, getRotationQuaternionReturnsDefaultValuesWithoutSetBefore)
    {
        Node& node = this->createNode("node");
        quat q;
        EXPECT_EQ(ERotationType::Euler_XYZ, node.getRotationType());
        EXPECT_EQ(ramses::StatusOK, node.getRotation(q));
        EXPECT_EQ(0.f, q.x);
        EXPECT_EQ(0.f, q.y);
        EXPECT_EQ(0.f, q.z);
        EXPECT_EQ(1.f, q.w);
    }

    TYPED_TEST(NodeTest, settingScaleOrTranlationMayNotBreakGetRotation)
    {
        Node& node = this->createNode("node");

        vec3f euler;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));
        EXPECT_EQ(euler, vec3f{0.f});
        EXPECT_EQ(ERotationType::Euler_XYZ, node.getRotationType());

        EXPECT_EQ(ramses::StatusOK, node.setTranslation({2, 2, 2}));
        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));

        EXPECT_EQ(euler, vec3f{0.f});
        EXPECT_EQ(ERotationType::Euler_XYZ, node.getRotationType());
    }

    TYPED_TEST(NodeTest, canSetAndGetRotationAfterSettingScaleOrTranslation)
    {
        Node& node = this->createNode("node");

        EXPECT_EQ(ramses::StatusOK, node.setTranslation({2, 2, 2}));

        EXPECT_EQ(ramses::StatusOK, node.setRotation({1.f, 2.f, 3.f}, ERotationType::Euler_ZYZ));

        vec3f euler;
        EXPECT_EQ(ramses::StatusOK, node.getRotation(euler));

        EXPECT_EQ(euler, vec3f(1.f, 2.f, 3.f));
        EXPECT_EQ(ERotationType::Euler_ZYZ, node.getRotationType());
    }

    TYPED_TEST(NodeTest, getsIdentityInverseModelMatrixInitially)
    {
        Node& node = this->createNode("node");
        matrix44f modelMat;
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(modelMat));

        expectMatricesEqual(glm::identity<glm::mat4>(), modelMat);
    }

    TYPED_TEST(NodeTest, getsModelMatrixComputedFromTransformationChain_SingleNode)
    {
        Node& node = this->template createObject<Node>();

        node.setTranslation({1.f, 2.f, 3.f});
        node.setScaling({4.f, 5.f, 6.f});
        node.setRotation({7.f, 8.f, 9.f}, ERotationType::Euler_ZYX);

        const auto transMat = glm::translate(glm::vec3{ 1.f, 2.f, 3.f });
        const auto scaleMat = glm::scale(glm::vec3{ 4.f, 5.f, 6.f });
        const auto rotMat = ramses_internal::Math3d::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationType::Euler_ZYX);
        const auto expectedModelMat = transMat * rotMat * scaleMat;

        matrix44f modelMat;
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(expectedModelMat, modelMat);
    }

    TYPED_TEST(NodeTest, getsModelMatrixComputedFromTransformationChain_MultipleNodes)
    {
        Node& translationNode = this->template createObject<Node>();
        Node& scaleNode = this->template createObject<Node>();
        Node& rotateNode = this->template createObject<Node>();

        translationNode.setTranslation({1.f, 2.f, 3.f});
        scaleNode.setScaling({4.f, 5.f, 6.f});
        rotateNode.setRotation({7.f, 8.f, 9.f}, ERotationType::Euler_ZYX);

        translationNode.addChild(rotateNode);
        rotateNode.addChild(scaleNode);

        Node& node = this->createNode("node");
        scaleNode.addChild(node);

        const auto transMat = glm::translate(glm::vec3{ 1.f, 2.f, 3.f });
        const auto scaleMat = glm::scale(glm::vec3{ 4.f, 5.f, 6.f });
        const auto rotMat = ramses_internal::Math3d::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationType::Euler_ZYX);
        const auto expectedModelMat = transMat * rotMat * scaleMat;

        matrix44f modelMat;
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(expectedModelMat, modelMat);
    }

    TYPED_TEST(NodeTest, getsInverseModelMatrixComputedFromTransformationChain_SingleNode)
    {
        Node& node = this->template createObject<Node>();

        node.setTranslation({1.f, 2.f, 3.f});
        node.setScaling({4.f, 5.f, 6.f});
        node.setRotation({7.f, 8.f, 9.f}, ERotationType::Euler_YZX);

        const auto transMat = glm::translate(glm::vec3{ -1.f, -2.f, -3.f });
        const auto scaleMat = glm::inverse(glm::scale(glm::vec3{ 4.f, 5.f, 6.f }));
        const auto rotMat = glm::transpose(ramses_internal::Math3d::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationType::Euler_YZX));
        const auto expectedInverseModelMat = scaleMat * rotMat * transMat;

        matrix44f inverseModelMat;
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(inverseModelMat));

        expectMatricesEqual(expectedInverseModelMat, inverseModelMat);
    }

    TYPED_TEST(NodeTest, getsInverseModelMatrixComputedFromTransformationChain_MultipleNodes)
    {
        Node& translationNode = this->template createObject<Node>();
        Node& scaleNode = this->template createObject<Node>();
        Node& rotateNode = this->template createObject<Node>();

        translationNode.setTranslation({1.f, 2.f, 3.f});
        scaleNode.setScaling({4.f, 5.f, 6.f});
        rotateNode.setRotation({7.f, 8.f, 9.f}, ERotationType::Euler_YXY);

        translationNode.addChild(rotateNode);
        rotateNode.addChild(scaleNode);

        Node& node = this->createNode("node");
        scaleNode.addChild(node);

        const auto transMat = glm::translate(glm::vec3{ -1.f, -2.f, -3.f });
        const auto scaleMat = glm::inverse(glm::scale(glm::vec3{ 4.f, 5.f, 6.f }));
        const auto rotMat = glm::transpose(ramses_internal::Math3d::Rotation({ 7.f, 8.f, 9.f, 1.f }, ramses_internal::ERotationType::Euler_YXY));
        const auto expectedInverseModelMat = scaleMat * rotMat * transMat;

        matrix44f inverseModelMat;
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(inverseModelMat));

        expectMatricesEqual(expectedInverseModelMat, inverseModelMat);
    }

    TYPED_TEST(NodeTest, instantiationDoesNotCreateMultipleLLNodes)
    {
        const Node& node = this->createNode("node");

        // This is to take in account any additional objects(derived from Node) created by helper creation util
        // that the currently tested node depends on.
        // E.g. PickableObject depends on Camera which is a node as well and would be wrongly counted here.
        const size_t additionalAllocatedNodeCount = this->m_creationHelper.getAdditionalAllocatedNodeCount();

        EXPECT_EQ(1u, node.m_impl.getIScene().getNodeCount() - additionalAllocatedNodeCount);
        EXPECT_TRUE(this->m_internalScene.isNodeAllocated(node.m_impl.getNodeHandle()));
    }

    TYPED_TEST(NodeTest, destructionDestroysLLNode)
    {
        Node& node = this->createNode("node");
        const auto handle = node.m_impl.getNodeHandle();
        this->m_scene.destroy(node);
        EXPECT_FALSE(this->m_internalScene.isNodeAllocated(handle));
    }
}
