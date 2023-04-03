//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "impl/PropertyImpl.h"
#include "impl/LuaScriptImpl.h"
#include "internals/SolState.h"
#include "internals/ErrorReporting.h"

#include "ramses-logic/Property.h"
#include "ramses-logic/LogicEngine.h"

#include "flatbuffers/flatbuffers.h"

#include "generated/PropertyGen.h"

#include "LogicNodeDummy.h"
#include "LogTestUtils.h"

#include <memory>
#include <fmt/format.h>

namespace rlogic::internal
{
    class AProperty : public ::testing::Test
    {
    public:
        LogicNodeDummyImpl m_dummyNode{ "DummyNode" };

        std::unique_ptr<PropertyImpl> CreateInputProperty(EPropertyType type, bool assignDummyLogicNode = true)
        {
            return CreateProperty(MakeType("", type), EPropertySemantics::ScriptInput, assignDummyLogicNode);
        }

        std::unique_ptr<PropertyImpl> CreateOutputProperty(EPropertyType type, bool assignDummyLogicNode = true)
        {
            return CreateProperty(MakeType("", type), EPropertySemantics::ScriptOutput, assignDummyLogicNode);
        }

        std::unique_ptr<PropertyImpl> CreateBindingInput(EPropertyType type)
        {
            return CreateProperty(MakeType("", type), EPropertySemantics::BindingInput, true);
        }

        std::unique_ptr<PropertyImpl> CreateProperty(const HierarchicalTypeData& typeData, EPropertySemantics semantics, bool assignDummyLogicNode)
        {
            auto property = std::make_unique<PropertyImpl>(typeData, semantics);
            if (assignDummyLogicNode)
            {
                property->setLogicNode(m_dummyNode);
            }
            return property;
        }

        // Silence logs, unless explicitly enabled, to reduce spam and speed up tests
        ScopedLogContextLevel m_silenceLogs{ELogMessageType::Off};
    };

    TEST_F(AProperty, HasANameAfterCreation)
    {
        Property property(CreateProperty(MakeType("PropertyName", EPropertyType::Float), EPropertySemantics::ScriptInput, true));
        EXPECT_EQ("PropertyName", property.getName());
    }

    TEST_F(AProperty, HasATypeAfterCreation)
    {
        Property property(CreateInputProperty(EPropertyType::Float));
        EXPECT_EQ(EPropertyType::Float, property.getType());
    }

    TEST_F(AProperty, HasAReferenceFromImplToHLObject)
    {
        Property property(CreateInputProperty(EPropertyType::Float));
        auto& impl = *property.m_impl;
        EXPECT_EQ(&property, &impl.getPropertyInstance());
        const auto& constImpl = impl;
        EXPECT_EQ(&property, &constImpl.getPropertyInstance());
    }

    TEST_F(AProperty, IsNotLinkedAfterCreation)
    {
        Property inputProperty(CreateInputProperty(EPropertyType::Float));
        EXPECT_FALSE(inputProperty.isLinked());
        EXPECT_FALSE(inputProperty.hasIncomingLink());
        EXPECT_FALSE(inputProperty.hasOutgoingLink());
        EXPECT_FALSE(inputProperty.getIncomingLink());
        EXPECT_EQ(0u, inputProperty.getOutgoingLinksCount());
        EXPECT_FALSE(inputProperty.getOutgoingLink(0u));
        Property outputProperty(CreateOutputProperty(EPropertyType::Float));
        EXPECT_FALSE(outputProperty.isLinked());
        EXPECT_FALSE(outputProperty.hasIncomingLink());
        EXPECT_FALSE(outputProperty.hasOutgoingLink());
        EXPECT_FALSE(outputProperty.getIncomingLink());
        EXPECT_EQ(0u, outputProperty.getOutgoingLinksCount());
        EXPECT_FALSE(outputProperty.getOutgoingLink(0u));
    }

    TEST_F(AProperty, CanBeLinkedAndUnlinked)
    {
        Property property1(CreateOutputProperty(EPropertyType::Float));
        Property property2(CreateInputProperty(EPropertyType::Float));
        property2.m_impl->setIncomingLink(*property1.m_impl, false);
        EXPECT_FALSE(property1.m_impl->isInput());
        EXPECT_TRUE(property1.m_impl->isOutput());
        EXPECT_TRUE(property1.isLinked());
        EXPECT_TRUE(property1.hasOutgoingLink());
        EXPECT_FALSE(property1.hasIncomingLink());
        EXPECT_FALSE(property1.getIncomingLink());
        EXPECT_TRUE(property2.m_impl->isInput());
        EXPECT_FALSE(property2.m_impl->isOutput());
        EXPECT_TRUE(property2.isLinked());
        EXPECT_TRUE(property2.hasIncomingLink());
        EXPECT_FALSE(property2.hasOutgoingLink());
        ASSERT_TRUE(property2.getIncomingLink());
        EXPECT_EQ(property2.getIncomingLink()->source, &property1);
        EXPECT_EQ(property2.getIncomingLink()->target, &property2);
        EXPECT_FALSE(property2.getIncomingLink()->isWeakLink);
        ASSERT_EQ(1u, property1.getOutgoingLinksCount());
        ASSERT_TRUE(property1.getOutgoingLink(0u));
        EXPECT_EQ(property1.getOutgoingLink(0u)->source, &property1);
        EXPECT_EQ(property1.getOutgoingLink(0u)->target, &property2);
        EXPECT_FALSE(property1.getOutgoingLink(0u)->isWeakLink);

        property2.m_impl->resetIncomingLink();
        EXPECT_FALSE(property1.isLinked());
        EXPECT_FALSE(property2.isLinked());
        EXPECT_FALSE(property1.getIncomingLink());
        EXPECT_EQ(0u, property1.getOutgoingLinksCount());
        EXPECT_FALSE(property2.getIncomingLink());
        EXPECT_EQ(0u, property2.getOutgoingLinksCount());
    }

    TEST_F(AProperty, CanLinkInterfaceProperties)
    {
        Property property1(CreateProperty(MakeType("name", EPropertyType::Float), EPropertySemantics::Interface, true));
        Property property2(CreateProperty(MakeType("name", EPropertyType::Float), EPropertySemantics::Interface, true));
        property2.m_impl->setIncomingLink(*property1.m_impl, false);
        EXPECT_TRUE(property1.m_impl->isInput());
        EXPECT_TRUE(property1.m_impl->isOutput());
        EXPECT_TRUE(property1.isLinked());
        EXPECT_TRUE(property1.hasOutgoingLink());
        EXPECT_FALSE(property1.hasIncomingLink());
        EXPECT_TRUE(property2.m_impl->isInput());
        EXPECT_TRUE(property2.m_impl->isOutput());
        EXPECT_TRUE(property2.isLinked());
        EXPECT_TRUE(property2.hasIncomingLink());
        EXPECT_FALSE(property2.hasOutgoingLink());
    }

    TEST_F(AProperty, CanBeLinkedAndUnlinked_weakLink)
    {
        Property property1(CreateOutputProperty(EPropertyType::Float));
        Property property2(CreateInputProperty(EPropertyType::Float));
        property2.m_impl->setIncomingLink(*property1.m_impl, true);
        EXPECT_FALSE(property1.m_impl->isInput());
        EXPECT_TRUE(property1.m_impl->isOutput());
        EXPECT_TRUE(property1.isLinked());
        EXPECT_TRUE(property1.hasOutgoingLink());
        EXPECT_FALSE(property1.hasIncomingLink());
        EXPECT_FALSE(property1.getIncomingLink());
        EXPECT_TRUE(property2.m_impl->isInput());
        EXPECT_FALSE(property2.m_impl->isOutput());
        EXPECT_TRUE(property2.isLinked());
        EXPECT_TRUE(property2.hasIncomingLink());
        EXPECT_FALSE(property2.hasOutgoingLink());
        ASSERT_TRUE(property2.getIncomingLink());
        EXPECT_EQ(property2.getIncomingLink()->source, &property1);
        EXPECT_EQ(property2.getIncomingLink()->target, &property2);
        EXPECT_TRUE(property2.getIncomingLink()->isWeakLink);
        ASSERT_EQ(1u, property1.getOutgoingLinksCount());
        ASSERT_TRUE(property1.getOutgoingLink(0u));
        EXPECT_EQ(property1.getOutgoingLink(0u)->source, &property1);
        EXPECT_EQ(property1.getOutgoingLink(0u)->target, &property2);
        EXPECT_TRUE(property1.getOutgoingLink(0u)->isWeakLink);

        property2.m_impl->resetIncomingLink();
        EXPECT_FALSE(property1.isLinked());
        EXPECT_FALSE(property2.isLinked());
        EXPECT_FALSE(property1.getIncomingLink());
        EXPECT_EQ(0u, property1.getOutgoingLinksCount());
        EXPECT_FALSE(property2.getIncomingLink());
        EXPECT_EQ(0u, property2.getOutgoingLinksCount());
    }

    TEST_F(AProperty, ReportsNoOutgoingLinkIfOutOfBounds)
    {
        Property property1(CreateOutputProperty(EPropertyType::Float));
        Property property2(CreateInputProperty(EPropertyType::Float));
        Property property3(CreateInputProperty(EPropertyType::Float));
        property2.m_impl->setIncomingLink(*property1.m_impl, true);
        property3.m_impl->setIncomingLink(*property1.m_impl, false);

        ASSERT_EQ(2u, property1.getOutgoingLinksCount());
        // link 1
        ASSERT_TRUE(property1.getOutgoingLink(0u));
        EXPECT_EQ(property1.getOutgoingLink(0u)->source, &property1);
        EXPECT_EQ(property1.getOutgoingLink(0u)->target, &property2);
        EXPECT_TRUE(property1.getOutgoingLink(0u)->isWeakLink);
        // link 2
        ASSERT_TRUE(property1.getOutgoingLink(1u));
        EXPECT_EQ(property1.getOutgoingLink(1u)->source, &property1);
        EXPECT_EQ(property1.getOutgoingLink(1u)->target, &property3);
        EXPECT_FALSE(property1.getOutgoingLink(1u)->isWeakLink);
        // link 3 does not exist
        EXPECT_FALSE(property1.getOutgoingLink(2u));
    }

    TEST_F(AProperty, CanBeInitializedWithAValue)
    {
        PropertyImpl property(MakeType("", EPropertyType::Float), EPropertySemantics::ScriptInput, PropertyValue{0.5f});
        EXPECT_FLOAT_EQ(0.5f, property.getValueAs<float>());
    }

    TEST_F(AProperty, CanCheckIfPropertyHasChild)
    {
        const Property propertyWithChildren(CreateProperty(MakeStruct("", { {"child1", EPropertyType::Int32}, {"child2", EPropertyType::Float} }), EPropertySemantics::ScriptInput, false));
        ASSERT_EQ(2u, propertyWithChildren.getChildCount());

        EXPECT_FALSE(propertyWithChildren.hasChild("invalidChildName"));
        EXPECT_EQ(nullptr, propertyWithChildren.getChild("invalidChildName"));
        EXPECT_TRUE(propertyWithChildren.hasChild("child1"));
        EXPECT_NE(nullptr, propertyWithChildren.getChild("child1"));
        EXPECT_TRUE(propertyWithChildren.hasChild("child2"));
        EXPECT_NE(nullptr, propertyWithChildren.getChild("child2"));
    }

    TEST_F(AProperty, BindingInputHasNoUserValueBeforeSetExplicitly)
    {
        Property prop(CreateBindingInput(EPropertyType::Float));

        EXPECT_FALSE(prop.m_impl->bindingInputHasNewValue());
        EXPECT_FALSE(prop.m_impl->checkForBindingInputNewValueAndReset());
    }

    TEST_F(AProperty, BindingInputHasUserValueAfterSetIsCalledSuccessfully)
    {
        Property prop(CreateBindingInput(EPropertyType::Float));

        EXPECT_FALSE(prop.m_impl->bindingInputHasNewValue());
        // Set with wrong type (failed sets) have no effect on user value status
        EXPECT_FALSE(prop.set<int32_t>(5));
        EXPECT_FALSE(prop.m_impl->bindingInputHasNewValue());
        EXPECT_TRUE(prop.set<float>(0.5f));
        EXPECT_TRUE(prop.m_impl->checkForBindingInputNewValueAndReset());
    }

    TEST_F(AProperty, BindingInputHasUserValueAfterLinkIsActivated_andValueChanged)
    {
        Property linkTarget(CreateBindingInput(EPropertyType::Float));
        Property linkSource(CreateProperty(MakeType("", EPropertyType::Float), EPropertySemantics::ScriptOutput, true));

        // Set to different than default value
        linkSource.m_impl->setValue({0.5f});

        // Simulate link behavior
        linkTarget.m_impl->setValue(linkSource.m_impl->getValue());
        EXPECT_TRUE(linkTarget.m_impl->checkForBindingInputNewValueAndReset());
    }

    TEST_F(AProperty, BindingInputHasNewUserValueAfterLinkIsActivated_whenNewValueSameAsOldValue)
    {
        Property linkTarget(CreateBindingInput(EPropertyType::Float));
        Property linkSource(CreateOutputProperty(EPropertyType::Float));

        // Set same value to both
        linkSource.set<float>(.5f);
        linkTarget.set<float>(.5f);

        // Simulate link behavior
        linkTarget.m_impl->setValue(linkSource.m_impl->getValue());
        EXPECT_TRUE(linkTarget.m_impl->checkForBindingInputNewValueAndReset());
    }

    TEST_F(AProperty, BindingInputHasNoUserValueAnymore_WhenConsumed)
    {
        Property prop(CreateBindingInput(EPropertyType::Float));

        ASSERT_FALSE(prop.m_impl->bindingInputHasNewValue());
        EXPECT_TRUE(prop.set<float>(0.5f));
        // Consume value => has no value any more
        EXPECT_TRUE(prop.m_impl->checkForBindingInputNewValueAndReset());
        EXPECT_FALSE(prop.m_impl->bindingInputHasNewValue());
    }

    TEST_F(AProperty, DoesntHaveChildrenAfterCreation)
    {
        Property desc(CreateInputProperty(EPropertyType::Float));
        ASSERT_EQ(0u, desc.getChildCount());
    }

    TEST_F(AProperty, ReturnsDefaultValue_ForPrimitiveTypes)
    {
        Property aFloat(CreateInputProperty(EPropertyType::Float));
        ASSERT_TRUE(aFloat.get<float>());
        EXPECT_FLOAT_EQ(0.0f, *aFloat.get<float>());

        Property aInt(CreateInputProperty(EPropertyType::Int32));
        ASSERT_TRUE(aInt.get<int32_t>());
        EXPECT_EQ(0, *aInt.get<int32_t>());

        Property aInt64(CreateInputProperty(EPropertyType::Int64));
        ASSERT_TRUE(aInt64.get<int64_t>());
        EXPECT_EQ(0L, *aInt64.get<int64_t>());

        Property aBool(CreateInputProperty(EPropertyType::Bool));
        ASSERT_TRUE(aBool.get<bool>());
        EXPECT_EQ(false, *aBool.get<bool>());

        Property aString(CreateInputProperty(EPropertyType::String));
        ASSERT_TRUE(aString.get<std::string>());
        EXPECT_EQ("", *aString.get<std::string>());
    }

    TEST_F(AProperty, ReturnsDefaultValue_VectorTypes)
    {
        Property aVec2f(CreateInputProperty(EPropertyType::Vec2f));
        Property aVec3f(CreateInputProperty(EPropertyType::Vec3f));
        Property aVec4f(CreateInputProperty(EPropertyType::Vec4f));
        Property aVec2i(CreateInputProperty(EPropertyType::Vec2i));
        Property aVec3i(CreateInputProperty(EPropertyType::Vec3i));
        Property aVec4i(CreateInputProperty(EPropertyType::Vec4i));

        EXPECT_TRUE(aVec2f.get<vec2f>());
        EXPECT_TRUE(aVec3f.get<vec3f>());
        EXPECT_TRUE(aVec4f.get<vec4f>());
        EXPECT_TRUE(aVec2i.get<vec2i>());
        EXPECT_TRUE(aVec3i.get<vec3i>());
        EXPECT_TRUE(aVec4i.get<vec4i>());

        vec2f vec2fValue = *aVec2f.get<vec2f>();
        vec3f vec3fValue = *aVec3f.get<vec3f>();
        vec4f vec4fValue = *aVec4f.get<vec4f>();

        ASSERT_EQ(2u, vec2fValue.size());
        EXPECT_FLOAT_EQ(0.0f, vec2fValue[0]);
        EXPECT_FLOAT_EQ(0.0f, vec2fValue[1]);

        ASSERT_EQ(3u, vec3fValue.size());
        EXPECT_FLOAT_EQ(0.0f, vec3fValue[0]);
        EXPECT_FLOAT_EQ(0.0f, vec3fValue[1]);
        EXPECT_FLOAT_EQ(0.0f, vec3fValue[2]);

        ASSERT_EQ(4u, vec4fValue.size());
        EXPECT_FLOAT_EQ(0.0f, vec4fValue[0]);
        EXPECT_FLOAT_EQ(0.0f, vec4fValue[1]);
        EXPECT_FLOAT_EQ(0.0f, vec4fValue[2]);
        EXPECT_FLOAT_EQ(0.0f, vec4fValue[3]);

        vec2i vec2iValue = *aVec2i.get<vec2i>();
        vec3i vec3iValue = *aVec3i.get<vec3i>();
        vec4i vec4iValue = *aVec4i.get<vec4i>();

        ASSERT_EQ(2u, vec2iValue.size());
        EXPECT_EQ(0, vec2iValue[0]);
        EXPECT_EQ(0, vec2iValue[1]);

        ASSERT_EQ(3u, vec3iValue.size());
        EXPECT_EQ(0, vec3iValue[0]);
        EXPECT_EQ(0, vec3iValue[1]);
        EXPECT_EQ(0, vec3iValue[2]);

        ASSERT_EQ(4u, vec4iValue.size());
        EXPECT_EQ(0, vec4iValue[0]);
        EXPECT_EQ(0, vec4iValue[1]);
        EXPECT_EQ(0, vec4iValue[2]);
        EXPECT_EQ(0, vec4iValue[3]);
    }

    TEST_F(AProperty, ReturnsValueIfItIsSetBeforehand_PrimitiveTypes)
    {
        Property aFloat(CreateInputProperty(EPropertyType::Float));
        Property aInt32(CreateInputProperty(EPropertyType::Int32));
        Property aInt64(CreateInputProperty(EPropertyType::Int64));
        Property aBool(CreateInputProperty(EPropertyType::Bool));
        Property aString(CreateInputProperty(EPropertyType::String));

        EXPECT_TRUE(aFloat.set<float>(47.11f));
        EXPECT_TRUE(aInt32.set<int32_t>(5));
        EXPECT_TRUE(aInt64.set<int64_t>(7));
        EXPECT_TRUE(aBool.set<bool>(true));
        EXPECT_TRUE(aString.set<std::string>("hello"));

        const auto valueFloat = aFloat.get<float>();
        const auto valueInt32 = aInt32.get<int32_t>();
        const auto valueInt64 = aInt64.get<int64_t>();
        const auto valueBool = aBool.get<bool>();
        const auto valueString = aString.get<std::string>();
        ASSERT_TRUE(valueFloat);
        ASSERT_TRUE(valueInt32);
        ASSERT_TRUE(valueInt64);
        ASSERT_TRUE(valueBool);
        ASSERT_TRUE(valueString);

        EXPECT_FLOAT_EQ(47.11f, *valueFloat);
        EXPECT_EQ(5, *valueInt32);
        EXPECT_EQ(7, *valueInt64);
        EXPECT_EQ(true, *valueBool);
        EXPECT_EQ("hello", *valueString);
    }

    TEST_F(AProperty, ReturnsValueIfItIsSetBeforehand_VectorTypes_Float)
    {
        Property avec2f(CreateInputProperty(EPropertyType::Vec2f));
        Property avec3f(CreateInputProperty(EPropertyType::Vec3f));
        Property avec4f(CreateInputProperty(EPropertyType::Vec4f));

        EXPECT_TRUE(avec2f.set<vec2f>({ 0.1f, 0.2f }));
        EXPECT_TRUE(avec3f.set<vec3f>({ 0.1f, 0.2f, 0.3f }));
        EXPECT_TRUE(avec4f.set<vec4f>({ 0.1f, 0.2f, 0.3f, 0.4f }));

        auto valueVec2f = avec2f.get<vec2f>();
        auto valueVec3f = avec3f.get<vec3f>();
        auto valueVec4f = avec4f.get<vec4f>();
        ASSERT_TRUE(valueVec2f);
        ASSERT_TRUE(valueVec3f);
        ASSERT_TRUE(valueVec4f);

        vec2f expectedValueVec2f {0.1f, 0.2f};
        vec3f expectedValueVec3f {0.1f, 0.2f, 0.3f};
        vec4f expectedValueVec4f {0.1f, 0.2f, 0.3f, 0.4f};
        EXPECT_TRUE(expectedValueVec2f == *valueVec2f);
        EXPECT_TRUE(expectedValueVec3f == *valueVec3f);
        EXPECT_TRUE(expectedValueVec4f == *valueVec4f);
    }

    TEST_F(AProperty, ReturnsValueIfItIsSetBeforehand_VectorTypes_Int)
    {
        Property avec2i(CreateInputProperty(EPropertyType::Vec2i));
        Property avec3i(CreateInputProperty(EPropertyType::Vec3i));
        Property avec4i(CreateInputProperty(EPropertyType::Vec4i));

        EXPECT_TRUE(avec2i.set<vec2i>({1, 2}));
        EXPECT_TRUE(avec3i.set<vec3i>({1, 2, 3}));
        EXPECT_TRUE(avec4i.set<vec4i>({1, 2, 3, 4}));

        auto valueVec2i = avec2i.get<vec2i>();
        auto valueVec3i = avec3i.get<vec3i>();
        auto valueVec4i = avec4i.get<vec4i>();
        ASSERT_TRUE(valueVec2i);
        ASSERT_TRUE(valueVec3i);
        ASSERT_TRUE(valueVec4i);

        vec2i expectedValueVec2i{1, 2};
        vec3i expectedValueVec3i{1, 2, 3};
        vec4i expectedValueVec4i{1, 2, 3, 4};
        EXPECT_TRUE(expectedValueVec2i == *valueVec2i);
        EXPECT_TRUE(expectedValueVec3i == *valueVec3i);
        EXPECT_TRUE(expectedValueVec4i == *valueVec4i);
    }

    TEST_F(AProperty, IsInitializedAsInputOrOutput)
    {
        Property inputProperty(CreateInputProperty(EPropertyType::Float));
        Property outputProperty(CreateOutputProperty(EPropertyType::Int32));

        EXPECT_TRUE(inputProperty.m_impl->isInput());
        EXPECT_FALSE(inputProperty.m_impl->isOutput());
        EXPECT_EQ(internal::EPropertySemantics::ScriptInput, inputProperty.m_impl->getPropertySemantics());
        EXPECT_TRUE(outputProperty.m_impl->isOutput());
        EXPECT_FALSE(outputProperty.m_impl->isInput());
        EXPECT_EQ(internal::EPropertySemantics::ScriptOutput, outputProperty.m_impl->getPropertySemantics());
    }

    TEST_F(AProperty, CannotSetOutputManually)
    {
        Property outputProperty(CreateOutputProperty(EPropertyType::Int32));

        EXPECT_TRUE(outputProperty.m_impl->isOutput());
        EXPECT_EQ(internal::EPropertySemantics::ScriptOutput, outputProperty.m_impl->getPropertySemantics());

        EXPECT_FALSE(outputProperty.set<int32_t>(45));
    }

    TEST_F(AProperty, ReturnsNoValueWhenAccessingWithWrongType)
    {
        Property floatProp(CreateInputProperty(EPropertyType::Float));
        Property vec2fProp(CreateInputProperty(EPropertyType::Vec2f));
        Property vec3fProp(CreateInputProperty(EPropertyType::Vec3f));
        Property vec4fProp(CreateInputProperty(EPropertyType::Vec4f));
        Property int32Prop(CreateInputProperty(EPropertyType::Int32));
        Property int64Prop(CreateInputProperty(EPropertyType::Int64));
        Property vec2iProp(CreateInputProperty(EPropertyType::Vec2i));
        Property vec3iProp(CreateInputProperty(EPropertyType::Vec3i));
        Property vec4iProp(CreateInputProperty(EPropertyType::Vec4i));
        Property boolProp(CreateInputProperty(EPropertyType::Bool));
        Property stringProp(CreateInputProperty(EPropertyType::String));
        Property structProp(CreateInputProperty(EPropertyType::Struct));
        Property arrayProp(CreateInputProperty(EPropertyType::Array));

        // Floats
        EXPECT_TRUE(floatProp.get<float>());
        EXPECT_FALSE(floatProp.get<int32_t>());

        EXPECT_TRUE(vec2fProp.get<vec2f>());
        EXPECT_FALSE(vec2fProp.get<vec2i>());

        EXPECT_TRUE(vec3fProp.get<vec3f>());
        EXPECT_FALSE(vec3fProp.get<vec3i>());

        EXPECT_TRUE(vec4fProp.get<vec4f>());
        EXPECT_FALSE(vec4fProp.get<vec4i>());

        // Integers
        EXPECT_TRUE(int32Prop.get<int32_t>());
        EXPECT_FALSE(int32Prop.get<int64_t>());
        EXPECT_FALSE(int32Prop.get<float>());

        EXPECT_TRUE(int64Prop.get<int64_t>());
        EXPECT_FALSE(int64Prop.get<float>());
        EXPECT_FALSE(int64Prop.get<int32_t>());

        EXPECT_TRUE(vec2iProp.get<vec2i>());
        EXPECT_FALSE(vec2iProp.get<vec2f>());

        EXPECT_TRUE(vec3iProp.get<vec3i>());
        EXPECT_FALSE(vec3iProp.get<vec3f>());

        EXPECT_TRUE(vec4iProp.get<vec4i>());
        EXPECT_FALSE(vec4iProp.get<vec4f>());

        // Others
        EXPECT_TRUE(boolProp.get<bool>());
        EXPECT_FALSE(boolProp.get<int32_t>());

        EXPECT_TRUE(stringProp.get<std::string>());
        EXPECT_FALSE(stringProp.get<bool>());

        // Complex types never have value
        EXPECT_FALSE(structProp.get<std::string>());
        EXPECT_FALSE(structProp.get<bool>());

        EXPECT_FALSE(arrayProp.get<int32_t>());
        EXPECT_FALSE(arrayProp.get<vec2f>());
    }

    TEST_F(AProperty, ReturnsNullptrForGetChildByIndexIfPropertyHasNoChildren)
    {
        Property property_float(CreateInputProperty(EPropertyType::Float));

        EXPECT_EQ(nullptr, property_float.getChild(0));
    }

    TEST_F(AProperty, ReturnsNullptrForGetChildByNameIfPropertyHasNoChildren)
    {
        Property property_float(CreateInputProperty(EPropertyType::Float));

        EXPECT_EQ(nullptr, property_float.getChild("child"));
    }

    TEST_F(AProperty, CanBeEmptyAndConst)
    {
        auto           rootImpl = CreateInputProperty(EPropertyType::Struct);
        const Property root(std::move(rootImpl));

        const auto child = root.getChild(0);
        EXPECT_EQ(nullptr, child);
    }

    TEST_F(AProperty, CanHaveNestedProperties)
    {
        Property root(CreateProperty(MakeStruct("", {{"child1", EPropertyType::Int32}, {"child2", EPropertyType::Float}}), EPropertySemantics::ScriptInput, false));

        ASSERT_EQ(2u, root.getChildCount());

        auto c1 = root.getChild(0);
        auto c2 = root.getChild(1);

        EXPECT_EQ("child1", c1->getName());
        EXPECT_EQ("child2", c2->getName());

        const auto& const_root = root;
        auto        c3         = const_root.getChild(0);
        auto        c4         = const_root.getChild(1);

        EXPECT_EQ("child1", c3->getName());
        EXPECT_EQ("child2", c4->getName());
    }

    TEST_F(AProperty, SetsValueIfTheTypeMatches)
    {
        Property floatProperty(CreateInputProperty(EPropertyType::Float));
        Property int32Property(CreateInputProperty(EPropertyType::Int32));
        Property int64Property(CreateInputProperty(EPropertyType::Int64));
        Property stringProperty(CreateInputProperty(EPropertyType::String));
        Property boolProperty(CreateInputProperty(EPropertyType::Bool));

        EXPECT_TRUE(floatProperty.set<float>(47.11f));
        EXPECT_TRUE(int32Property.set<int32_t>(4711));
        EXPECT_TRUE(int64Property.set<int64_t>(4711111));
        EXPECT_TRUE(stringProperty.set<std::string>("4711"));
        EXPECT_TRUE(boolProperty.set<bool>(true));

        const auto floatValue  = floatProperty.get<float>();
        const auto int32Value  = int32Property.get<int32_t>();
        const auto int64Value  = int64Property.get<int64_t>();
        const auto stringValue = stringProperty.get<std::string>();
        const auto boolValue   = boolProperty.get<bool>();

        ASSERT_TRUE(floatValue);
        ASSERT_TRUE(int32Value);
        ASSERT_TRUE(int64Value);
        ASSERT_TRUE(stringValue);
        ASSERT_TRUE(boolValue);

        ASSERT_EQ(47.11f, *floatValue);
        ASSERT_EQ(4711, *int32Value);
        ASSERT_EQ(4711111, *int64Value);
        ASSERT_EQ("4711", *stringValue);
        ASSERT_EQ(true, *boolValue);
    }

    TEST_F(AProperty, DoesNotSetValueIfTheTypeDoesNotMatch)
    {
        Property floatProperty(CreateInputProperty(EPropertyType::Float));
        Property int32Property(CreateInputProperty(EPropertyType::Int32));
        Property int64Property(CreateInputProperty(EPropertyType::Int64));
        Property stringProperty(CreateInputProperty(EPropertyType::String));
        Property boolProperty(CreateInputProperty(EPropertyType::Bool));

        EXPECT_FALSE(floatProperty.set<int32_t>(4711));
        EXPECT_FALSE(int32Property.set<float>(47.11f));
        EXPECT_FALSE(int64Property.set<int32_t>(47));
        EXPECT_FALSE(stringProperty.set<bool>(true));
        EXPECT_FALSE(boolProperty.set<std::string>("4711"));
        EXPECT_FALSE(floatProperty.set<vec2f>({0.1f, 0.2f}));

        const auto floatValue  = floatProperty.get<float>();
        const auto int32Value  = int32Property.get<int32_t>();
        const auto int64Value  = int64Property.get<int64_t>();
        const auto stringValue = stringProperty.get<std::string>();
        const auto boolValue   = boolProperty.get<bool>();

        EXPECT_TRUE(floatValue);
        EXPECT_TRUE(int32Value);
        EXPECT_TRUE(int64Value);
        EXPECT_TRUE(stringValue);
        EXPECT_TRUE(boolValue);
        EXPECT_EQ(0.0f, floatValue);
        EXPECT_EQ(0, int32Value);
        EXPECT_EQ(0, int64Value);
        EXPECT_EQ("", stringValue);
        EXPECT_EQ(false, boolValue);
    }

    TEST_F(AProperty, ReturnsChildByName)
    {
        Property root(CreateProperty(MakeStruct("", { {"child1", EPropertyType::Int32}, {"child2", EPropertyType::Float} }), EPropertySemantics::ScriptInput, true));

        const Property* c1 = root.getChild("child1");
        EXPECT_EQ("child1", c1->getName());

        const Property* c2 = root.getChild("child2");
        EXPECT_EQ("child2", c2->getName());

        const Property* c3 = root.getChild("does_not_exist");
        EXPECT_FALSE(c3);

        const Property& rootConst = root;
        c1 = rootConst.getChild("child1");
        c2 = rootConst.getChild("child2");
        EXPECT_EQ("child1", c1->getName());
        EXPECT_EQ("child2", c2->getName());
    }

    TEST_F(AProperty, ReturnsConstChildByName)
    {
        Property root(CreateProperty(MakeStruct("", { {"child1", EPropertyType::Int32}, {"child2", EPropertyType::Float} }), EPropertySemantics::ScriptInput, true));

        const Property* c1 = root.getChild("child1");
        EXPECT_EQ("child1", c1->getName());

        const Property* c2 = root.getChild("child1");
        EXPECT_EQ("child1", c2->getName());

        const Property* c3 = root.getChild("does_not_exist");
        EXPECT_FALSE(c3);
    }

    class AProperty_SerializationLifecycle : public AProperty
    {
    protected:
        ErrorReporting m_errorReporting;
        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
    };

    TEST_F(AProperty_SerializationLifecycle, StructWithoutChildren)
    {
        {
            PropertyImpl structNoChildren(MakeType("noChildren", EPropertyType::Struct), EPropertySemantics::ScriptInput);
            (void)PropertyImpl::Serialize(structNoChildren, m_flatBufferBuilder, m_serializationMap);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_EQ(serialized.name()->string_view(), "noChildren");
        EXPECT_EQ(serialized.children()->size(), 0u);

        {
            std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);
            ASSERT_EQ(0u, deserialized->getChildCount());
            EXPECT_EQ(EPropertyType::Struct, deserialized->getType());
            EXPECT_EQ("noChildren", deserialized->getName());
            EXPECT_EQ(EPropertySemantics::ScriptInput, deserialized->getPropertySemantics());
        }
    }

    TEST_F(AProperty_SerializationLifecycle, KeepsPropertyOrder)
    {
        {
            auto structType = MakeStruct("parent",
                {
                    TypeData{"child0", EPropertyType::Float},
                    TypeData{"child1", EPropertyType::Float},
                    TypeData{"child2", EPropertyType::Float}
                });
            PropertyImpl structProperty(structType, EPropertySemantics::ScriptInput);
            (void)PropertyImpl::Serialize(structProperty, m_flatBufferBuilder, m_serializationMap);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        ASSERT_EQ(3u, deserialized->getChildCount());
        EXPECT_EQ(EPropertyType::Struct, deserialized->getType());

        EXPECT_EQ("child0", deserialized->getChild(0)->getName());
        EXPECT_EQ("child1", deserialized->getChild(1)->getName());
        EXPECT_EQ("child2", deserialized->getChild(2)->getName());
    }

    TEST_F(AProperty_SerializationLifecycle, MultiLevelNesting)
    {
        {
            const HierarchicalTypeData rootType(TypeData{"root", EPropertyType::Struct},
                {
                    HierarchicalTypeData(TypeData{"nested", EPropertyType::Struct},
                        {
                            MakeType("float", EPropertyType::Float),
                            MakeStruct("nested", {TypeData("float", EPropertyType::Float)})
                        })
                }
            );

            PropertyImpl root(rootType, EPropertySemantics::ScriptInput);
            (void)PropertyImpl::Serialize(root, m_flatBufferBuilder, m_serializationMap);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        ASSERT_EQ(1u, deserialized->getChildCount());
        EXPECT_EQ(EPropertyType::Struct, deserialized->getType());

        auto propertyNested1 = deserialized->getChild(0u);
        EXPECT_EQ(EPropertyType::Struct, propertyNested1->getType());
        EXPECT_EQ("nested", propertyNested1->getName());

        ASSERT_EQ(2u, propertyNested1->getChildCount());
        auto propertyFloat1 = propertyNested1->getChild(0u);
        auto propertyNested2 = propertyNested1->getChild(1u);

        EXPECT_EQ(EPropertyType::Float, propertyFloat1->getType());
        EXPECT_EQ("float", propertyFloat1->getName());
        EXPECT_EQ(EPropertyType::Struct, propertyNested2->getType());
        EXPECT_EQ("nested", propertyNested2->getName());

        ASSERT_EQ(1u, propertyNested2->getChildCount());
        auto propertyFloat2 = propertyNested2->getChild(0u);

        EXPECT_EQ(EPropertyType::Float, propertyFloat2->getType());
        EXPECT_EQ("float", propertyFloat2->getName());
    }

    // Making this test templated makes it a lot harder to read, better leave it so - simple, stupid
    TEST_F(AProperty_SerializationLifecycle, AllSupportedPropertyTypes)
    {
        {
            auto rootImpl = CreateProperty(MakeStruct("root",
                {
                    {"Int32", EPropertyType::Int32},
                    {"Int64", EPropertyType::Int64},
                    {"Float", EPropertyType::Float},
                    {"Bool", EPropertyType::Bool},
                    {"String", EPropertyType::String},
                    {"Vec2f", EPropertyType::Vec2f},
                    {"Vec3f", EPropertyType::Vec3f},
                    {"Vec4f", EPropertyType::Vec4f},
                    {"Vec2i", EPropertyType::Vec2i},
                    {"Vec3i", EPropertyType::Vec3i},
                    {"Vec4i", EPropertyType::Vec4i},
                    {"DefaultValue", EPropertyType::Vec4i},
                }), EPropertySemantics::ScriptInput, true);

            auto propInt32  = rootImpl->getChild("Int32");
            auto propInt64  = rootImpl->getChild("Int64");
            auto propFloat  = rootImpl->getChild("Float");
            auto propBool   = rootImpl->getChild("Bool");
            auto propString = rootImpl->getChild("String");
            auto propVec2f  = rootImpl->getChild("Vec2f");
            auto propVec3f  = rootImpl->getChild("Vec3f");
            auto propVec4f  = rootImpl->getChild("Vec4f");
            auto propVec2i  = rootImpl->getChild("Vec2i");
            auto propVec3i  = rootImpl->getChild("Vec3i");
            auto propVec4i  = rootImpl->getChild("Vec4i");

            propInt32->set(4711);
            propInt64->set<int64_t>(4711111);
            propFloat->set(47.11f);
            propBool->set(true);
            propString->set<std::string>("4711");
            propVec2f->set<vec2f>({0.1f, 0.2f});
            propVec3f->set<vec3f>({1.1f, 1.2f, 1.3f});
            propVec4f->set<vec4f>({2.1f, 2.2f, 2.3f, 2.4f});
            propVec2i->set<vec2i>({1, 2});
            propVec3i->set<vec3i>({3, 4, 5});
            propVec4i->set<vec4i>({6, 7, 8, 9});

            (void)PropertyImpl::Serialize(*rootImpl, m_flatBufferBuilder, m_serializationMap);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        ASSERT_EQ(12u, deserialized->getChildCount());
        EXPECT_EQ(EPropertyType::Struct, deserialized->getType());

        const auto propInt32 = deserialized->getChild(0);
        const auto propInt64 = deserialized->getChild(1);
        const auto propFloat = deserialized->getChild(2);
        const auto propBool = deserialized->getChild(3);
        const auto propString = deserialized->getChild(4);
        const auto propVec2f = deserialized->getChild(5);
        const auto propVec3f = deserialized->getChild(6);
        const auto propVec4f = deserialized->getChild(7);
        const auto propVec2i = deserialized->getChild(8);
        const auto propVec3i = deserialized->getChild(9);
        const auto propVec4i = deserialized->getChild(10);
        const auto propDefValue = deserialized->getChild(11);

        EXPECT_EQ("Int32", propInt32->getName());
        EXPECT_EQ("Int64", propInt64->getName());
        EXPECT_EQ("Float", propFloat->getName());
        EXPECT_EQ("Bool", propBool->getName());
        EXPECT_EQ("String", propString->getName());
        EXPECT_EQ("Vec2f", propVec2f->getName());
        EXPECT_EQ("Vec3f", propVec3f->getName());
        EXPECT_EQ("Vec4f", propVec4f->getName());
        EXPECT_EQ("Vec2i", propVec2i->getName());
        EXPECT_EQ("Vec3i", propVec3i->getName());
        EXPECT_EQ("Vec4i", propVec4i->getName());
        EXPECT_EQ("DefaultValue", propDefValue->getName());

        const vec2f expectedValueVec2f{0.1f, 0.2f};
        const vec3f expectedValueVec3f{1.1f, 1.2f, 1.3f};
        const vec4f expectedValueVec4f{2.1f, 2.2f, 2.3f, 2.4f};
        const vec2i expectedValueVec2i{1, 2};
        const vec3i expectedValueVec3i{3, 4, 5};
        const vec4i expectedValueVec4i{6, 7, 8, 9};
        EXPECT_EQ(4711, *propInt32->get<int32_t>());
        EXPECT_EQ(4711111L, *propInt64->get<int64_t>());
        EXPECT_FLOAT_EQ(47.11f, *propFloat->get<float>());
        EXPECT_TRUE(*propBool->get<bool>());
        EXPECT_EQ("4711", *propString->get<std::string>());
        EXPECT_EQ(expectedValueVec2f, *propVec2f->get<vec2f>());
        EXPECT_EQ(expectedValueVec3f, *propVec3f->get<vec3f>());
        EXPECT_EQ(expectedValueVec4f, *propVec4f->get<vec4f>());
        EXPECT_EQ(expectedValueVec2i, *propVec2i->get<vec2i>());
        EXPECT_EQ(expectedValueVec3i, *propVec3i->get<vec3i>());
        EXPECT_EQ(expectedValueVec4i, *propVec4i->get<vec4i>());
        EXPECT_FALSE(propDefValue->m_impl->checkForBindingInputNewValueAndReset());
    }

    TEST_F(AProperty_SerializationLifecycle, ErrorWhenNameMissing)
    {
        {
            auto propertyOffset = rlogic_serialization::CreateProperty(
                m_flatBufferBuilder,
                0
            );
            m_flatBufferBuilder.Finish(propertyOffset);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_F(AProperty_SerializationLifecycle, ErrorWhenTypeCorrupted)
    {
        {
            // Simulate bad things with enums, but this can happen with corrupted binary data and we need to handle it safely nevertheless
            auto invalidType = static_cast<rlogic_serialization::EPropertyRootType>(std::numeric_limits<uint8_t>::max());
            auto propertyOffset = rlogic_serialization::CreateProperty(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("name"),
                invalidType
            );
            m_flatBufferBuilder.Finish(propertyOffset);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: invalid type!");
    }

    TEST_F(AProperty_SerializationLifecycle, ErrorWhenChildHasErrors)
    {
        {
            // Child is invalid because it has no name
            auto childOffset = rlogic_serialization::CreateProperty(
                m_flatBufferBuilder,
                0
            );
            // Parent is fine, but references a corrupt child property
            auto propertyOffset = rlogic_serialization::CreateProperty(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("name"),
                rlogic_serialization::EPropertyRootType::Struct,
                m_flatBufferBuilder.CreateVector(std::vector{childOffset})
            );
            m_flatBufferBuilder.Finish(propertyOffset);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_F(AProperty_SerializationLifecycle, ErrorWhenComplexTypeHasNoChildInfo)
    {
        {
            // Parent is fine, but references a corrupt child property
            auto propertyOffset = rlogic_serialization::CreateProperty(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("name"),
                rlogic_serialization::EPropertyRootType::Struct,
                0
            );
            m_flatBufferBuilder.Finish(propertyOffset);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: complex type has no child type info!");
    }

    TEST_F(AProperty_SerializationLifecycle, ErrorWhenMissingValueForPrimitiveType_AllUnionTypes)
    {
        // A bit ugly, but simple way to iterate all enum values
        for (rlogic_serialization::PropertyValue valueType = rlogic_serialization::PropertyValue::float_s; valueType <= rlogic_serialization::PropertyValue::MAX; valueType = rlogic_serialization::PropertyValue(uint32_t(valueType) + 1))
        {
            {
                auto propertyOffset = rlogic_serialization::CreateProperty(
                    m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    rlogic_serialization::EPropertyRootType::Primitive,
                    0,
                    valueType,
                    0 // no union value type provided -> error when deserialized
                );
                m_flatBufferBuilder.Finish(propertyOffset);
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
            m_errorReporting.clear();
            std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

            EXPECT_FALSE(deserialized);
            ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
            EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: invalid union!");
        }
    }

    // String requires individual test, other types are tested below in AProperty_SerializationLifecycle_T
    TEST_F(AProperty_SerializationLifecycle, ErrorWhenPrimitiveValueIsCorrupt_String)
    {
        {
            auto propertyOffset = rlogic_serialization::CreateProperty(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("name"),
                rlogic_serialization::EPropertyRootType::Primitive,
                0,
                rlogic_serialization::PropertyValue::NONE, // setting NONE here makes the enum tuple invalid and would trigger seg fault if not checked
                m_flatBufferBuilder.CreateStruct(m_flatBufferBuilder.CreateString("test string")).Union()
            );
            m_flatBufferBuilder.Finish(propertyOffset);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: invalid type!");
    }

    // two element type
    template <typename T>
    class AProperty_SerializationLifecycle_T : public AProperty_SerializationLifecycle
    {
    };
    // The list of types we want to test.
    using ValueTypes = ::testing::Types <
        rlogic_serialization::float_s,
        rlogic_serialization::vec2f_s,
        rlogic_serialization::vec3f_s,
        rlogic_serialization::vec4f_s,
        rlogic_serialization::int32_s,
        rlogic_serialization::vec2i_s,
        rlogic_serialization::vec3i_s,
        rlogic_serialization::vec4i_s,
        rlogic_serialization::bool_s
        //string_s requires a separate test because it's not a struct but a table, see below
    >;

    TYPED_TEST_SUITE(AProperty_SerializationLifecycle_T, ValueTypes);

    TYPED_TEST(AProperty_SerializationLifecycle_T, ErrorWhenPrimitiveValueIsCorrupt)
    {
        {
            const TypeParam unionValue; // data doesn't matter, just use default constructor for simplicity
            auto propertyOffset = rlogic_serialization::CreateProperty(
                this->m_flatBufferBuilder,
                this->m_flatBufferBuilder.CreateString("name"),
                rlogic_serialization::EPropertyRootType::Primitive,
                0,
                rlogic_serialization::PropertyValue::NONE, // setting NONE here makes the enum tuple invalid and would trigger seg fault if not checked
                this->m_flatBufferBuilder.CreateStruct(unionValue).Union()
            );
            this->m_flatBufferBuilder.Finish(propertyOffset);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::Property>(this->m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<PropertyImpl> deserialized = PropertyImpl::Deserialize(serialized, EPropertySemantics::ScriptInput, this->m_errorReporting, this->m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(this->m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(this->m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: invalid type!");
    }

    TEST_F(AProperty, DoesNotSetLogicNodeToDirtyIfValueIsNotChanged)
    {
        auto int32Property  = CreateInputProperty(EPropertyType::Int32);
        auto int64Property  = CreateInputProperty(EPropertyType::Int64);
        auto floatProperty  = CreateInputProperty(EPropertyType::Float);
        auto vec2fProperty  = CreateInputProperty(EPropertyType::Vec2f);
        auto vec3iProperty  = CreateInputProperty(EPropertyType::Vec3i);
        auto stringProperty = CreateInputProperty(EPropertyType::String);

        int32Property->setValue(42);
        int64Property->setValue(int64_t{ 421 });
        floatProperty->setValue(42.f);
        vec2fProperty->setValue(vec2f{4.f, 2.f});
        vec3iProperty->setValue(vec3i{4, 2, 3});
        stringProperty->setValue(std::string("42"));

        int32Property->getLogicNode().setDirty(false);
        int64Property->getLogicNode().setDirty(false);
        floatProperty->getLogicNode().setDirty(false);
        vec2fProperty->getLogicNode().setDirty(false);
        vec3iProperty->getLogicNode().setDirty(false);
        stringProperty->getLogicNode().setDirty(false);

        int32Property->setValue(42);
        int64Property->setValue(int64_t{ 421 });
        floatProperty->setValue(42.f);
        vec2fProperty->setValue(vec2f{ 4.f, 2.f });
        vec3iProperty->setValue(vec3i{ 4, 2, 3 });
        stringProperty->setValue(std::string("42"));

        EXPECT_FALSE(int32Property->getLogicNode().isDirty());
        EXPECT_FALSE(int64Property->getLogicNode().isDirty());
        EXPECT_FALSE(floatProperty->getLogicNode().isDirty());
        EXPECT_FALSE(vec2fProperty->getLogicNode().isDirty());
        EXPECT_FALSE(vec3iProperty->getLogicNode().isDirty());
        EXPECT_FALSE(stringProperty->getLogicNode().isDirty());
    }

    TEST_F(AProperty, SetsLogicNodeToDirtyIfValueIsChanged)
    {
        auto int32Property  = CreateInputProperty(EPropertyType::Int32);
        auto int64Property  = CreateInputProperty(EPropertyType::Int64);
        auto floatProperty  = CreateInputProperty(EPropertyType::Float);
        auto vec2fProperty  = CreateInputProperty(EPropertyType::Vec2f);
        auto vec3iProperty  = CreateInputProperty(EPropertyType::Vec3i);
        auto stringProperty = CreateInputProperty(EPropertyType::String);

        int32Property->setValue(42);
        int64Property->setValue(int64_t{ 421 });
        floatProperty->setValue(42.f);
        vec2fProperty->setValue(vec2f{4.f, 2.f});
        vec3iProperty->setValue(vec3i{4, 2, 3});
        stringProperty->setValue(std::string("42"));

        EXPECT_TRUE(int32Property->getLogicNode().isDirty());
        EXPECT_TRUE(int64Property->getLogicNode().isDirty());
        EXPECT_TRUE(floatProperty->getLogicNode().isDirty());
        EXPECT_TRUE(vec2fProperty->getLogicNode().isDirty());
        EXPECT_TRUE(vec3iProperty->getLogicNode().isDirty());
        EXPECT_TRUE(stringProperty->getLogicNode().isDirty());
    }

    TEST_F(AProperty, FailsToSetINT64ValueThatCannotBeRepresentedInLua)
    {
        Property int64Property{ CreateProperty(MakeType("int64input", EPropertyType::Int64), EPropertySemantics::ScriptInput, true) };

        ELogMessageType logType{ ELogMessageType::Info };
        std::string logMessage;
        ScopedLogContextLevel logCollector{ ELogMessageType::Error, [&](ELogMessageType type, std::string_view message)
        {
            logType = type;
            logMessage = message;
        }
        };

        EXPECT_FALSE(int64Property.set(std::numeric_limits<int64_t>::max()));
        EXPECT_EQ(logType, ELogMessageType::Error);
        EXPECT_EQ(logMessage, fmt::format("Invalid value when setting property 'int64input', Lua cannot handle full range of 64-bit integer, trying to set '{}' which is out of this range!",
            std::numeric_limits<int64_t>::max()).c_str());

        EXPECT_FALSE(int64Property.set(std::numeric_limits<int64_t>::min()));
        EXPECT_EQ(logType, ELogMessageType::Error);
        EXPECT_EQ(logMessage, fmt::format("Invalid value when setting property 'int64input', Lua cannot handle full range of 64-bit integer, trying to set '{}' which is out of this range!",
            std::numeric_limits<int64_t>::min()).c_str());

        static constexpr auto maxIntegerAsDouble = static_cast<int64_t>(1LLU << 53u);
        EXPECT_TRUE(int64Property.set(maxIntegerAsDouble));
        EXPECT_FALSE(int64Property.set(maxIntegerAsDouble + 1));
        EXPECT_EQ(logType, ELogMessageType::Error);
        EXPECT_EQ(logMessage, fmt::format("Invalid value when setting property 'int64input', Lua cannot handle full range of 64-bit integer, trying to set '{}' which is out of this range!",
            maxIntegerAsDouble + 1).c_str());

        EXPECT_TRUE(int64Property.set(-maxIntegerAsDouble));
        EXPECT_FALSE(int64Property.set(-maxIntegerAsDouble - 1));
        EXPECT_EQ(logType, ELogMessageType::Error);
        EXPECT_EQ(logMessage, fmt::format("Invalid value when setting property 'int64input', Lua cannot handle full range of 64-bit integer, trying to set '{}' which is out of this range!",
            -maxIntegerAsDouble - 1).c_str());
    }
}
