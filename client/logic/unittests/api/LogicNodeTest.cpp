//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "ramses-logic/LogicNode.h"
#include "ramses-logic/Property.h"
#include "impl/LogicNodeImpl.h"
#include "impl/PropertyImpl.h"

namespace rlogic::internal
{
    class LogicNodeDummy : public LogicNode
    {
    public:
        explicit LogicNodeDummy(std::unique_ptr<internal::LogicNodeImpl> impl)
            : LogicNode(std::move(impl))
        {
            m_impl.setLogicObject(*this);
        }
    };

    class LogicNodeImplMock : public LogicNodeImpl
    {
    public:
        // Forwarding constructor
        explicit LogicNodeImplMock(std::string_view name)
            : LogicNodeImpl(name, 1u)
        {
        }

        // Expose protected method for testing
        using LogicNodeImpl::setRootProperties;

        MOCK_METHOD(std::optional<LogicNodeRuntimeError>, update, (), (override, final));
        MOCK_METHOD(void, createRootProperties, (), (override, final));
    };

    class ALogicNodeImpl : public ::testing::Test
    {
    };

    TEST_F(ALogicNodeImpl, RemembersNameGivenInConstructor)
    {
        const LogicNodeImplMock logicNode("name");
        EXPECT_EQ(logicNode.getName(), "name");
    }

    TEST_F(ALogicNodeImpl, CanReceiveNewName)
    {
        LogicNodeImplMock logicNode("name");
        logicNode.setName("newName");
        EXPECT_EQ(logicNode.getName(), "newName");
    }

    TEST_F(ALogicNodeImpl, DirtyByDefault)
    {
        const LogicNodeImplMock logicNode("");
        EXPECT_TRUE(logicNode.isDirty());
    }

    TEST_F(ALogicNodeImpl, DirtyWhenSetDirty)
    {
        LogicNodeImplMock logicNode("");
        logicNode.setDirty(false);
        EXPECT_FALSE(logicNode.isDirty());
        logicNode.setDirty(true);
        EXPECT_TRUE(logicNode.isDirty());
    }

    TEST_F(ALogicNodeImpl, TakesOwnershipOfGivenProperties)
    {
        const HierarchicalTypeData nestedTypeData(
            TypeData{ "root", EPropertyType::Struct }, {
                HierarchicalTypeData(TypeData{"nested", EPropertyType::Struct}, {
                    MakeType("float", EPropertyType::Float),
                    MakeStruct("nested", { TypeData("float", EPropertyType::Float) })
                    })
            });

        auto inputType = nestedTypeData;
        auto outputType = nestedTypeData;
        // These usually come from subclasses deserialization code
        auto inputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(inputType, EPropertySemantics::ScriptInput));
        auto outputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(outputType, EPropertySemantics::ScriptOutput));

        LogicNodeDummy logicNode{ std::make_unique<LogicNodeImplMock>("name") };
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) known test type
        (static_cast<LogicNodeImplMock&>(logicNode.m_impl)).setRootProperties(std::move(inputs), std::move(outputs));

        // const outputs
        {
            const Property* root = logicNode.getOutputs();
            EXPECT_EQ(&logicNode, &root->getOwningLogicNode());
            ASSERT_EQ(1u, root->getChildCount());

            const auto nested = root->getChild(0);
            EXPECT_EQ(&logicNode, &nested->getOwningLogicNode());
            ASSERT_EQ(2u, nested->getChildCount());

            const auto nestedFloat = nested->getChild(0);
            EXPECT_EQ(&logicNode, &nestedFloat->getOwningLogicNode());

            const auto nestedNested = nested->getChild(1);
            EXPECT_EQ(&logicNode, &nestedNested->getOwningLogicNode());
            ASSERT_EQ(1u, nestedNested->getChildCount());

            const auto nestedNestedFloat = nestedNested->getChild(0);
            EXPECT_EQ(&logicNode, &nestedNestedFloat->getOwningLogicNode());
        }

        // non-const inputs
        {
            Property* root = logicNode.getInputs();
            EXPECT_EQ(&logicNode, &root->getOwningLogicNode());
            ASSERT_EQ(1u, root->getChildCount());

            const auto nested = root->getChild(0);
            EXPECT_EQ(&logicNode, &nested->getOwningLogicNode());
            ASSERT_EQ(2u, nested->getChildCount());

            const auto nestedFloat = nested->getChild(0);
            EXPECT_EQ(&logicNode, &nestedFloat->getOwningLogicNode());

            const auto nestedNested = nested->getChild(1);
            EXPECT_EQ(&logicNode, &nestedNested->getOwningLogicNode());
            ASSERT_EQ(1u, nestedNested->getChildCount());

            const auto nestedNestedFloat = nestedNested->getChild(0);
            EXPECT_EQ(&logicNode, &nestedNestedFloat->getOwningLogicNode());
        }
    }
}
