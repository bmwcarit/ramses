//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "LogicEngineTest_Base.h"
#include "ramses/client/logic/Property.h"
#include "impl/logic/LogicNodeImpl.h"

namespace ramses::internal
{
    class ALogicNode : public ALogicEngine
    {
    protected:
        LogicNode& m_logicNode{ *m_logicEngine->createLuaInterface(m_interfaceSourceCode, "name") };
    };

    TEST_F(ALogicNode, HasName)
    {
        EXPECT_EQ("name", m_logicNode.getName());
    }

    TEST_F(ALogicNode, CanReceiveNewName)
    {
        EXPECT_TRUE(m_logicNode.setName("newName"));
        EXPECT_EQ("newName", m_logicNode.getName());
    }

    TEST_F(ALogicNode, DirtyByDefault)
    {
        EXPECT_TRUE(m_logicNode.impl().isDirty());
    }

    TEST_F(ALogicNode, DirtyWhenSetDirty)
    {
        m_logicNode.impl().setDirty(false);
        EXPECT_FALSE(m_logicNode.impl().isDirty());
        m_logicNode.impl().setDirty(true);
        EXPECT_TRUE(m_logicNode.impl().isDirty());
    }

    TEST_F(ALogicNode, OwnsProperties)
    {
        // const outputs
        {
            const Property* root = m_logicNode.getOutputs();
            EXPECT_EQ(&m_logicNode, &root->getOwningLogicNode());
            ASSERT_EQ(1u, root->getChildCount());

            const auto prop = root->getChild(0);
            EXPECT_EQ(&m_logicNode, &prop->getOwningLogicNode());
        }

        // non-const inputs
        {
            Property* root = m_logicNode.getInputs();
            EXPECT_EQ(&m_logicNode, &root->getOwningLogicNode());
            ASSERT_EQ(1u, root->getChildCount());

            const auto prop = root->getChild(0);
            EXPECT_EQ(&m_logicNode, &prop->getOwningLogicNode());
        }
    }
}
