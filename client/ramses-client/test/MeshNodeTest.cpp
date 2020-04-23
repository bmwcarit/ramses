//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/UInt16Array.h"

#include "ClientTestUtils.h"
#include "GeometryBindingImpl.h"
#include "AppearanceImpl.h"
#include "MeshNodeImpl.h"
#include "ArrayResourceImpl.h"
#include "Resource/EffectResource.h"
#include "EffectImpl.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class MeshNodeTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    protected:
        virtual void SetUp()
        {
            m_meshNode = m_scene.createMeshNode("node");
        }

        AssertionResult renderableDataInstancesAreSetInScene(DataInstanceHandle uniformDataInstance, DataInstanceHandle attributeDataInstance)
        {
            const RenderableHandle renderableHandle = m_meshNode->impl.getRenderableHandle();
            const auto actualDataInstances = m_internalScene.getRenderable(renderableHandle).dataInstances;
            if (uniformDataInstance != actualDataInstances[ERenderableDataSlotType_Uniforms])
            {
                return AssertionFailure() << "Renderable uniform data instance is not set in m_scene!";
            }
            if (attributeDataInstance != actualDataInstances[ERenderableDataSlotType_Geometry])
            {
                return AssertionFailure() << "Renderable attribute data instance is not set in m_scene!";
            }
            return AssertionSuccess();
        }

        AssertionResult effectResourceIsSetInScene(const Appearance& appearance)
        {
            const EffectImpl* effect = appearance.impl.getEffectImpl();
            const Renderable renderable = m_internalScene.getRenderable(m_meshNode->impl.getRenderableHandle());
            const DataLayoutHandle& dataLayout = m_internalScene.getLayoutOfDataInstance(renderable.dataInstances[ERenderableDataSlotType_Geometry]);
            const ResourceContentHash& effectHash = m_internalScene.getDataLayout(dataLayout).getEffectHash();

            EXPECT_EQ(effect->getLowlevelResourceHash(), effectHash);

            return AssertionSuccess();
        }

        AssertionResult renderableStateIsSetInScene(const Appearance& appearance)
        {
            const RenderableHandle renderableHandle = m_meshNode->impl.getRenderableHandle();

            EXPECT_EQ(appearance.impl.getRenderStateHandle(), m_internalScene.getRenderable(renderableHandle).renderState);

            return AssertionSuccess();
        }

        AssertionResult meshNodeUniformAndAttributesIsSetInScene(const Appearance& appearance, const GeometryBinding& geometry)
        {
            EXPECT_TRUE(renderableDataInstancesAreSetInScene(appearance.impl.getUniformDataInstance(), geometry.impl.getAttributeDataInstance()));

            EXPECT_TRUE(effectResourceIsSetInScene(appearance));
            EXPECT_TRUE(renderableStateIsSetInScene(appearance));

            return AssertionSuccess();
        }

        void setAnAppearanceForTesting()
        {
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");
            m_meshNode->setAppearance(*appearance);
        }

        void setAGeometryBindingForTesting(bool useIndices = true)
        {
            GeometryBinding& geometry = createValidGeometry(nullptr, useIndices);
            m_meshNode->setGeometryBinding(geometry);
        }

        MeshNode* m_meshNode;
    };

    TEST_F(MeshNodeTest, canValidate)
    {
        setAnAppearanceForTesting();
        setAGeometryBindingForTesting();

        EXPECT_EQ(StatusOK, m_meshNode->validate());
    }

    TEST_F(MeshNodeTest, failsValidationIfNoAppearanceWasSet)
    {
        setAGeometryBindingForTesting();

        EXPECT_NE(StatusOK, m_meshNode->validate());
    }

    TEST_F(MeshNodeTest, failsValidationIfNoGeometryWasSet)
    {
        setAnAppearanceForTesting();

        EXPECT_NE(StatusOK, m_meshNode->validate());
    }

    TEST_F(MeshNodeTest, failsValidationIfStartIndexExeedsSizeOfIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryBindingForTesting();

        m_meshNode->setStartIndex(2);

        EXPECT_NE(StatusOK, m_meshNode->validate());
    }

    TEST_F(MeshNodeTest, failsValidationIfIndexCountExeedsSizeOfIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryBindingForTesting();

        m_meshNode->setIndexCount(2);

        EXPECT_NE(StatusOK, m_meshNode->validate());
    }

    TEST_F(MeshNodeTest, failsValidationIfStartIndexPlusIndexCountExeedsSizeOfIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryBindingForTesting();

        m_meshNode->setStartIndex(1);
        m_meshNode->setIndexCount(1);

        EXPECT_NE(StatusOK, m_meshNode->validate());
    }

    TEST_F(MeshNodeTest, getsNullPointerForInitalAppearanceAndGeometry)
    {
        EXPECT_EQ(nullptr, m_meshNode->getAppearance());
        EXPECT_EQ(nullptr, m_meshNode->getGeometryBinding());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameAppearance)
    {
        Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");

        EXPECT_EQ(StatusOK, m_meshNode->setAppearance(*appearance));

        EXPECT_EQ(appearance, m_meshNode->getAppearance());
    }

    TEST_F(MeshNodeTest, reportsErrorWhenSetAppearanceFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));

        Appearance* appearance = anotherScene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");
        ASSERT_TRUE(appearance != nullptr);

        EXPECT_NE(StatusOK, m_meshNode->setAppearance(*appearance));
        client.destroy(anotherScene);
    }

    TEST_F(MeshNodeTest, setsAndGetsSameGeometry)
    {
        GeometryBinding& geometry = createValidGeometry();
        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry.setIndices(indexArray));

        EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry));

        EXPECT_EQ(&geometry, m_meshNode->getGeometryBinding());
    }

    TEST_F(MeshNodeTest, reportsErrorWhenSetGeometryFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));

        GeometryBinding* geometry = anotherScene.createGeometryBinding(*TestEffects::CreateTestEffect(client), "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry->setIndices(indexArray));

        EXPECT_NE(StatusOK, m_meshNode->setGeometryBinding(*geometry));
        client.destroy(anotherScene);
    }

    TEST_F(MeshNodeTest, settingGeometryWithIndicesSetsIndicesCount)
    {
        GeometryBinding& geometry = createValidGeometry();
        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry.setIndices(indexArray));
        EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry));
        EXPECT_EQ(indexArray.impl.getElementCount(), m_meshNode->getIndexCount());
    }

    TEST_F(MeshNodeTest, settingGeometryHavingDifferentEffectFromAppearanceReportsError)
    {
        Effect* effect1 = TestEffects::CreateTestEffect(client);
        Effect* effect2 = TestEffects::CreateDifferentTestEffect(client);

        Appearance* appearance = m_scene.createAppearance(*effect1, "appearance");
        EXPECT_EQ(StatusOK, m_meshNode->setAppearance(*appearance));

        GeometryBinding& geometry = *m_scene.createGeometryBinding(*effect2);
        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry.setIndices(indexArray));
        EXPECT_NE(StatusOK, m_meshNode->setGeometryBinding(geometry));
    }

    TEST_F(MeshNodeTest, settingAppearanceHavingDifferentEffectFromGeometryReportsError)
    {
        Effect* effect1 = TestEffects::CreateTestEffect(client);
        Effect* effect2 = TestEffects::CreateDifferentTestEffect(client);

        GeometryBinding& geometry = *m_scene.createGeometryBinding(*effect2);
        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry.setIndices(indexArray));
        EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry));

        Appearance* appearance = m_scene.createAppearance(*effect1, "appearance");
        EXPECT_NE(StatusOK, m_meshNode->setAppearance(*appearance));
    }

    TEST_F(MeshNodeTest, setsAppearanceMultipleTimesAndChecksInScene)
    {
        GeometryBinding& geometry = createValidGeometry(TestEffects::CreateTestEffect(client));
        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry.setIndices(indexArray));
        EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry));

        for (int i = 0; i < 3; i++)
        {
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");
            EXPECT_EQ(StatusOK, m_meshNode->setAppearance(*appearance));
            EXPECT_EQ(appearance, m_meshNode->getAppearance());
            EXPECT_TRUE(meshNodeUniformAndAttributesIsSetInScene(*appearance, geometry));
        }
    }

    TEST_F(MeshNodeTest, setsGeometryMultipleTimesAndChecksInScene)
    {
        Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");
        EXPECT_EQ(StatusOK, m_meshNode->setAppearance(*appearance));

        for (int i = 0; i < 3; i++)
        {
            GeometryBinding& geometry = createValidGeometry(TestEffects::CreateTestEffect(client));
            const UInt16Array& indexArray = createValidIndexArray();
            EXPECT_EQ(StatusOK, geometry.setIndices(indexArray));
            EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry));
            EXPECT_EQ(appearance, m_meshNode->getAppearance());
            EXPECT_TRUE(meshNodeUniformAndAttributesIsSetInScene(*appearance, geometry));
        }
    }

    TEST_F(MeshNodeTest, removeAppearanceAndGeometryRemovesTheseFromMeshNode)
    {
        Appearance* appearance = this->m_scene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");
        EXPECT_EQ(StatusOK, m_meshNode->setAppearance(*appearance));
        GeometryBinding& geometry = createValidGeometry(TestEffects::CreateTestEffect(client));
        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry.setIndices(indexArray));
        EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry));

        EXPECT_EQ(StatusOK, m_meshNode->removeAppearanceAndGeometry());

        EXPECT_TRUE(nullptr == m_meshNode->getAppearance());
        EXPECT_TRUE(nullptr == m_meshNode->getGeometryBinding());

        const RenderableHandle renderableHandle = m_meshNode->impl.getRenderableHandle();
        EXPECT_FALSE(this->m_internalScene.getRenderable(renderableHandle).dataInstances[ERenderableDataSlotType_Uniforms].isValid());
        EXPECT_FALSE(this->m_internalScene.getRenderable(renderableHandle).dataInstances[ERenderableDataSlotType_Geometry].isValid());
        EXPECT_FALSE(this->m_internalScene.getRenderable(renderableHandle).renderState.isValid());
    }

    TEST_F(MeshNodeTest, canSetNewAppearanceAndGeometryWhichAreIncompatibleWithThePrevious)
    {
        // first set of appearance/geometry
        Effect* effect1 = TestEffects::CreateTestEffect(client);
        GeometryBinding& geometry1 = createValidGeometry(effect1);
        const UInt16Array& indexArray = createValidIndexArray();
        EXPECT_EQ(StatusOK, geometry1.setIndices(indexArray));
        EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry1));
        Appearance* appearance1 = this->m_scene.createAppearance(*effect1, "appearance");
        EXPECT_EQ(StatusOK, m_meshNode->setAppearance(*appearance1));

        // reset
        EXPECT_EQ(StatusOK, m_meshNode->removeAppearanceAndGeometry());

        // second set of appearance/geometry not compatible with the previous ones
        Effect* effect2 = TestEffects::CreateDifferentTestEffect(client);
        GeometryBinding& geometry2 = createValidGeometry(effect2);
        EXPECT_EQ(StatusOK, geometry2.setIndices(indexArray));
        EXPECT_EQ(StatusOK, m_meshNode->setGeometryBinding(geometry2));
        Appearance* appearance2 = this->m_scene.createAppearance(*effect2, "appearance");
        EXPECT_EQ(StatusOK, m_meshNode->setAppearance(*appearance2));
    }

    TEST_F(MeshNodeTest, checksIfMeshNodeIsInitiallyVisible)
    {
        EXPECT_EQ(m_meshNode->impl.getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(MeshNodeTest, setsAndGetsSameStartIndex)
    {
        uint32_t startIndex = 1;
        EXPECT_EQ(StatusOK, m_meshNode->setStartIndex(startIndex));

        EXPECT_EQ(startIndex, m_meshNode->getStartIndex());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameStartVertex)
    {
        uint32_t startVertex = 231u;
        EXPECT_EQ(StatusOK, m_meshNode->setStartVertex(startVertex));

        EXPECT_EQ(startVertex, m_meshNode->getStartVertex());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameIndexCount)
    {
        const uint32_t indexCount = 1u;
        EXPECT_EQ(StatusOK, m_meshNode->setIndexCount(indexCount));

        EXPECT_EQ(indexCount, m_meshNode->getIndexCount());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameInstanceCount)
    {
        const uint32_t instanceCount(501u);
        EXPECT_EQ(StatusOK, m_meshNode->setInstanceCount(instanceCount));

        EXPECT_EQ(instanceCount, m_meshNode->getInstanceCount());
    }

    TEST_F(MeshNodeTest, doesNotAllowNoInstances)
    {
        EXPECT_NE(StatusOK, m_meshNode->setInstanceCount(0u));
    }

    TEST_F(MeshNodeTest, succeedsValidationIfNotUsingIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryBindingForTesting(false);

        m_meshNode->setIndexCount(1);

        EXPECT_EQ(StatusOK, m_meshNode->validate());
    }

    TEST_F(MeshNodeTest, failsValidationIfNotUsingIndexArrayAndIndexCountNotSet)
    {
        setAnAppearanceForTesting();
        setAGeometryBindingForTesting(false);

        EXPECT_NE(StatusOK, m_meshNode->validate());
    }
}
