//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "../../ramses-client/test/SimpleSceneTopology.h"

#include "ramses-client-api/Node.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-hmi-utils.h"
#include "ramses-utils.h"

using namespace testing;

namespace ramses
{
    // Some helper methods used in all fixtures
    class BoundingSphereTestUtils
    {
    protected:

        static BoundingSphere CreateSphere(float x, float y, float z, float radius)
        {
            BoundingSphere bs;
            bs.xPos = x;
            bs.yPos = y;
            bs.zPos = z;
            bs.radius = radius;
            return bs;
        }

        static bool IsDefault(const BoundingSphere& sphere)
        {
            return
                sphere.xPos == 0 &&
                sphere.yPos == 0 &&
                sphere.zPos == 0 &&
                sphere.radius == 0;
        }
    };

    class ASceneWithMultipleNodes : public SimpleSceneTopology, public BoundingSphereTestUtils
    {
        /*
        The SimpleSceneTopology provides the following scene graph:

                        m_root
                    /          \
                m_vis1          m_vis2
                /      \          /    \
        m_mesh1a  m_mesh1b  m_mesh2a  m_mesh2b
        */

    public:
        ASceneWithMultipleNodes() : SimpleSceneTopology()
        {
        }
    };

    class ASceneWithSingleNode : public LocalTestClientWithScene, public ::testing::Test, public BoundingSphereTestUtils
    {

    public:
        ASceneWithSingleNode()
            : m_root(*m_scene.createNode("root"))
        {
        }

    protected:
        Node& m_root;
    };

    TEST_F(ASceneWithSingleNode, WhenStoringABoundingSphereItShouldBeReturnedUnchanged)
    {
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_root, CreateSphere(1.f, 2.f, 3.f, 4.f));

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 1.f);
        EXPECT_EQ(result.yPos, 2.f);
        EXPECT_EQ(result.zPos, 3.f);
        EXPECT_EQ(result.radius, 4.f);
    }

    TEST_F(ASceneWithMultipleNodes, WhenStoringMultipleBoundingSpheresForTheSameNodeOnlyTheLastBoundingSphereShouldBeStored)
    {
        // Setting multiple bounding spheres for the same node should override
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_root, CreateSphere(1.f, 2.f, 3.f, 4.f));
        collection.setBoundingSphere(m_root, CreateSphere(5.f, 6.f, 7.f, 8.f)); // Override

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 5.f);
        EXPECT_EQ(result.yPos, 6.f);
        EXPECT_EQ(result.zPos, 7.f);
        EXPECT_EQ(result.radius, 8.f);
    }

    TEST_F(ASceneWithSingleNode, WhenStoringMultipleBoundingSpheresForTheSameNodeOnlyTheLastBoundingSphereShouldBeStored)
    {
        // Setting multiple bounding spheres for the same node should override
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_root, CreateSphere(1.f, 2.f, 3.f, 4.f));
        collection.setBoundingSphere(m_root, CreateSphere(5.f, 6.f, 7.f, 8.f)); // Override

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 5.f);
        EXPECT_EQ(result.yPos, 6.f);
        EXPECT_EQ(result.zPos, 7.f);
        EXPECT_EQ(result.radius, 8.f);
    }

    TEST_F(ASceneWithMultipleNodes, WhenRemovingABoundingSphereFromANodeItShouldReturnDefaultSphereAfterwards)
    {
        BoundingSphereCollection collection(this->getScene());
        collection.removeBoundingSphere(m_root);

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_TRUE(IsDefault(result));
    }

    TEST_F(ASceneWithSingleNode, WhenRemovingABoundingSphereFromANodeItShouldReturnDefaultSphereAfterwards)
    {
        BoundingSphereCollection collection(this->getScene());
        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_TRUE(IsDefault(result));
    }

    TEST_F(ASceneWithMultipleNodes, GivenARootNodeWithNoBoundingSpheresADefaultSphereShouldBeReturned)
    {
        // No bounding spheres have been set. The scene should be traversed, but the end-result should be a sphere of size 0.f
        BoundingSphereCollection collection(this->getScene());
        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_TRUE(IsDefault(result));
    }

    TEST_F(ASceneWithMultipleNodes, GivenALeafNodeWithNoBoundingSpheresADefaultSphereShouldBeReturned)
    {
        // Same test as above, but for one of the leaf nodes
        BoundingSphereCollection collection(this->getScene());
        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_mesh1b);
        EXPECT_TRUE(IsDefault(result));
    }

    TEST_F(ASceneWithMultipleNodes, GivenASceneWithNodeWithBoundingSphereLocatedAtLeafShouldBeFoundWhenTraversingFromRootNode)
    {
        // Single node with bounding sphere, located at the leaf of the node graph. Should be found when traversing from the root node.
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_mesh2a, CreateSphere(1.f, 2.f, 3.f, 4.f));

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 1.f);
        EXPECT_EQ(result.yPos, 2.f);
        EXPECT_EQ(result.zPos, 3.f);
        EXPECT_EQ(result.radius, 4.f);
    }

    TEST_F(ASceneWithMultipleNodes, GivenASceneWithTwoLeafNodesWithBoundingSpheresAtSeparateSidesOfTheGraphShouldReturnAnEnclosingBoundingSphereOverBothNodes)
    {
        // Multiple child nodes with bounding spheres, the end-result should an enclosing sphere over all spheres.
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_mesh1a, CreateSphere(-2.f, 1.f, 0.f, 2.f)); // Left side of graph
        collection.setBoundingSphere(m_mesh2b, CreateSphere(2.f, 1.f, 0.f, 2.f)); // Right side of graph

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 0.f);
        EXPECT_EQ(result.yPos, 1.f);
        EXPECT_EQ(result.zPos, 0.f);
        EXPECT_GE(result.radius, 4.f);
    }

    TEST_F(ASceneWithMultipleNodes, GivenASceneWithTwoLeafNodesWithBoundingSpheresWhereOneOfThemIsInvisibleOnlyTheVisibleNodeShouldBeFound)
    {
        // Two leaf nodes with bounding spheres: One of each side of the graph. One of them will be hidden by visibility
        // of the parent. The expected result is that only the bounding sphere on the other side is found.
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_mesh1b, CreateSphere(5.f, 0.f, 0.f, 2.f)); // Left side of graph (should be invisible)
        collection.setBoundingSphere(m_mesh2a, CreateSphere(-3.f, 1.f, -1.f, 4.f)); // Right side of graph (the expected result)

        // Hide parent for left leaf
        m_vis1.setVisibility(false);

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, -3.f);
        EXPECT_EQ(result.yPos, 1.f);
        EXPECT_EQ(result.zPos, -1.f);
        EXPECT_EQ(result.radius, 4.f);
    }

    TEST_F(ASceneWithMultipleNodes, GivenANodeWithScaleTheComputedBoundingSphereShouldAlsoBeScaled)
    {
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_mesh1b, CreateSphere(0.f, 0.f, 0.f, 10.f));

        Node& newNode = *getScene().createNode();
        newNode.setScaling(2.f, 2.f, 2.f);

        // Inject the new node
        m_vis1.setParent(newNode);
        newNode.setParent(m_root);

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 0.f);
        EXPECT_EQ(result.yPos, 0.f);
        EXPECT_EQ(result.zPos, 0.f);
        EXPECT_EQ(result.radius, 20.f); // Expect double size
    }

    TEST_F(ASceneWithMultipleNodes, GivenANodeWithTranslationTheComputedBoundingSphereShouldAlsoBeTranslated)
    {
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_mesh1b, CreateSphere(0.f, 0.f, 0.f, 10.f));

        Node& newNode = *getScene().createNode();
        newNode.setTranslation(1.f, 2.f, 3.f);

        // Inject the new node
        m_vis1.setParent(newNode);
        newNode.setParent(m_root);

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 1.f);
        EXPECT_EQ(result.yPos, 2.f);
        EXPECT_EQ(result.zPos, 3.f);
        EXPECT_EQ(result.radius, 10.f);
    }

    TEST_F(ASceneWithMultipleNodes, GivenANodeWithTransformTheComputedBoundingSphereShouldAlsoBeTransformed) // Scale + translation
    {
        BoundingSphereCollection collection(this->getScene());
        collection.setBoundingSphere(m_mesh1b, CreateSphere(0.f, 0.f, 0.f, 10.f));

        Node& newNode = *getScene().createNode();
        newNode.setTranslation(1.f, 2.f, 3.f);
        newNode.setScaling(2.f, 2.f, 2.f);

        // Inject the new node
        m_vis1.setParent(newNode);
        newNode.setParent(m_root);

        BoundingSphere result = collection.computeBoundingSphereInWorldSpace(m_root);
        EXPECT_EQ(result.xPos, 1.f);
        EXPECT_EQ(result.yPos, 2.f);
        EXPECT_EQ(result.zPos, 3.f);
        EXPECT_EQ(result.radius, 20.f);
    }

    TEST_F(ASceneWithSingleNode, GivenACenteredCloudOfVerticesComputeTheCorrectResult)
    {
        const float radius = 5.f;

        // Build up a point cloud centered around {0,0,0}
        const float vertexData[] =
        {
            radius, 0.f, 0.f,
            -radius, 0.f, 0.f,

            0.f, radius, 0.f,
            0.f, -radius, 0.f,

            0.f, 0.f, radius,
            0.f, 0.f, -radius
        };

        const Vector3fArray& data = *this->getClient().createConstVector3fArray(6, vertexData);
        BoundingSphere result = BoundingSphereCollection::ComputeBoundingSphereForVertices(data);

        EXPECT_EQ(result.xPos, 0.f);
        EXPECT_EQ(result.yPos, 0.f);
        EXPECT_EQ(result.zPos, 0.f);
        EXPECT_EQ(result.radius, radius);
    }

    TEST_F(ASceneWithSingleNode, GivenACenteredCloudOfVerticesFromFileComputeTheCorrectResult)
    {
        const float radius = 5.f;

        // Build up a point cloud centered around {0,0,0}
        const float vertexData[] =
        {
            radius, 0.f, 0.f,
            -radius, 0.f, 0.f,

            0.f, radius, 0.f,
            0.f, -radius, 0.f,

            0.f, 0.f, radius,
            0.f, 0.f, -radius
        };

        const char* Resourcename = "myVertexArray";
        const char* ResourceDescriptionName = "myResDesc";

        // Create vertex resource and save to file
        {
            RamsesFramework tempFramework;
            RamsesClient tempClient("temp client", tempFramework);
            const Vector3fArray& tempData = *tempClient.createConstVector3fArray(6, vertexData, ramses::ResourceCacheFlag_DoNotCache, Resourcename);
            ResourceFileDescription tempResourceDesc(ResourceDescriptionName);
            tempResourceDesc.add(&tempData);
            tempClient.saveResources(tempResourceDesc, false);
        }

        // Load again
        ResourceFileDescription loadedResources(ResourceDescriptionName);
        this->getClient().loadResources(loadedResources);
        const Vector3fArray& loadedData = *RamsesUtils::TryConvert<Vector3fArray>(*this->getClient().findObjectByName(Resourcename));

        BoundingSphere result = BoundingSphereCollection::ComputeBoundingSphereForVertices(loadedData);

        EXPECT_EQ(result.xPos, 0.f);
        EXPECT_EQ(result.yPos, 0.f);
        EXPECT_EQ(result.zPos, 0.f);
        EXPECT_EQ(result.radius, radius);
    }

    TEST_F(ASceneWithSingleNode, GivenAnOffsetCloudOfVerticesComputeTheCorrectResult)
    {
        const float xOffset = 1.f;
        const float yOffset = 2.f;
        const float zOffset = 3.f;
        const float radius = 10.f;

        // Build up a point cloud centered around {1,2,3}
        const float vertexData[] =
        {
            radius + xOffset,      yOffset,              zOffset,
            -radius + xOffset,     yOffset,              zOffset,

            xOffset,               radius + yOffset,     zOffset,
            xOffset,              -radius + yOffset,     zOffset,

            xOffset,               yOffset,              radius + zOffset,
            xOffset,               yOffset,             -radius + zOffset
        };

        const Vector3fArray& data = *this->getClient().createConstVector3fArray(6, vertexData);
        BoundingSphere result = BoundingSphereCollection::ComputeBoundingSphereForVertices(data);

        EXPECT_EQ(result.xPos, xOffset);
        EXPECT_EQ(result.yPos, yOffset);
        EXPECT_EQ(result.zPos, zOffset);
        EXPECT_EQ(result.radius, radius);
    }

    TEST_F(ASceneWithSingleNode, GivenTwoVerticesLocatedDiagonallyToOriginFindTheCorrectRadius)
    {
        const float sideLength = 5.f;

        // The purpose of the test is to verify the calculated radius, given vertex positions located diagonally to the center position.
        const float vertexData[] =
        {
            sideLength, sideLength, sideLength,
            -sideLength, -sideLength, -sideLength
        };

        const Vector3fArray& data = *this->getClient().createConstVector3fArray(2, vertexData);
        BoundingSphere result = BoundingSphereCollection::ComputeBoundingSphereForVertices(data);

        EXPECT_EQ(result.xPos, 0.f);
        EXPECT_EQ(result.yPos, 0.f);
        EXPECT_EQ(result.zPos, 0.f);
        EXPECT_NEAR(result.radius, 8.7f, 0.1f); // sqrt(5^2 + 5^2 + 5^2)
    }
}
