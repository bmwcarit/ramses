//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "Scene/TransformationCachedScene.h"
#include "Scene/Scene.h"
#include "TestEqualHelper.h"

using namespace testing;

namespace ramses_internal
{
    class ATransformationCachedScene : public testing::Test
    {
    public:
        ATransformationCachedScene()
            : scene(SceneInfo())
        {
            this->nodeWithoutTransform = this->scene.allocateNode();
            this->nodeWithTransform = this->scene.allocateNode();
            this->transform = this->scene.allocateTransform(this->nodeWithTransform);
        }

    protected:
        void expectIdentityMatrices(const NodeHandle nodeToCheck) const
        {
            this->expectCorrectMatrices(nodeToCheck, Matrix44f::Identity, Matrix44f::Identity);
        }

        void expectCorrectMatrices(const NodeHandle nodeToCheck, const Matrix44f& worldMatrix, const Matrix44f& objectMatrix) const
        {
            expectMatrixFloatEqual(worldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, nodeToCheck));
            expectMatrixFloatEqual(objectMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_Object, nodeToCheck));

            // should also work when state is clean (after first update)
            expectMatrixFloatEqual(worldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, nodeToCheck));
            expectMatrixFloatEqual(objectMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_Object, nodeToCheck));
        }

        TransformationCachedScene scene;
        NodeHandle nodeWithTransform;
        NodeHandle nodeWithoutTransform;
        TransformHandle transform;
    };

    TEST_F(ATransformationCachedScene, GivesIdentityMatricesForNodesWithoutTransform)
    {
        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, GivesIdentityMatricesForInvalidNodes)
    {
        const NodeHandle invalidNode = NodeHandle::Invalid();
        this->expectIdentityMatrices(invalidNode);
    }

    TEST_F(ATransformationCachedScene, GivesIdentityMatricesForNodeWithEmptyTransform)
    {
        this->expectIdentityMatrices(this->nodeWithTransform);
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsScaled)
    {
        this->scene.setScaling(this->transform, Vector3(0.5f));
        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.5f), Matrix44f::Scaling(2.f));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsTranslated)
    {
        this->scene.setTranslation(this->transform, Vector3(0.5f));

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Translation({ 0.5f, 0.5f, 0.5f }), Matrix44f::Translation({ -0.5f, -0.5f, -0.5f }));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsRotated)
    {
        this->scene.setRotation(this->transform, Vector4(0.5f, 0.0f, 0.0f, 1.f), ERotationType::Euler_XYZ);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Rotation({ 0.5f, 0.f, 0.f, 1.f }, ERotationType::Euler_XYZ),
            Matrix44f::Rotation({ -0.5f, 0.f, 0.f, 1.f }, ERotationType::Euler_XYZ));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsQuatRotated)
    {
        const auto rotation = Vector4(0.5f, 0.5f, -0.5f, 0.5f);
        this->scene.setRotation(this->transform, rotation, ERotationType::Quaternion);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Rotation(rotation, ERotationType::Quaternion),
            Matrix44f::Rotation(rotation, ERotationType::Quaternion).inverse());
    }

    TEST_F(ATransformationCachedScene, AppliesTranslationAfterRotation)
    {
        const Vector3 translation(0.5f);
        const Vector4 rotation{ 30.f, 0.f, 0.f, 1.f };
        this->scene.setTranslation(this->transform, translation);
        this->scene.setRotation(this->transform, Vector4(rotation), ERotationType::Euler_ZYX);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Translation(translation) * Matrix44f::Rotation(rotation, ERotationType::Euler_ZYX),
            Matrix44f::Rotation(-1 * rotation, ERotationType::Euler_ZYX) * Matrix44f::Translation(-translation));
    }

    TEST_F(ATransformationCachedScene, AppliesTranslationAfterScaling)
    {
        const Vector3 translation(0.5f);
        const Vector3 scaling{ 1.f, 2.f, 3.f };
        this->scene.setTranslation(this->transform, translation);
        this->scene.setScaling(this->transform, scaling);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Translation(translation) * Matrix44f::Scaling(scaling),
            Matrix44f::Scaling(scaling.inverse()) * Matrix44f::Translation(-translation));
    }

    TEST_F(ATransformationCachedScene, AppliesScalingBeforeRotation)
    {
        const Vector4 rotation{ 30.f, 0.f, 0.f, 1.f };
        const Vector3 scaling{ 2.f, 3.f, 4.f };
        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_ZYX);
        this->scene.setScaling(this->transform, scaling);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Rotation(rotation, ERotationType::Euler_ZYX) * Matrix44f::Scaling(scaling),
            Matrix44f::Scaling(scaling.inverse()) * Matrix44f::Rotation(-1 * rotation, ERotationType::Euler_ZYX));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenRotationConventionIsSet)
    {
        const Vector4 rotation{ 5.f, 10.0f, 150.0f, 1.f };
        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XYZ);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Rotation(rotation, ERotationType::Euler_XYZ),
            Matrix44f::Rotation(-1 * rotation, ERotationType::Euler_ZYX));

        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XZY);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Rotation(rotation, ERotationType::Euler_XZY),
            Matrix44f::Rotation(-1 * rotation, ERotationType::Euler_YZX));

        Vector4 rotationQuat{0.2f, 0.3f, 0.5f, 0.6f};
        rotationQuat /= rotationQuat.length(); // normalize
        this->scene.setRotation(this->transform, rotationQuat, ERotationType::Quaternion);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Matrix44f::Rotation(rotationQuat, ERotationType::Quaternion),
            Matrix44f::Rotation(rotationQuat, ERotationType::Quaternion).inverse());
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenRotationConventionIsSetDifferentFromParent)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const Vector3 rotationParent{ 60.f, 0.f, 0.f }; //rotate only in X-Axis
        this->scene.setRotation(this->transform, Vector4(rotationParent), ERotationType::Euler_ZXY);

        const Vector3 rotationChild{ 0.f, 30.f, 0.f }; //rotate only in Y-Axis
        this->scene.setRotation(childTransform, Vector4(rotationChild), ERotationType::Euler_XYZ); //use different rotationType

        const auto expectedChildRotation = Vector4(rotationChild + rotationParent); //because both rotate only on 1 axis this is still possible
        const Matrix44f expectedWorldMatrix = Matrix44f::Rotation(expectedChildRotation, ERotationType::Euler_ZYX); //Child rotate around Y, then parent rotate around X
        const Matrix44f expectedObjectMatrix = Matrix44f::Rotation(-1 * expectedChildRotation, ERotationType::Euler_XYZ); //invert angle AND rotationType
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, Vector4(rotationParent), ERotationType::Euler_ZYX);
    }

    TEST_F(ATransformationCachedScene, QuatParentEulerChild)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const Vector4 rotationParent{0.5f, 0.f, 0.f, 0.8660254f}; // rotate only in X-Axis with 60 degrees
        this->scene.setRotation(this->transform, rotationParent, ERotationType::Quaternion);

        const Vector3 rotationChild{ 0.f, 30.f, 0.f }; //rotate only in Y-Axis
        this->scene.setRotation(childTransform, Vector4(rotationChild), ERotationType::Euler_XYZ); //use different rotationType

        const auto expectedChildRotation = Vector4(rotationChild + Vector3(60.f, 0.f, 0.f)); //because both rotate only on 1 axis this is still possible
        const Matrix44f expectedWorldMatrix = Matrix44f::Rotation(expectedChildRotation, ERotationType::Euler_ZYX); //Child rotate around Y, then parent rotate around X
        const Matrix44f expectedObjectMatrix = Matrix44f::Rotation(-1 * expectedChildRotation, ERotationType::Euler_XYZ); //invert angle AND rotationType
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, rotationParent, ERotationType::Euler_ZYX);
    }

    TEST_F(ATransformationCachedScene, EulerParentQuatChild)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const Vector3 rotationParent{ 60.f, 0.f, 0.f }; //rotate only in X-Axis
        this->scene.setRotation(this->transform, Vector4(rotationParent), ERotationType::Euler_ZXY);

        const Vector4 rotationChild{0.f, 0.3826834f, 0.f, 0.9238795f}; // rotate only in Y-Axis with 45 degrees
        this->scene.setRotation(childTransform, rotationChild, ERotationType::Quaternion);

        const auto expectedChildRotation = Vector4(Vector3(0.f, 45.f, 0.f) + rotationParent); //because both rotate only on 1 axis this is still possible
        const Matrix44f expectedWorldMatrix = Matrix44f::Rotation(expectedChildRotation, ERotationType::Euler_ZYX); //Child rotate around Y, then parent rotate around X
        const Matrix44f expectedObjectMatrix = Matrix44f::Rotation(-1 * expectedChildRotation, ERotationType::Euler_XYZ); //invert angle AND rotationType
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, Vector4(rotationParent), ERotationType::Euler_ZYX);
    }

    TEST_F(ATransformationCachedScene, QuatParentQuatChild)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const Vector4 rotationParent{0.5f, 0.f, 0.f, 0.8660254f}; // rotate only in X-Axis with 60 degrees
        this->scene.setRotation(this->transform, rotationParent, ERotationType::Quaternion);

        const Vector4 rotationChild{0.f, 0.3826834f, 0.f, 0.9238795f}; // rotate only in Y-Axis with 45 degrees
        this->scene.setRotation(childTransform, rotationChild, ERotationType::Quaternion);

        // quaternion multiplication - remove if glm is available
        auto qmul = [](const Vector4& p, const Vector4& q) {
            Vector4 out;
            out.w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
            out.x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
            out.y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
            out.z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
            return out;
        };

        const auto      expectedChildRotation = qmul(rotationParent, rotationChild);
        const Matrix44f expectedWorldMatrix = Matrix44f::Rotation(expectedChildRotation, ERotationType::Quaternion);
        const Matrix44f expectedObjectMatrix = Matrix44f::Rotation( expectedChildRotation, ERotationType::Quaternion).inverse();
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, Vector4(rotationParent), ERotationType::Euler_ZYX);
    }

    TEST_F(ATransformationCachedScene, UpdatesMatrixCacheAfterAddingParentWithTransform)
    {
        // add parent with transform
        const NodeHandle parent = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        this->scene.setScaling(parentTransform, Vector3(0.5f));

        this->expectIdentityMatrices(this->nodeWithTransform);

        this->scene.addChildToNode(parent, this->nodeWithTransform);

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.5f), Matrix44f::Scaling(2.f));
    }

    TEST_F(ATransformationCachedScene, UpdatesMatrixCacheAfterChangingTransformOfParent)
    {
        // add parent with transform
        const NodeHandle parent = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        this->scene.setScaling(parentTransform, Vector3(0.5f));
        this->scene.addChildToNode(parent, this->nodeWithTransform);

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.5f), Matrix44f::Scaling(2.f));

        this->scene.setScaling(parentTransform, Vector3(0.25f));

        this->expectCorrectMatrices(this->nodeWithTransform, Matrix44f::Scaling(0.25f), Matrix44f::Scaling(4.f));
    }

    TEST_F(ATransformationCachedScene, ReturnsIdentityMatrixForUntransformedNodeAfterRemovingParentWithTransform)
    {
        this->scene.setScaling(this->transform, Vector3(0.25f));
        this->scene.addChildToNode(this->nodeWithTransform, this->nodeWithoutTransform);

        this->expectCorrectMatrices(this->nodeWithoutTransform, Matrix44f::Scaling(0.25f), Matrix44f::Scaling(4.f));

        this->scene.removeChildFromNode(this->nodeWithTransform, this->nodeWithoutTransform);

        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, PropagatesMatricesToMultipleChildNodes)
    {
        // add parent with transform
        const NodeHandle childLeft = this->scene.allocateNode();
        const NodeHandle childRight = this->scene.allocateNode();
        this->scene.addChildToNode(this->nodeWithTransform, childLeft);
        this->scene.addChildToNode(this->nodeWithTransform, childRight);

        this->expectIdentityMatrices(childLeft);
        this->expectIdentityMatrices(childRight);

        this->scene.setTranslation(this->transform, Vector3(0.1f, 0.2f, 1));

        const Matrix44f expectedWorldMatrix = Matrix44f::Translation(Vector3(0.1f, 0.2f, 1));
        const Matrix44f expectedObjectMatrix = Matrix44f::Translation(Vector3(-0.1f, -0.2f, -1));
        this->expectCorrectMatrices(childLeft, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childRight, expectedWorldMatrix, expectedObjectMatrix);
    }

    TEST_F(ATransformationCachedScene, PropagatesMatricesToMultipleChildNodeWhenRotationConventionIsSet)
    {
        // add parent with transform
        const NodeHandle childLeft = this->scene.allocateNode();
        const NodeHandle childRight = this->scene.allocateNode();
        this->scene.addChildToNode(this->nodeWithTransform, childLeft);
        this->scene.addChildToNode(this->nodeWithTransform, childRight);

        this->expectIdentityMatrices(nodeWithoutTransform);
        const Vector4 rotation{ 60.f, 30.f, 30.f, 1.f };
        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XYZ);

        const Matrix44f expectedWorldMatrix = Matrix44f::Rotation(rotation, ERotationType::Euler_XYZ);
        const Matrix44f expectedObjectMatrix = Matrix44f::Rotation(-1 * rotation, ERotationType::Euler_ZYX); //invert input angles AND rotationType
        this->expectCorrectMatrices(nodeWithTransform, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childLeft, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childRight, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_ZYX);

        const Matrix44f expectedWorldMatrixUpdated = Matrix44f::Rotation(rotation, ERotationType::Euler_ZYX);
        const Matrix44f expectedObjectMatrixUpdated = Matrix44f::Rotation(-1 * rotation, ERotationType::Euler_XYZ); //invert input angles AND rotationType
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixUpdated, expectedObjectMatrixUpdated);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixUpdated, expectedObjectMatrixUpdated);
    }

    TEST_F(ATransformationCachedScene, confidenceTest_UpdatesMatricesOfMultipleChildNodesWithTransform)
    {
        // create a small transformation graph - parent and 2 children, all transforms
        const NodeHandle childLeft = this->scene.allocateNode();
        const NodeHandle childRight = this->scene.allocateNode();
        this->scene.addChildToNode(this->nodeWithTransform, childLeft);
        this->scene.addChildToNode(this->nodeWithTransform, childRight);

        TransformHandle childLeftTransform = this->scene.allocateTransform(childLeft);
        TransformHandle childRightTransform = this->scene.allocateTransform(childRight);

        const Vector3 leftTranslation(0.1f, 0.2f, 0.3f);
        const Vector3 rightTranslation(0.4f, 0.5f, 0.6f);
        Vector3 parentTranslation(0.7f, 0.8f, 0.9f);

        this->scene.setTranslation(childLeftTransform, leftTranslation);
        this->scene.setTranslation(childRightTransform, rightTranslation);
        this->scene.setTranslation(this->transform, parentTranslation);

        Matrix44f expectedWorldMatrixLeft = Matrix44f::Translation(parentTranslation + leftTranslation);
        Matrix44f expectedWorldMatrixRight = Matrix44f::Translation(parentTranslation + rightTranslation);
        Matrix44f expectedWorldMatrixParent = Matrix44f::Translation(parentTranslation);

        Matrix44f expectedObjectMatrixLeft = Matrix44f::Translation(-parentTranslation - leftTranslation);
        Matrix44f expectedObjectMatrixRight = Matrix44f::Translation(-parentTranslation - rightTranslation);
        Matrix44f expectedObjectMatrixParent = Matrix44f::Translation(-parentTranslation);

        // expect that transformation matrices of all 3 transforms are correct
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixLeft, expectedObjectMatrixLeft);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixRight, expectedObjectMatrixRight);
        this->expectCorrectMatrices(this->nodeWithTransform, expectedWorldMatrixParent, expectedObjectMatrixParent);

        // modify parent translation
        parentTranslation.y = 99.f;
        this->scene.setTranslation(this->transform, parentTranslation);

        expectedWorldMatrixLeft = Matrix44f::Translation(parentTranslation + leftTranslation);
        expectedWorldMatrixRight = Matrix44f::Translation(parentTranslation + rightTranslation);
        expectedWorldMatrixParent = Matrix44f::Translation(parentTranslation);

        expectedObjectMatrixLeft = Matrix44f::Translation(-parentTranslation - leftTranslation);
        expectedObjectMatrixRight = Matrix44f::Translation(-parentTranslation - rightTranslation);
        expectedObjectMatrixParent = Matrix44f::Translation(-parentTranslation);

        // expect that parent translation modification is reflected to all dependent transformation matrices
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixLeft, expectedObjectMatrixLeft);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixRight, expectedObjectMatrixRight);
        this->expectCorrectMatrices(this->nodeWithTransform, expectedWorldMatrixParent, expectedObjectMatrixParent);
    }

    TEST_F(ATransformationCachedScene, DoesNotAffectNodeWithoutTransformAfterAddingChildWithTransform)
    {
        this->scene.addChildToNode(this->nodeWithoutTransform, this->nodeWithTransform);
        this->scene.setTranslation(this->transform, Vector3(1, 2, 3));

        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, UpdatesNodeMatricesWhenItHasAParentWithTransform)
    {
        this->scene.addChildToNode(this->nodeWithTransform, this->nodeWithoutTransform);

        const Vector3 translation(1, 2, 3);
        const Vector4 rotation(4, 5, 6, 1);
        const Vector3 scaling(7, 8, 9);

        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XYZ);
        this->scene.setTranslation(this->transform, translation);
        this->scene.setScaling(this->transform, scaling);

        const Matrix44f expectedWorldMatrix =
            Matrix44f::Translation(translation) *
            Matrix44f::Rotation(rotation, ERotationType::Euler_XYZ)*
            Matrix44f::Scaling(scaling);

        const Matrix44f expectedObjectMatrix =
            Matrix44f::Scaling(scaling.inverse()) *
            Matrix44f::Rotation(rotation, ERotationType::Euler_XYZ).transpose() *
            Matrix44f::Translation(-translation);

        this->expectCorrectMatrices(this->nodeWithoutTransform, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.removeChildFromNode(this->nodeWithTransform, this->nodeWithoutTransform);

        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, UpdatesMatricesWhenAddingNodeAsChildOfOtherNode)
    {
        const NodeHandle parent = this->scene.allocateNode();
        const NodeHandle child = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        const TransformHandle childTransform = this->scene.allocateTransform(child);

        Vector3 parentTranslation(1, 2, 3);
        Vector3 childTranslation(3, 4, 5);

        this->scene.setTranslation(parentTransform, parentTranslation);
        this->scene.setTranslation(childTransform, childTranslation);

        Matrix44f expectedParentWorldMatrix =
            Matrix44f::Translation(parentTranslation);
        Matrix44f expectedChildWorldMatrix =
            Matrix44f::Translation(childTranslation);

        // force cache update
        expectMatrixFloatEqual(expectedParentWorldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, parent));
        expectMatrixFloatEqual(expectedChildWorldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, child));

        this->scene.addChildToNode(parent, child);

        Matrix44f expectedUpdatedChildWorldMatrix =
            Matrix44f::Translation(childTranslation + parentTranslation);
        Matrix44f expectedUpdatedChildObjectMatrix =
            Matrix44f::Translation(-childTranslation - parentTranslation);

        this->expectCorrectMatrices(child, expectedUpdatedChildWorldMatrix, expectedUpdatedChildObjectMatrix);
    }
}
