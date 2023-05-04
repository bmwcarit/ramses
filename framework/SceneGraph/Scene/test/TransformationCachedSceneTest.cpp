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
#include "Math3d/Rotation.h"
#include "glm/gtx/transform.hpp"

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
            this->expectCorrectMatrices(nodeToCheck, glm::identity<glm::mat4>(), glm::identity<glm::mat4>());
        }

        void expectCorrectMatrices(const NodeHandle nodeToCheck, const glm::mat4& worldMatrix, const glm::mat4& objectMatrix) const
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
        this->scene.setScaling(this->transform, glm::vec3(0.5f));
        this->expectCorrectMatrices(this->nodeWithTransform, glm::scale(glm::vec3(0.5f)), glm::scale(glm::vec3(2.f)));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsTranslated)
    {
        this->scene.setTranslation(this->transform, glm::vec3(0.5f));

        this->expectCorrectMatrices(this->nodeWithTransform, glm::translate(glm::vec3{ 0.5f, 0.5f, 0.5f }), glm::translate(glm::vec3{ -0.5f, -0.5f, -0.5f }));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsRotated)
    {
        this->scene.setRotation(this->transform, glm::vec4(0.5f, 0.0f, 0.0f, 1.f), ERotationType::Euler_XYZ);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Math3d::Rotation({ 0.5f, 0.f, 0.f, 1.f }, ERotationType::Euler_XYZ),
            Math3d::Rotation({ -0.5f, 0.f, 0.f, 1.f }, ERotationType::Euler_XYZ));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenTransformOfNodeIsQuatRotated)
    {
        const auto rotation = glm::vec4(0.5f, 0.5f, -0.5f, 0.5f);
        this->scene.setRotation(this->transform, rotation, ERotationType::Quaternion);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Math3d::Rotation(rotation, ERotationType::Quaternion),
            glm::inverse(Math3d::Rotation(rotation, ERotationType::Quaternion)));
    }

    TEST_F(ATransformationCachedScene, AppliesTranslationAfterRotation)
    {
        const glm::vec3 translation(0.5f);
        const glm::vec4 rotation{ 30.f, 0.f, 0.f, 1.f };
        this->scene.setTranslation(this->transform, translation);
        this->scene.setRotation(this->transform, glm::vec4(rotation), ERotationType::Euler_ZYX);

        this->expectCorrectMatrices(this->nodeWithTransform,
            glm::translate(translation) * Math3d::Rotation(rotation, ERotationType::Euler_ZYX),
            Math3d::Rotation(-rotation, ERotationType::Euler_ZYX) * glm::translate(-translation));
    }

    TEST_F(ATransformationCachedScene, AppliesTranslationAfterScaling)
    {
        const glm::vec3 translation(0.5f);
        const glm::vec3 scaling{ 1.f, 2.f, 3.f };
        this->scene.setTranslation(this->transform, translation);
        this->scene.setScaling(this->transform, scaling);

        this->expectCorrectMatrices(this->nodeWithTransform,
            glm::translate(translation) * glm::scale(scaling),
            glm::scale(glm::vec3(1.f) / scaling) * glm::translate(-translation));
    }

    TEST_F(ATransformationCachedScene, AppliesScalingBeforeRotation)
    {
        const glm::vec4 rotation{ 30.f, 0.f, 0.f, 1.f };
        const glm::vec3 scaling{ 2.f, 3.f, 4.f };
        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_ZYX);
        this->scene.setScaling(this->transform, scaling);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Math3d::Rotation(rotation, ERotationType::Euler_ZYX) * glm::scale(scaling),
            glm::scale(glm::vec3(1.f) / scaling) * Math3d::Rotation(-rotation, ERotationType::Euler_ZYX));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenRotationConventionIsSet)
    {
        const glm::vec4 rotation{ 5.f, 10.0f, 150.0f, 1.f };
        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XYZ);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Math3d::Rotation(rotation, ERotationType::Euler_XYZ),
            Math3d::Rotation(-rotation, ERotationType::Euler_ZYX));

        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XZY);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Math3d::Rotation(rotation, ERotationType::Euler_XZY),
            Math3d::Rotation(-rotation, ERotationType::Euler_YZX));

        const glm::vec4 rotationQuat = glm::normalize(glm::vec4{0.2f, 0.3f, 0.5f, 0.6f});
        this->scene.setRotation(this->transform, rotationQuat, ERotationType::Quaternion);

        this->expectCorrectMatrices(this->nodeWithTransform,
            Math3d::Rotation(rotationQuat, ERotationType::Quaternion),
            glm::inverse(Math3d::Rotation(rotationQuat, ERotationType::Quaternion)));
    }

    TEST_F(ATransformationCachedScene, GivesCorrectValueWhenRotationConventionIsSetDifferentFromParent)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const glm::vec3 rotationParent{ 60.f, 0.f, 0.f }; //rotate only in X-Axis
        this->scene.setRotation(this->transform, glm::vec4(rotationParent, 1.f), ERotationType::Euler_ZXY);

        const glm::vec3 rotationChild{ 0.f, 30.f, 0.f }; //rotate only in Y-Axis
        this->scene.setRotation(childTransform, glm::vec4(rotationChild, 1.f), ERotationType::Euler_XYZ); //use different rotationType

        const auto expectedChildRotation = glm::vec4(rotationChild + rotationParent, 1.f); //because both rotate only on 1 axis this is still possible
        const auto expectedWorldMatrix = Math3d::Rotation(expectedChildRotation, ERotationType::Euler_ZYX); //Child rotate around Y, then parent rotate around X
        const auto expectedObjectMatrix = Math3d::Rotation(-expectedChildRotation, ERotationType::Euler_XYZ); //invert angle AND rotationType
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, glm::vec4(rotationParent, 1.f), ERotationType::Euler_ZYX);
    }

    TEST_F(ATransformationCachedScene, QuatParentEulerChild)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const glm::vec4 rotationParent{0.5f, 0.f, 0.f, 0.8660254f}; // rotate only in X-Axis with 60 degrees
        this->scene.setRotation(this->transform, rotationParent, ERotationType::Quaternion);

        const glm::vec3 rotationChild{ 0.f, 30.f, 0.f }; //rotate only in Y-Axis
        this->scene.setRotation(childTransform, glm::vec4(rotationChild, 1.f), ERotationType::Euler_XYZ); //use different rotationType

        const auto expectedChildRotation = glm::vec4(rotationChild + glm::vec3(60.f, 0.f, 0.f), 1.f); //because both rotate only on 1 axis this is still possible
        const auto expectedWorldMatrix = Math3d::Rotation(expectedChildRotation, ERotationType::Euler_ZYX); //Child rotate around Y, then parent rotate around X
        const auto expectedObjectMatrix = Math3d::Rotation(-expectedChildRotation, ERotationType::Euler_XYZ); //invert angle AND rotationType
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

        const glm::vec3 rotationParent{ 60.f, 0.f, 0.f }; //rotate only in X-Axis
        this->scene.setRotation(this->transform, glm::vec4(rotationParent, 1.f), ERotationType::Euler_ZXY);

        const glm::vec4 rotationChild{0.f, 0.3826834f, 0.f, 0.9238795f}; // rotate only in Y-Axis with 45 degrees
        this->scene.setRotation(childTransform, rotationChild, ERotationType::Quaternion);

        const auto expectedChildRotation = glm::vec4(glm::vec3(0.f, 45.f, 0.f) + rotationParent, 1.f); //because both rotate only on 1 axis this is still possible
        const auto expectedWorldMatrix = Math3d::Rotation(expectedChildRotation, ERotationType::Euler_ZYX); //Child rotate around Y, then parent rotate around X
        const auto expectedObjectMatrix = Math3d::Rotation(-expectedChildRotation, ERotationType::Euler_XYZ); //invert angle AND rotationType
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, glm::vec4(rotationParent, 1.f), ERotationType::Euler_ZYX);
    }

    TEST_F(ATransformationCachedScene, QuatParentQuatChild)
    {
        // add parent with transform
        const NodeHandle childNode = this->scene.allocateNode();
        const TransformHandle childTransform = this->scene.allocateTransform(childNode);
        this->scene.addChildToNode(this->nodeWithTransform, childNode);

        this->expectIdentityMatrices(nodeWithoutTransform);
        this->expectIdentityMatrices(childNode);

        const glm::vec4 rotationParent{0.5f, 0.f, 0.f, 0.8660254f}; // rotate only in X-Axis with 60 degrees
        this->scene.setRotation(this->transform, rotationParent, ERotationType::Quaternion);

        const glm::vec4 rotationChild{0.f, 0.3826834f, 0.f, 0.9238795f}; // rotate only in Y-Axis with 45 degrees
        this->scene.setRotation(childTransform, rotationChild, ERotationType::Quaternion);

        // quaternion multiplication - remove if glm is available
        auto qmul = [](const glm::vec4& p, const glm::vec4& q) {
            glm::vec4 out;
            out.w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
            out.x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
            out.y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
            out.z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
            return out;
        };

        const auto      expectedChildRotation = qmul(rotationParent, rotationChild);
        const auto expectedWorldMatrix = Math3d::Rotation(expectedChildRotation, ERotationType::Quaternion);
        const auto expectedObjectMatrix = glm::inverse(Math3d::Rotation( expectedChildRotation, ERotationType::Quaternion));
        this->expectCorrectMatrices(childNode, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, glm::vec4(rotationParent), ERotationType::Euler_ZYX);
    }

    TEST_F(ATransformationCachedScene, UpdatesMatrixCacheAfterAddingParentWithTransform)
    {
        // add parent with transform
        const NodeHandle parent = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        this->scene.setScaling(parentTransform, glm::vec3(0.5f));

        this->expectIdentityMatrices(this->nodeWithTransform);

        this->scene.addChildToNode(parent, this->nodeWithTransform);

        this->expectCorrectMatrices(this->nodeWithTransform, glm::scale(glm::vec3(0.5f)), glm::scale(glm::vec3(2.f)));
    }

    TEST_F(ATransformationCachedScene, UpdatesMatrixCacheAfterChangingTransformOfParent)
    {
        // add parent with transform
        const NodeHandle parent = this->scene.allocateNode();
        const TransformHandle parentTransform = this->scene.allocateTransform(parent);
        this->scene.setScaling(parentTransform, glm::vec3(0.5f));
        this->scene.addChildToNode(parent, this->nodeWithTransform);

        this->expectCorrectMatrices(this->nodeWithTransform, glm::scale(glm::vec3(0.5f)), glm::scale(glm::vec3(2.f)));

        this->scene.setScaling(parentTransform, glm::vec3(0.25f));

        this->expectCorrectMatrices(this->nodeWithTransform, glm::scale(glm::vec3(0.25f)), glm::scale(glm::vec3(4.f)));
    }

    TEST_F(ATransformationCachedScene, ReturnsIdentityMatrixForUntransformedNodeAfterRemovingParentWithTransform)
    {
        this->scene.setScaling(this->transform, glm::vec3(0.25f));
        this->scene.addChildToNode(this->nodeWithTransform, this->nodeWithoutTransform);

        this->expectCorrectMatrices(this->nodeWithoutTransform, glm::scale(glm::vec3(0.25f)), glm::scale(glm::vec3(4.f)));

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

        this->scene.setTranslation(this->transform, glm::vec3(0.1f, 0.2f, 1));

        const auto expectedWorldMatrix = glm::translate(glm::vec3(0.1f, 0.2f, 1));
        const auto expectedObjectMatrix = glm::translate(glm::vec3(-0.1f, -0.2f, -1));
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
        const glm::vec4 rotation{ 60.f, 30.f, 30.f, 1.f };
        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XYZ);

        const auto expectedWorldMatrix = Math3d::Rotation(rotation, ERotationType::Euler_XYZ);
        const auto expectedObjectMatrix = Math3d::Rotation(-rotation, ERotationType::Euler_ZYX); //invert input angles AND rotationType
        this->expectCorrectMatrices(nodeWithTransform, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childLeft, expectedWorldMatrix, expectedObjectMatrix);
        this->expectCorrectMatrices(childRight, expectedWorldMatrix, expectedObjectMatrix);

        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_ZYX);

        const auto expectedWorldMatrixUpdated = Math3d::Rotation(rotation, ERotationType::Euler_ZYX);
        const auto expectedObjectMatrixUpdated = Math3d::Rotation(-rotation, ERotationType::Euler_XYZ); //invert input angles AND rotationType
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

        const glm::vec3 leftTranslation(0.1f, 0.2f, 0.3f);
        const glm::vec3 rightTranslation(0.4f, 0.5f, 0.6f);
        glm::vec3 parentTranslation(0.7f, 0.8f, 0.9f);

        this->scene.setTranslation(childLeftTransform, leftTranslation);
        this->scene.setTranslation(childRightTransform, rightTranslation);
        this->scene.setTranslation(this->transform, parentTranslation);

        auto expectedWorldMatrixLeft = glm::translate(parentTranslation + leftTranslation);
        auto expectedWorldMatrixRight = glm::translate(parentTranslation + rightTranslation);
        auto expectedWorldMatrixParent = glm::translate(parentTranslation);

        auto expectedObjectMatrixLeft = glm::translate(-parentTranslation - leftTranslation);
        auto expectedObjectMatrixRight = glm::translate(-parentTranslation - rightTranslation);
        auto expectedObjectMatrixParent = glm::translate(-parentTranslation);

        // expect that transformation matrices of all 3 transforms are correct
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixLeft, expectedObjectMatrixLeft);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixRight, expectedObjectMatrixRight);
        this->expectCorrectMatrices(this->nodeWithTransform, expectedWorldMatrixParent, expectedObjectMatrixParent);

        // modify parent translation
        parentTranslation.y = 99.f;
        this->scene.setTranslation(this->transform, parentTranslation);

        expectedWorldMatrixLeft = glm::translate(parentTranslation + leftTranslation);
        expectedWorldMatrixRight = glm::translate(parentTranslation + rightTranslation);
        expectedWorldMatrixParent = glm::translate(parentTranslation);

        expectedObjectMatrixLeft = glm::translate(-parentTranslation - leftTranslation);
        expectedObjectMatrixRight = glm::translate(-parentTranslation - rightTranslation);
        expectedObjectMatrixParent = glm::translate(-parentTranslation);

        // expect that parent translation modification is reflected to all dependent transformation matrices
        this->expectCorrectMatrices(childLeft, expectedWorldMatrixLeft, expectedObjectMatrixLeft);
        this->expectCorrectMatrices(childRight, expectedWorldMatrixRight, expectedObjectMatrixRight);
        this->expectCorrectMatrices(this->nodeWithTransform, expectedWorldMatrixParent, expectedObjectMatrixParent);
    }

    TEST_F(ATransformationCachedScene, DoesNotAffectNodeWithoutTransformAfterAddingChildWithTransform)
    {
        this->scene.addChildToNode(this->nodeWithoutTransform, this->nodeWithTransform);
        this->scene.setTranslation(this->transform, glm::vec3(1, 2, 3));

        this->expectIdentityMatrices(this->nodeWithoutTransform);
    }

    TEST_F(ATransformationCachedScene, UpdatesNodeMatricesWhenItHasAParentWithTransform)
    {
        this->scene.addChildToNode(this->nodeWithTransform, this->nodeWithoutTransform);

        const glm::vec3 translation(1, 2, 3);
        const glm::vec4 rotation(4, 5, 6, 1);
        const glm::vec3 scaling(7, 8, 9);

        this->scene.setRotation(this->transform, rotation, ERotationType::Euler_XYZ);
        this->scene.setTranslation(this->transform, translation);
        this->scene.setScaling(this->transform, scaling);

        const auto expectedWorldMatrix =
            glm::translate(translation) *
            Math3d::Rotation(rotation, ERotationType::Euler_XYZ)*
            glm::scale(scaling);

        const auto expectedObjectMatrix =
            glm::scale(1.f / scaling) *
            glm::transpose(Math3d::Rotation(rotation, ERotationType::Euler_XYZ)) *
            glm::translate(-translation);

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

        glm::vec3 parentTranslation(1, 2, 3);
        glm::vec3 childTranslation(3, 4, 5);

        this->scene.setTranslation(parentTransform, parentTranslation);
        this->scene.setTranslation(childTransform, childTranslation);

        auto expectedParentWorldMatrix =
            glm::translate(parentTranslation);
        auto expectedChildWorldMatrix =
            glm::translate(childTranslation);

        // force cache update
        expectMatrixFloatEqual(expectedParentWorldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, parent));
        expectMatrixFloatEqual(expectedChildWorldMatrix, this->scene.updateMatrixCache(ETransformationMatrixType_World, child));

        this->scene.addChildToNode(parent, child);

        auto expectedUpdatedChildWorldMatrix =
            glm::translate(childTranslation + parentTranslation);
        auto expectedUpdatedChildObjectMatrix =
            glm::translate(-childTranslation - parentTranslation);

        this->expectCorrectMatrices(child, expectedUpdatedChildWorldMatrix, expectedUpdatedChildObjectMatrix);
    }
}
