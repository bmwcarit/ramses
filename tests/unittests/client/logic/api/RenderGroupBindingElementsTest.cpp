//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"
#include "ramses/client/RenderGroup.h"
#include "impl/logic/RenderGroupBindingElementsImpl.h"

namespace ramses::internal
{
    class ARenderGroupBindingElements : public ALogicEngine
    {
    protected:
        void expectElements(const RenderGroupBindingElementsImpl::Elements& elements) const
        {
            EXPECT_EQ(m_elements.impl().getElements(), elements);
        }

        RenderGroupBindingElements m_elements;
    };

    TEST_F(ARenderGroupBindingElements, CanAddMeshAndRenderGroupElement)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(m_elements.addElement(*m_renderGroup, "rg"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });
    }

    TEST_F(ARenderGroupBindingElements, CanBeCopyAndMoveConstructed)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        const RenderGroupBindingElementsImpl::Elements expectedElements{ { "mesh", m_meshNode } };

        RenderGroupBindingElements elementsCopy{ m_elements };
        EXPECT_EQ(elementsCopy.impl().getElements(), expectedElements);

        RenderGroupBindingElements elementsMove{ std::move(elementsCopy) };
        EXPECT_EQ(elementsMove.impl().getElements(), expectedElements);
    }

    TEST_F(ARenderGroupBindingElements, CanBeCopyAndMoveAssigned)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        const RenderGroupBindingElementsImpl::Elements expectedElements{ { "mesh", m_meshNode } };

        RenderGroupBindingElements elementsCopy;
        elementsCopy = m_elements;
        EXPECT_EQ(elementsCopy.impl().getElements(), expectedElements);

        RenderGroupBindingElements elementsMove;
        elementsMove = std::move(elementsCopy);
        EXPECT_EQ(elementsMove.impl().getElements(), expectedElements);
    }

    TEST_F(ARenderGroupBindingElements, AddsElementUnderItsObjectNameIfNoElementNameProvided)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode));
        expectElements({ { "meshNode", m_meshNode } });
    }

    TEST_F(ARenderGroupBindingElements, FailsToAddElementIfNoElementNameProvidedAndObjectNameEmpty)
    {
        EXPECT_TRUE(m_renderGroup->getName().empty());
        EXPECT_FALSE(m_elements.addElement(*m_renderGroup));
        expectElements({});
    }

    TEST_F(ARenderGroupBindingElements, FailsToAddElementMoreThanOnce)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(m_elements.addElement(*m_renderGroup, "rg"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });
        EXPECT_FALSE(m_elements.addElement(*m_meshNode, "mesh"));
        EXPECT_FALSE(m_elements.addElement(*m_renderGroup, "rg"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });
    }

    TEST_F(ARenderGroupBindingElements, FailsToAddElementFromOtherScene)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(m_elements.addElement(*m_renderGroup, "rg"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });

        auto otherScene{ m_ramses.createScene(sceneId_t{ 666u }) };
        auto otherRG = otherScene->createRenderGroup("rg2");
        auto otherMesh = otherScene->createMeshNode("mesh2");
        EXPECT_FALSE(m_elements.addElement(*otherRG, "rg2"));
        EXPECT_FALSE(m_elements.addElement(*otherMesh, "mesh2"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });
    }
}
