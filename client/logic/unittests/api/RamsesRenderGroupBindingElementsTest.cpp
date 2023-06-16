//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"
#include "ramses-client-api/RenderGroup.h"
#include "impl/RamsesRenderGroupBindingElementsImpl.h"

namespace ramses::internal
{
    class ARamsesRenderGroupBindingElements : public ALogicEngine
    {
    protected:
        void expectElements(const RamsesRenderGroupBindingElementsImpl::Elements& elements) const
        {
            EXPECT_EQ(m_elements.m_impl->getElements(), elements);
        }

        RamsesRenderGroupBindingElements m_elements;
    };

    TEST_F(ARamsesRenderGroupBindingElements, CanAddMeshAndRenderGroupElement)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(m_elements.addElement(*m_renderGroup, "rg"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });
    }

    TEST_F(ARamsesRenderGroupBindingElements, CanBeCopyAndMoveConstructed)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        const RamsesRenderGroupBindingElementsImpl::Elements expectedElements{ { "mesh", m_meshNode } };

        RamsesRenderGroupBindingElements elementsCopy{ m_elements };
        EXPECT_EQ(elementsCopy.m_impl->getElements(), expectedElements);

        RamsesRenderGroupBindingElements elementsMove{ std::move(elementsCopy) };
        EXPECT_EQ(elementsMove.m_impl->getElements(), expectedElements);
    }

    TEST_F(ARamsesRenderGroupBindingElements, CanBeCopyAndMoveAssigned)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        const RamsesRenderGroupBindingElementsImpl::Elements expectedElements{ { "mesh", m_meshNode } };

        RamsesRenderGroupBindingElements elementsCopy;
        elementsCopy = m_elements;
        EXPECT_EQ(elementsCopy.m_impl->getElements(), expectedElements);

        RamsesRenderGroupBindingElements elementsMove;
        elementsMove = std::move(elementsCopy);
        EXPECT_EQ(elementsMove.m_impl->getElements(), expectedElements);
    }

    TEST_F(ARamsesRenderGroupBindingElements, AddsElementUnderItsObjectNameIfNoElementNameProvided)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode));
        expectElements({ { "meshNode", m_meshNode } });
    }

    TEST_F(ARamsesRenderGroupBindingElements, FailsToAddElementIfNoElementNameProvidedAndObjectNameEmpty)
    {
        EXPECT_STREQ("", m_renderGroup->getName());
        EXPECT_FALSE(m_elements.addElement(*m_renderGroup));
        expectElements({});
    }

    TEST_F(ARamsesRenderGroupBindingElements, FailsToAddElementMoreThanOnce)
    {
        EXPECT_TRUE(m_elements.addElement(*m_meshNode, "mesh"));
        EXPECT_TRUE(m_elements.addElement(*m_renderGroup, "rg"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });
        EXPECT_FALSE(m_elements.addElement(*m_meshNode, "mesh"));
        EXPECT_FALSE(m_elements.addElement(*m_renderGroup, "rg"));
        expectElements({ { "mesh", m_meshNode }, { "rg", m_renderGroup } });
    }
}
