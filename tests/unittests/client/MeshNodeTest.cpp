//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses/client/MeshNode.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/ArrayResource.h"

#include "ClientTestUtils.h"
#include "impl/GeometryImpl.h"
#include "impl/AppearanceImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/ArrayResourceImpl.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "impl/EffectImpl.h"

using namespace testing;

namespace ramses::internal
{
    class MeshNodeTest : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_meshNode = m_scene.createMeshNode("node");
        }

        AssertionResult renderableDataInstancesAreSetInScene(DataInstanceHandle uniformDataInstance, DataInstanceHandle attributeDataInstance)
        {
            const RenderableHandle renderableHandle = m_meshNode->impl().getRenderableHandle();
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
            const EffectImpl* effect = appearance.impl().getEffectImpl();
            const Renderable renderable = m_internalScene.getRenderable(m_meshNode->impl().getRenderableHandle());
            const DataLayoutHandle& dataLayout = m_internalScene.getLayoutOfDataInstance(renderable.dataInstances[ERenderableDataSlotType_Geometry]);
            const ResourceContentHash& effectHash = m_internalScene.getDataLayout(dataLayout).getEffectHash();

            EXPECT_EQ(effect->getLowlevelResourceHash(), effectHash);

            return AssertionSuccess();
        }

        AssertionResult renderableStateIsSetInScene(const Appearance& appearance)
        {
            const RenderableHandle renderableHandle = m_meshNode->impl().getRenderableHandle();

            EXPECT_EQ(appearance.impl().getRenderStateHandle(), m_internalScene.getRenderable(renderableHandle).renderState);

            return AssertionSuccess();
        }

        AssertionResult meshNodeUniformAndAttributesIsSetInScene(const Appearance& appearance, const Geometry& geometry)
        {
            EXPECT_TRUE(renderableDataInstancesAreSetInScene(appearance.impl().getUniformDataInstance(), geometry.impl().getAttributeDataInstance()));

            EXPECT_TRUE(effectResourceIsSetInScene(appearance));
            EXPECT_TRUE(renderableStateIsSetInScene(appearance));

            return AssertionSuccess();
        }

        void setAnAppearanceForTesting()
        {
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(m_scene), "appearance");
            m_meshNode->setAppearance(*appearance);
        }

        void setAGeometryForTesting(bool useIndices = true)
        {
            Geometry& geometry = createValidGeometry(nullptr, useIndices);
            m_meshNode->setGeometry(geometry);
        }

        MeshNode* m_meshNode{nullptr};
    };

    TEST_F(MeshNodeTest, canValidate)
    {
        setAnAppearanceForTesting();
        setAGeometryForTesting();

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(MeshNodeTest, failsValidationIfNoAppearanceWasSet)
    {
        setAGeometryForTesting();

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(MeshNodeTest, failsValidationIfNoGeometryWasSet)
    {
        setAnAppearanceForTesting();

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(MeshNodeTest, failsValidationIfStartIndexExeedsSizeOfIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryForTesting();

        m_meshNode->setStartIndex(2);

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(MeshNodeTest, failsValidationIfIndexCountExeedsSizeOfIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryForTesting();

        m_meshNode->setIndexCount(2);

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(MeshNodeTest, failsValidationIfStartIndexPlusIndexCountExeedsSizeOfIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryForTesting();

        m_meshNode->setStartIndex(1);
        m_meshNode->setIndexCount(1);

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(MeshNodeTest, getsNullPointerForInitalAppearanceAndGeometry)
    {
        EXPECT_EQ(nullptr, m_meshNode->getAppearance());
        EXPECT_EQ(nullptr, m_meshNode->getGeometry());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameAppearance)
    {
        Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(m_scene), "appearance");

        EXPECT_TRUE(m_meshNode->setAppearance(*appearance));

        EXPECT_EQ(appearance, m_meshNode->getAppearance());
    }

    TEST_F(MeshNodeTest, reportsErrorWhenSetAppearanceFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(sceneId_t(12u));

        Appearance* appearance = anotherScene.createAppearance(*TestEffects::CreateTestEffect(anotherScene), "appearance");
        ASSERT_TRUE(appearance != nullptr);

        EXPECT_FALSE(m_meshNode->setAppearance(*appearance));
        client.destroy(anotherScene);
    }

    TEST_F(MeshNodeTest, setsAndGetsSameGeometry)
    {
        Geometry& geometry = createValidGeometry();
        ArrayResource& indexArray = createValidIndexArray();
        EXPECT_TRUE(geometry.setIndices(indexArray));

        EXPECT_TRUE(m_meshNode->setGeometry(geometry));

        EXPECT_EQ(&geometry, m_meshNode->getGeometry());
    }

    TEST_F(MeshNodeTest, reportsErrorWhenSetGeometryFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(sceneId_t(12u));

        Geometry* geometry = anotherScene.createGeometry(*TestEffects::CreateTestEffect(anotherScene), "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const uint16_t indices = 0;
        ArrayResource& indexArray = *anotherScene.createArrayResource(1u, &indices);
        EXPECT_TRUE(geometry->setIndices(indexArray));

        EXPECT_FALSE(m_meshNode->setGeometry(*geometry));
        client.destroy(anotherScene);
    }

    TEST_F(MeshNodeTest, settingGeometryWithIndicesSetsIndicesCount)
    {
        Geometry& geometry = createValidGeometry();
        ArrayResource& indexArray = createValidIndexArray();
        EXPECT_TRUE(geometry.setIndices(indexArray));
        EXPECT_TRUE(m_meshNode->setGeometry(geometry));
        EXPECT_EQ(indexArray.impl().getElementCount(), m_meshNode->getIndexCount());
    }

    TEST_F(MeshNodeTest, settingGeometryHavingDifferentEffectFromAppearanceReportsError)
    {
        Effect* effect1 = TestEffects::CreateTestEffect(m_scene);
        Effect* effect2 = TestEffects::CreateDifferentTestEffect(m_scene);

        Appearance* appearance = m_scene.createAppearance(*effect1, "appearance");
        EXPECT_TRUE(m_meshNode->setAppearance(*appearance));

        Geometry& geometry = *m_scene.createGeometry(*effect2);
        ArrayResource& indexArray = createValidIndexArray();
        EXPECT_TRUE(geometry.setIndices(indexArray));
        EXPECT_FALSE(m_meshNode->setGeometry(geometry));
    }

    TEST_F(MeshNodeTest, settingAppearanceHavingDifferentEffectFromGeometryReportsError)
    {
        Effect* effect1 = TestEffects::CreateTestEffect(m_scene);
        Effect* effect2 = TestEffects::CreateDifferentTestEffect(m_scene);

        Geometry& geometry = *m_scene.createGeometry(*effect2);
        ArrayResource& indexArray = createValidIndexArray();
        EXPECT_TRUE(geometry.setIndices(indexArray));
        EXPECT_TRUE(m_meshNode->setGeometry(geometry));

        Appearance* appearance = m_scene.createAppearance(*effect1, "appearance");
        EXPECT_FALSE(m_meshNode->setAppearance(*appearance));
    }

    TEST_F(MeshNodeTest, setsAppearanceMultipleTimesAndChecksInScene)
    {
        Geometry& geometry = createValidGeometry(TestEffects::CreateTestEffect(m_scene));
        ArrayResource& indexArray = createValidIndexArray();
        EXPECT_TRUE(geometry.setIndices(indexArray));
        EXPECT_TRUE(m_meshNode->setGeometry(geometry));

        for (int i = 0; i < 3; i++)
        {
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(m_scene), "appearance");
            EXPECT_TRUE(m_meshNode->setAppearance(*appearance));
            EXPECT_EQ(appearance, m_meshNode->getAppearance());
            EXPECT_TRUE(meshNodeUniformAndAttributesIsSetInScene(*appearance, geometry));
        }
    }

    TEST_F(MeshNodeTest, setsGeometryMultipleTimesAndChecksInScene)
    {
        Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(m_scene), "appearance");
        EXPECT_TRUE(m_meshNode->setAppearance(*appearance));

        for (int i = 0; i < 3; i++)
        {
            Geometry& geometry = createValidGeometry(TestEffects::CreateTestEffect(m_scene));
            ArrayResource& indexArray = createValidIndexArray();
            EXPECT_TRUE(geometry.setIndices(indexArray));
            EXPECT_TRUE(m_meshNode->setGeometry(geometry));
            EXPECT_EQ(appearance, m_meshNode->getAppearance());
            EXPECT_TRUE(meshNodeUniformAndAttributesIsSetInScene(*appearance, geometry));
        }
    }

    TEST_F(MeshNodeTest, removeAppearanceAndGeometryRemovesTheseFromMeshNode)
    {
        Appearance* appearance = this->m_scene.createAppearance(*TestEffects::CreateTestEffect(m_scene), "appearance");
        EXPECT_TRUE(m_meshNode->setAppearance(*appearance));
        Geometry& geometry = createValidGeometry(TestEffects::CreateTestEffect(m_scene));
        ArrayResource& indexArray = createValidIndexArray();
        EXPECT_TRUE(geometry.setIndices(indexArray));
        EXPECT_TRUE(m_meshNode->setGeometry(geometry));

        EXPECT_TRUE(m_meshNode->removeAppearanceAndGeometry());

        EXPECT_TRUE(nullptr == m_meshNode->getAppearance());
        EXPECT_TRUE(nullptr == m_meshNode->getGeometry());

        const RenderableHandle renderableHandle = m_meshNode->impl().getRenderableHandle();
        EXPECT_FALSE(this->m_internalScene.getRenderable(renderableHandle).dataInstances[ERenderableDataSlotType_Uniforms].isValid());
        EXPECT_FALSE(this->m_internalScene.getRenderable(renderableHandle).dataInstances[ERenderableDataSlotType_Geometry].isValid());
        EXPECT_FALSE(this->m_internalScene.getRenderable(renderableHandle).renderState.isValid());
    }

    TEST_F(MeshNodeTest, canSetNewAppearanceAndGeometryWhichAreIncompatibleWithThePrevious)
    {
        // first set of appearance/geometry
        Effect* effect1 = TestEffects::CreateTestEffect(m_scene);
        Geometry& geometry1 = createValidGeometry(effect1);
        ArrayResource& indexArray = createValidIndexArray();
        EXPECT_TRUE(geometry1.setIndices(indexArray));
        EXPECT_TRUE(m_meshNode->setGeometry(geometry1));
        Appearance* appearance1 = this->m_scene.createAppearance(*effect1, "appearance");
        EXPECT_TRUE(m_meshNode->setAppearance(*appearance1));

        // reset
        EXPECT_TRUE(m_meshNode->removeAppearanceAndGeometry());

        // second set of appearance/geometry not compatible with the previous ones
        Effect* effect2 = TestEffects::CreateDifferentTestEffect(m_scene);
        Geometry& geometry2 = createValidGeometry(effect2);
        EXPECT_TRUE(geometry2.setIndices(indexArray));
        EXPECT_TRUE(m_meshNode->setGeometry(geometry2));
        Appearance* appearance2 = this->m_scene.createAppearance(*effect2, "appearance");
        EXPECT_TRUE(m_meshNode->setAppearance(*appearance2));
    }

    TEST_F(MeshNodeTest, checksIfMeshNodeIsInitiallyVisible)
    {
        EXPECT_EQ(m_meshNode->impl().getFlattenedVisibility(), EVisibilityMode::Visible);
    }

    TEST_F(MeshNodeTest, setsAndGetsSameStartIndex)
    {
        uint32_t startIndex = 1;
        EXPECT_TRUE(m_meshNode->setStartIndex(startIndex));

        EXPECT_EQ(startIndex, m_meshNode->getStartIndex());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameStartVertex)
    {
        uint32_t startVertex = 231u;
        EXPECT_TRUE(m_meshNode->setStartVertex(startVertex));

        EXPECT_EQ(startVertex, m_meshNode->getStartVertex());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameIndexCount)
    {
        const uint32_t indexCount = 1u;
        EXPECT_TRUE(m_meshNode->setIndexCount(indexCount));

        EXPECT_EQ(indexCount, m_meshNode->getIndexCount());
    }

    TEST_F(MeshNodeTest, setsAndGetsSameInstanceCount)
    {
        const uint32_t instanceCount(501u);
        EXPECT_TRUE(m_meshNode->setInstanceCount(instanceCount));

        EXPECT_EQ(instanceCount, m_meshNode->getInstanceCount());
    }

    TEST_F(MeshNodeTest, succeedsValidationIfNotUsingIndexArray)
    {
        setAnAppearanceForTesting();
        setAGeometryForTesting(false);

        m_meshNode->setIndexCount(1);

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(MeshNodeTest, failsValidationIfNotUsingIndexArrayAndIndexCountNotSet)
    {
        setAnAppearanceForTesting();
        setAGeometryForTesting(false);

        ValidationReport report;
        m_meshNode->validate(report);
        EXPECT_TRUE(report.hasError());
    }
}
