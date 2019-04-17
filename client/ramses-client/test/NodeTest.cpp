//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-utils.h"

#include "NodeImpl.h"

#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"

using namespace testing;

namespace ramses
{
    template <typename NodeType>
    class NodeTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        NodeType& createNode(const char* name)
        {
            return (this->template createObject<NodeType>(name));
        }
    };

    TYPED_TEST_CASE(NodeTest, NodeTypes);

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
        EXPECT_EQ(NULL, child.getParent());
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
        Scene& otherScene = *this->client.createScene(1234u);
        CreationHelper otherSceneCreationHelper(&otherScene, NULL, &this->client);
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

        ASSERT_TRUE(parent.getChild(0) != 0);
        ASSERT_TRUE(child.getParent() != 0);

        this->m_scene.destroy(child);

        EXPECT_EQ(NULL, parent.getChild(0));
        EXPECT_FALSE(parent.hasChild());
    }

    TYPED_TEST(NodeTest, shouldRemoveParentOnItsDestruction)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != 0);
        ASSERT_TRUE(child.getParent() != 0);

        this->m_scene.destroy(parent);

        EXPECT_EQ(NULL, child.getParent());
        EXPECT_FALSE(child.hasParent());
    }

    TYPED_TEST(NodeTest, shouldRemoveChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));

        ASSERT_TRUE(parent.getChild(0) != 0);
        ASSERT_TRUE(child.getParent() != 0);

        EXPECT_EQ(StatusOK, parent.removeChild(child));

        EXPECT_EQ(NULL, parent.getChild(0));
        EXPECT_EQ(NULL, child.getParent());
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

        ASSERT_TRUE(child.getParent() != 0);
        ASSERT_TRUE(parent.getChild(0) != 0);

        EXPECT_EQ(StatusOK, child.removeParent());

        EXPECT_EQ(NULL, child.getParent());
        EXPECT_EQ(NULL, parent.getChild(0));
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

        ASSERT_TRUE(parent.getChild(0) != 0);
        ASSERT_TRUE(parent.getChild(1) != 0);
        ASSERT_TRUE(parent.getChild(2) != 0);

        EXPECT_EQ(StatusOK, parent.removeAllChildren());

        EXPECT_EQ(NULL, parent.getChild(0));
        EXPECT_EQ(NULL, parent.getChild(1));
        EXPECT_EQ(NULL, parent.getChild(2));
        EXPECT_EQ(NULL, child1.getParent());
        EXPECT_EQ(NULL, child2.getParent());
        EXPECT_EQ(NULL, child3.getParent());
        EXPECT_TRUE(child1.impl.isDirty());
        EXPECT_TRUE(child2.impl.isDirty());
        EXPECT_TRUE(child3.impl.isDirty());
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
        EXPECT_EQ(0, parent.getChild(3));

        parent.removeChild(child1);

        // children -> 3,4

        EXPECT_EQ(&child3, parent.getChild(0));
        EXPECT_EQ(&child4, parent.getChild(1));
        EXPECT_EQ(0, parent.getChild(2));
        EXPECT_EQ(0, parent.getChild(3));

        parent.removeChild(child4);

        // children -> 3

        EXPECT_EQ(&child3, parent.getChild(0));
        EXPECT_EQ(0, parent.getChild(1));
        EXPECT_EQ(0, parent.getChild(2));
        EXPECT_EQ(0, parent.getChild(3));
    }

    TYPED_TEST(NodeTest, canBeConvertedToNode)
    {
        RamsesObject& obj = this->createNode("node");
        EXPECT_TRUE(RamsesUtils::TryConvert<Node>(obj) != NULL);
        const RamsesObject& constObj = obj;
        EXPECT_TRUE(RamsesUtils::TryConvert<Node>(constObj) != NULL);
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingParentToItself)
    {
        Node& node = this->createNode("node");
        EXPECT_NE(StatusOK, node.setParent(node));
    }

    TYPED_TEST(NodeTest, reportsErrorWhenSettingParaentFromAnotherScene)
    {
        Scene& anotherScene = *this->client.createScene(12u);

        CreationHelper otherSceneCreationHelper(&anotherScene, NULL, &this->client);
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
        EXPECT_FALSE(node.impl.isDirty());
    }

    TYPED_TEST(NodeTest, canBeMarkedDirty)
    {
        Node& node = this->createNode("node");
        node.impl.markDirty();
        EXPECT_TRUE(node.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenSettingParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, child.setParent(parent));
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenAddingToParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenRemovedFromParent)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        this->m_scene.flush(); // to clear dirty state

        EXPECT_EQ(StatusOK, parent.removeChild(child));
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, staysCleanWhenSettingChild)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        EXPECT_FALSE(parent.impl.isDirty());
    }

    TYPED_TEST(NodeTest, isMarkedDirtyWhenParentDestroyed)
    {
        Node& parent = this->createNode("parent");
        Node& child = this->createNode("child");
        EXPECT_EQ(StatusOK, parent.addChild(child));
        this->m_scene.flush(); // to clear dirty state

        this->m_scene.destroy(parent);
        EXPECT_TRUE(child.impl.isDirty());
    }

    TYPED_TEST(NodeTest, staysCleanWhenRemovingNonExistingParent)
    {
        Node& nodeWithNoParent = this->createNode("node");
        EXPECT_NE(StatusOK, nodeWithNoParent.removeParent());
        EXPECT_FALSE(nodeWithNoParent.impl.isDirty());
    }

    void expectMatricesEqual(const float mat1[16], const float mat2[16])
    {
        for (uint32_t i = 0u; i < 16u; ++i)
        {
            EXPECT_FLOAT_EQ(mat1[i], mat2[i]);
        }
    }

    TYPED_TEST(NodeTest, getsIdentityModelMatrixInitially)
    {
        Node& node = this->createNode("node");
        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(ramses_internal::Matrix44f::Identity.data, modelMat);
    }

    TYPED_TEST(NodeTest, getsIdentityInverseModelMatrixInitially)
    {
        Node& node = this->createNode("node");
        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(modelMat));

        expectMatricesEqual(ramses_internal::Matrix44f::Identity.data, modelMat);
    }

    TYPED_TEST(NodeTest, getsModelMatrixComputedFromTransformationChain)
    {
        Node& translationNode = this->template createObject<Node>();
        Node& scaleNode = this->template createObject<Node>();
        Node& rotateNode = this->template createObject<Node>();

        translationNode.setTranslation(1.f, 2.f, 3.f);
        scaleNode.setScaling(4.f, 5.f, 6.f);
        rotateNode.setRotation(7.f, 8.f, 9.f);

        translationNode.addChild(scaleNode);
        scaleNode.addChild(rotateNode);

        Node& node = this->createNode("node");
        rotateNode.addChild(node);

        static const ramses_internal::Matrix44f transMat = ramses_internal::Matrix44f::Translation(1.f, 2.f, 3.f);
        static const ramses_internal::Matrix44f scaleMat = ramses_internal::Matrix44f::Scaling(4.f, 5.f, 6.f);
        static const ramses_internal::Matrix44f rotMat = ramses_internal::Matrix44f::RotationEulerZYX(7.f, 8.f, 9.f);
        static const ramses_internal::Matrix44f expectedModelMat = transMat * scaleMat * rotMat;

        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getModelMatrix(modelMat));

        expectMatricesEqual(expectedModelMat.data, modelMat);
    }

    TYPED_TEST(NodeTest, getsInverseModelMatrixComputedFromTransformationChain)
    {
        Node& translationNode = this->template createObject<Node>();
        Node& scaleNode = this->template createObject<Node>();
        Node& rotateNode = this->template createObject<Node>();

        translationNode.setTranslation(1.f, 2.f, 3.f);
        scaleNode.setScaling(4.f, 5.f, 6.f);
        rotateNode.setRotation(7.f, 8.f, 9.f);

        translationNode.addChild(scaleNode);
        scaleNode.addChild(rotateNode);

        Node& node = this->createNode("node");
        rotateNode.addChild(node);

        static const ramses_internal::Matrix44f transMat = ramses_internal::Matrix44f::Translation(-1.f, -2.f, -3.f);
        static const ramses_internal::Matrix44f scaleMat = ramses_internal::Matrix44f::Scaling(4.f, 5.f, 6.f).inverse();
        static const ramses_internal::Matrix44f rotMat = ramses_internal::Matrix44f::RotationEulerZYX(7.f, 8.f, 9.f).transpose();
        static const ramses_internal::Matrix44f expectedModelMat = rotMat * scaleMat * transMat;

        float modelMat[16] = { 0.f };
        EXPECT_EQ(StatusOK, node.getInverseModelMatrix(modelMat));

        expectMatricesEqual(expectedModelMat.data, modelMat);
    }

    TYPED_TEST(NodeTest, instantiationCreatesExactlyOneLLNode)
    {
        const Node& node = this->createNode("node");
        EXPECT_EQ(1u, node.impl.getIScene().getNodeCount());
        EXPECT_TRUE(this->m_internalScene.isNodeAllocated(node.impl.getNodeHandle()));
    }

    TYPED_TEST(NodeTest, destructionDestroysLLNode)
    {
        Node& node = this->createNode("node");
        const auto handle = node.impl.getNodeHandle();
        this->m_scene.destroy(node);
        EXPECT_FALSE(this->m_internalScene.isNodeAllocated(handle));
    }
}
