//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "internals/RamsesObjectResolver.h"
#include "internals/ErrorReporting.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "RamsesTestUtils.h"

#include "ramses-client-api/Node.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/RenderPass.h"

namespace rlogic::internal
{
    class ARamsesObjectResolver : public ::testing::Test
    {
    protected:
        RamsesTestSetup m_ramsesTestSetup;
        ramses::Scene* m_scene { m_ramsesTestSetup.createScene() };
        ErrorReporting m_errors;
        RamsesObjectResolver m_resolver {m_errors, *m_scene};
    };

    TEST_F(ARamsesObjectResolver, FindsSceneNodeByItsId)
    {
        ramses::Node* node = m_scene->createNode();
        EXPECT_EQ(node, m_resolver.findRamsesNodeInScene("some logic node", node->getSceneObjectId()));
        EXPECT_TRUE(m_errors.getErrors().empty());
    }

    TEST_F(ARamsesObjectResolver, FindsAppearanceByItsId)
    {
        ramses::Appearance& appearance = RamsesTestSetup::CreateTrivialTestAppearance(*m_scene);
        EXPECT_EQ(&appearance, m_resolver.findRamsesAppearanceInScene("some logic node", appearance.getSceneObjectId()));
        EXPECT_TRUE(m_errors.getErrors().empty());
    }

    TEST_F(ARamsesObjectResolver, FindsCameraByItsId)
    {
        ramses::Camera* camera = m_scene->createPerspectiveCamera();
        EXPECT_EQ(camera, m_resolver.findRamsesCameraInScene("some logic node", camera->getSceneObjectId()));
        EXPECT_TRUE(m_errors.getErrors().empty());
    }

    TEST_F(ARamsesObjectResolver, FindsRenderPassByItsId)
    {
        const ramses::RenderPass* rp = m_scene->createRenderPass();
        EXPECT_EQ(rp, m_resolver.findRamsesRenderPassInScene("some logic node", rp->getSceneObjectId()));
        EXPECT_TRUE(m_errors.getErrors().empty());
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenNodeWithGivenIdDoesNotExist)
    {
        ramses::sceneObjectId_t fakeNodeId{ 42 };

        EXPECT_FALSE(m_resolver.findRamsesNodeInScene("some logic node", fakeNodeId));
        EXPECT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Serialized Ramses Logic object 'some logic node' points to a Ramses object (id: 42) which couldn't be found in the provided scene!", m_errors.getErrors()[0].message);
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenAppearanceWithGivenIdDoesNotExist)
    {
        ramses::sceneObjectId_t fakeAppearanceId{ 42 };

        EXPECT_FALSE(m_resolver.findRamsesAppearanceInScene("some logic node", fakeAppearanceId));
        EXPECT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Serialized Ramses Logic object 'some logic node' points to a Ramses object (id: 42) which couldn't be found in the provided scene!", m_errors.getErrors()[0].message);
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenCameraWithGivenIdDoesNotExist)
    {
        ramses::sceneObjectId_t fakeCameraId{ 42 };

        EXPECT_FALSE(m_resolver.findRamsesCameraInScene("some logic node", fakeCameraId));
        EXPECT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Serialized Ramses Logic object 'some logic node' points to a Ramses object (id: 42) which couldn't be found in the provided scene!", m_errors.getErrors()[0].message);
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenRenderPassWithGivenIdDoesNotExist)
    {
        ramses::sceneObjectId_t fakeRenderPassId{ 42 };

        EXPECT_FALSE(m_resolver.findRamsesRenderPassInScene("some logic node", fakeRenderPassId));
        EXPECT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Serialized Ramses Logic object 'some logic node' points to a Ramses object (id: 42) which couldn't be found in the provided scene!", m_errors.getErrors()[0].message);
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenObjectWithGivenIdExists_ButIsNotANode)
    {
        ramses::Appearance& appearance = RamsesTestSetup::CreateTrivialTestAppearance(*m_scene);

        EXPECT_FALSE(m_resolver.findRamsesNodeInScene("some logic node", appearance.getSceneObjectId()));
        ASSERT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Ramses binding 'some logic node' points to a Ramses scene object (id: 2) which is not of the same type!", m_errors.getErrors()[0].message);
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenResolvedObjectExists_ButIsNotCamera)
    {
        ramses::Node* node = m_scene->createNode();

        EXPECT_FALSE(m_resolver.findRamsesCameraInScene("some logic node", node->getSceneObjectId()));
        ASSERT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Ramses binding 'some logic node' points to a Ramses scene object (id: 1) which is not of the same type!", m_errors.getErrors()[0].message);
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenResolvedObjectExists_ButIsNotAppearance)
    {
        ramses::Node* node = m_scene->createNode();

        EXPECT_FALSE(m_resolver.findRamsesAppearanceInScene("some logic node", node->getSceneObjectId()));
        ASSERT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Ramses binding 'some logic node' points to a Ramses scene object (id: 1) which is not of the same type!", m_errors.getErrors()[0].message);
    }

    TEST_F(ARamsesObjectResolver, ReportsErrorWhenResolvedObjectExists_ButIsNotRenderPass)
    {
        ramses::Node* node = m_scene->createNode();

        EXPECT_FALSE(m_resolver.findRamsesRenderPassInScene("some logic node", node->getSceneObjectId()));
        ASSERT_EQ(1u, m_errors.getErrors().size());
        EXPECT_EQ("Fatal error during loading from file! Ramses binding 'some logic node' points to a Ramses scene object (id: 1) which is not of the same type!", m_errors.getErrors()[0].message);
    }

    // Special case (ramses Camera is also a Node) - test that it works as expected when resolved by Id
    TEST_F(ARamsesObjectResolver, ResolvesRamsesCamera_WhenUsedAsIfItWasOnlyNode)
    {
        ramses::Camera* camera = m_scene->createPerspectiveCamera();

        EXPECT_EQ(camera, m_resolver.findRamsesNodeInScene("some logic node", camera->getSceneObjectId()));
        EXPECT_TRUE(m_errors.getErrors().empty());
    }
}
