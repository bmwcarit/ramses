//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LuaScriptTest_Base.h"

#include "impl/PropertyImpl.h"
#include "ramses-logic/Property.h"
#include "internals/WrappedLuaProperty.h"
#include "LogTestUtils.h"

#include "fmt/format.h"

namespace rlogic::internal
{
    class AWrappedLuaProperty : public ::testing::Test
    {
    protected:
        AWrappedLuaProperty()
        {
            WrappedLuaProperty::RegisterTypes(m_sol);
        }

        PropertyImpl makeTestProperty(HierarchicalTypeData typeInfo, EPropertySemantics semantics = EPropertySemantics::ScriptOutput)
        {
            PropertyImpl prop(std::move(typeInfo), semantics);
            setDummyDataRecursively(prop);
            return prop;
        }

        void setDummyDataRecursively(PropertyImpl& prop)
        {
            switch (prop.getType())
            {
            case EPropertyType::Float:
                prop.setValue(0.5f);
                break;
            case EPropertyType::Int32:
                prop.setValue(42);
                break;
            case EPropertyType::Int64:
                prop.setValue(int64_t{ 421 });
                break;
            case EPropertyType::String:
                prop.setValue(std::string("hello"));
                break;
            case EPropertyType::Bool:
                prop.setValue(false);
                break;
            case EPropertyType::Vec2f:
                prop.setValue(vec2f{ 0.1f, 0.2f });
                break;
            case EPropertyType::Vec3f:
                prop.setValue(vec3f{ 1.1f, 1.2f, 1.3f });
                break;
            case EPropertyType::Vec4f:
                prop.setValue(vec4f{ 2.1f, 2.2f, 2.3f, 1500000.4f });
                break;
            case EPropertyType::Vec2i:
                prop.setValue(vec2i{ 11, -12 });
                break;
            case EPropertyType::Vec3i:
                prop.setValue(vec3i{ 11, 12, 13 });
                break;
            case EPropertyType::Vec4i:
                prop.setValue(vec4i{ 11, 12, 13, 14000000 });
                break;
            case EPropertyType::Array:
            case EPropertyType::Struct:
                for (size_t i = 0; i < prop.getChildCount(); ++i)
                {
                    setDummyDataRecursively(*prop.getChild(i)->m_impl);
                }
                break;
            }
        }

        sol::protected_function_result  run_WithResult(std::string_view source)
        {
            // Reference temporary extractor
            sol::protected_function loaded = m_sol.load(source);
            assert(loaded.valid());
            return loaded();
        }

        HierarchicalTypeData m_structWithAllPrimitiveTypes = MakeStruct("ROOT", {
            TypeData{"Float", EPropertyType::Float},
            TypeData{"Vec2f", EPropertyType::Vec2f},
            TypeData{"Vec3f", EPropertyType::Vec3f},
            TypeData{"Vec4f", EPropertyType::Vec4f},
            TypeData{"Int32", EPropertyType::Int32},
            TypeData{"Int64", EPropertyType::Int64},
            TypeData{"Vec2i", EPropertyType::Vec2i},
            TypeData{"Vec3i", EPropertyType::Vec3i},
            TypeData{"Vec4i", EPropertyType::Vec4i},
            TypeData{"String", EPropertyType::String},
            TypeData{"Bool", EPropertyType::Bool}});

        sol::state m_sol;
        ScopedLogContextLevel m_silenceLogs = ScopedLogContextLevel{ELogMessageType::Off};
    };

    class AWrappedLuaProperty_Access : public AWrappedLuaProperty
    {
    protected:
        template <typename T>
        T extractValue(std::string_view statement)
        {
            const std::string assignment = fmt::format("value = {}", statement);
            run_WithResult(assignment);
            T resultValue = m_sol["value"];
            return resultValue;
        }
    };

    TEST_F(AWrappedLuaProperty_Access, WrapsProperty_ByReference)
    {
        PropertyImpl inputInt = makeTestProperty(MakeStruct("ROOT", {TypeData{"int", EPropertyType::Int32}}));
        WrappedLuaProperty wrapped(inputInt);
        m_sol["ROOT"] = std::ref(wrapped);

        WrappedLuaProperty& wrappedRef = m_sol["ROOT"];
        EXPECT_EQ(&wrappedRef, &wrapped);
        const WrappedLuaProperty& wrappedRefConst = m_sol["ROOT"];
        EXPECT_EQ(&wrappedRefConst, &wrapped);
    }

    TEST_F(AWrappedLuaProperty_Access, ExposesChildPropertiesAsWrappedObjects)
    {
        PropertyImpl nestedStruct(HierarchicalTypeData{ TypeData("Nested", EPropertyType::Struct), {m_structWithAllPrimitiveTypes} }, EPropertySemantics::ScriptInput);
        WrappedLuaProperty wrapped(nestedStruct);
        m_sol["Nested"] = std::ref(wrapped);

        // Use sol access overloads to check that type resolving works
        WrappedLuaProperty& innerStructRef = m_sol["Nested"]["ROOT"];
        // size() is the only externally exposed property of the wrapper, use it to verify the type resolving worked
        EXPECT_EQ(11u, innerStructRef.size());
    }

    TEST_F(AWrappedLuaProperty_Access, ResolvesPrimitiveTypes)
    {
        PropertyImpl inputAllPrimitives = makeTestProperty(m_structWithAllPrimitiveTypes, EPropertySemantics::ScriptInput);
        WrappedLuaProperty wrapped(inputAllPrimitives);
        m_sol["ROOT"] = std::ref(wrapped);

        EXPECT_FLOAT_EQ(0.5f, extractValue<float>("ROOT.Float"));
        EXPECT_FALSE(extractValue<bool>("ROOT.Bool"));
        EXPECT_EQ("hello", extractValue<std::string>("ROOT.String"));
        EXPECT_EQ(42, extractValue<int32_t>("ROOT.Int32"));
    }

    TEST_F(AWrappedLuaProperty_Access, ResolvesPrimitiveTypes_FloatVecTypes)
    {
        for (auto semantics: std::vector<EPropertySemantics>{EPropertySemantics::ScriptInput, EPropertySemantics::ScriptOutput})
        {
            PropertyImpl inputAllPrimitives = makeTestProperty(m_structWithAllPrimitiveTypes, semantics);
            WrappedLuaProperty wrapped(inputAllPrimitives);
            m_sol["ROOT"] = std::ref(wrapped);

            EXPECT_FLOAT_EQ(0.1f, extractValue<float>("ROOT.Vec2f[1]"));
            EXPECT_FLOAT_EQ(0.2f, extractValue<float>("ROOT.Vec2f[2]"));
            EXPECT_FLOAT_EQ(1.1f, extractValue<float>("ROOT.Vec3f[1]"));
            EXPECT_FLOAT_EQ(1.2f, extractValue<float>("ROOT.Vec3f[2]"));
            EXPECT_FLOAT_EQ(1.3f, extractValue<float>("ROOT.Vec3f[3]"));
            EXPECT_FLOAT_EQ(2.1f, extractValue<float>("ROOT.Vec4f[1]"));
            EXPECT_FLOAT_EQ(2.2f, extractValue<float>("ROOT.Vec4f[2]"));
            EXPECT_FLOAT_EQ(2.3f, extractValue<float>("ROOT.Vec4f[3]"));
            EXPECT_FLOAT_EQ(1500000.4f, extractValue<float>("ROOT.Vec4f[4]"));
        }
    }

    TEST_F(AWrappedLuaProperty_Access, ResolvesPrimitiveTypes_Int32VecTypes)
    {
        for (auto semantics : std::vector<EPropertySemantics>{ EPropertySemantics::ScriptInput, EPropertySemantics::ScriptOutput })
        {
            PropertyImpl inputAllPrimitives = makeTestProperty(m_structWithAllPrimitiveTypes, semantics);
            WrappedLuaProperty wrapped(inputAllPrimitives);
            m_sol["ROOT"] = std::ref(wrapped);

            EXPECT_EQ(11, extractValue<int32_t>("ROOT.Vec2i[1]"));
            EXPECT_EQ(-12, extractValue<int32_t>("ROOT.Vec2i[2]"));
            EXPECT_EQ(11, extractValue<int32_t>("ROOT.Vec3i[1]"));
            EXPECT_EQ(12, extractValue<int32_t>("ROOT.Vec3i[2]"));
            EXPECT_EQ(13, extractValue<int32_t>("ROOT.Vec3i[3]"));
            EXPECT_EQ(11, extractValue<int32_t>("ROOT.Vec4i[1]"));
            EXPECT_EQ(12, extractValue<int32_t>("ROOT.Vec4i[2]"));
            EXPECT_EQ(13, extractValue<int32_t>("ROOT.Vec4i[3]"));
            EXPECT_EQ(14000000, extractValue<int32_t>("ROOT.Vec4i[4]"));
        }
    }

    TEST_F(AWrappedLuaProperty_Access, ResolvesNestedStructFields)
    {
        for (auto semantics : std::vector<EPropertySemantics>{ EPropertySemantics::ScriptInput, EPropertySemantics::ScriptOutput })
        {
            PropertyImpl nestedStruct(HierarchicalTypeData{ TypeData("Nested", EPropertyType::Struct), {m_structWithAllPrimitiveTypes} }, semantics);
            setDummyDataRecursively(nestedStruct);
            WrappedLuaProperty wrapped(nestedStruct);
            m_sol["Nested"] = std::ref(wrapped);

            EXPECT_EQ(11, extractValue<int32_t>("Nested.ROOT.Vec2i[1]"));
            EXPECT_FLOAT_EQ(2.2f, extractValue<float>("Nested.ROOT.Vec4f[2]"));
        }
    }

    TEST_F(AWrappedLuaProperty_Access, ResolvesArrayElements)
    {
        for (auto semantics : std::vector<EPropertySemantics>{ EPropertySemantics::ScriptInput, EPropertySemantics::ScriptOutput })
        {
            PropertyImpl nestedArray(HierarchicalTypeData{ TypeData("Nested", EPropertyType::Struct), {MakeArray("array", 2, EPropertyType::Vec3f)} }, semantics);
            setDummyDataRecursively(nestedArray);
            WrappedLuaProperty wrapped(nestedArray);
            m_sol["Nested"] = std::ref(wrapped);

            //Set second element to a different value
            nestedArray.getChild("array")->getChild(1)->m_impl->setValue(vec3f{15, 16, 17});

            EXPECT_FLOAT_EQ(1.1f, extractValue<float>("Nested.array[1][1]"));
            EXPECT_FLOAT_EQ(1.2f, extractValue<float>("Nested.array[1][2]"));
            EXPECT_FLOAT_EQ(1.3f, extractValue<float>("Nested.array[1][3]"));
            EXPECT_FLOAT_EQ(15.f, extractValue<float>("Nested.array[2][1]"));
            EXPECT_FLOAT_EQ(16.f, extractValue<float>("Nested.array[2][2]"));
            EXPECT_FLOAT_EQ(17.f, extractValue<float>("Nested.array[2][3]"));
        }
    }


    class AWrappedLuaProperty_Assignment : public AWrappedLuaProperty_Access
    {
    protected:
        template <typename T>
        T assignExpressionToValue(std::string_view expression, std::string_view result)
        {
            run_WithResult(fmt::format("tempValue = {}", expression));
            run_WithResult(fmt::format("{} = tempValue", result));
            const T resultValue = m_sol["tempValue"];
            return resultValue;
        }
    };

    TEST_F(AWrappedLuaProperty_Assignment, AssignsPrimitiveFieldToWrappedOutputStruct)
    {
        PropertyImpl outputAllPrimitives = makeTestProperty(m_structWithAllPrimitiveTypes, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrapped(outputAllPrimitives);
        m_sol["ROOT"] = std::ref(wrapped);

        const sol::protected_function_result result = run_WithResult(R"(
            ROOT.Int = 12
        )");

        EXPECT_EQ(12, assignExpressionToValue<int32_t>("12", "ROOT.Int"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, CatchesErrorWhenTryingToAssignPrimitiveInputFields)
    {
        PropertyImpl input = makeTestProperty(MakeStruct("ROOT", {TypeData{"Int32", EPropertyType::Int32}}), EPropertySemantics::ScriptInput);
        WrappedLuaProperty wrapped(input);
        m_sol["ROOT"] = std::ref(wrapped);

        const sol::protected_function_result result = run_WithResult(R"(
            ROOT.Int32 = 12
        )");

        ASSERT_FALSE(result.valid());
        const sol::error err = result;
        EXPECT_THAT(err.what(), ::testing::HasSubstr("Error while writing to 'Int32'. Writing input values is not allowed, only outputs!"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, OfStructProperties)
    {
        PropertyImpl struct1(HierarchicalTypeData{ TypeData("S1", EPropertyType::Struct), {m_structWithAllPrimitiveTypes} }, EPropertySemantics::ScriptOutput);
        PropertyImpl struct2(HierarchicalTypeData{ TypeData("S2", EPropertyType::Struct), {m_structWithAllPrimitiveTypes} }, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrappedS1(struct1);
        WrappedLuaProperty wrappedS2(struct1);
        m_sol["S1"] = std::ref(wrappedS1);
        m_sol["S2"] = std::ref(wrappedS2);

        const sol::protected_function_result result = run_WithResult(R"(
            -- Assign from Lua table properties
            S1.ROOT = {
                Float = 0.5,
                Vec2f = {11.1, 11.2},
                Vec3f = {11.1, 11.2, 11.3},
                Vec4f = {11.1, 11.2, 11.3, 11.4},
                Int32 = 42,
                Int64 = 421,
                Vec2i = {21, 22},
                Vec3i = {31, 32, 33},
                Vec4i = {41, 42, 43, 44},
                String = "abc",
                Bool = true
            }
            -- Assign whole struct S1 -> to S2
            S2.ROOT = S1.ROOT
        )");

        ASSERT_TRUE(result.valid());

        EXPECT_FLOAT_EQ(0.5f, extractValue<float>("S2.ROOT.Float"));
        EXPECT_TRUE(extractValue<bool>("S2.ROOT.Bool"));
        EXPECT_EQ("abc", extractValue<std::string>("S2.ROOT.String"));
        EXPECT_EQ(42, extractValue<int32_t>("S2.ROOT.Int32"));
        EXPECT_EQ(421, extractValue<int64_t>("S2.ROOT.Int64"));

        EXPECT_EQ(21, extractValue<int32_t>("S2.ROOT.Vec2i[1]"));
        EXPECT_EQ(22, extractValue<int32_t>("S2.ROOT.Vec2i[2]"));
        EXPECT_EQ(31, extractValue<int32_t>("S2.ROOT.Vec3i[1]"));
        EXPECT_EQ(32, extractValue<int32_t>("S2.ROOT.Vec3i[2]"));
        EXPECT_EQ(33, extractValue<int32_t>("S2.ROOT.Vec3i[3]"));
        EXPECT_EQ(41, extractValue<int32_t>("S2.ROOT.Vec4i[1]"));
        EXPECT_EQ(42, extractValue<int32_t>("S2.ROOT.Vec4i[2]"));
        EXPECT_EQ(43, extractValue<int32_t>("S2.ROOT.Vec4i[3]"));
        EXPECT_EQ(44, extractValue<int32_t>("S2.ROOT.Vec4i[4]"));

        EXPECT_FLOAT_EQ(11.1f, extractValue<float>("S2.ROOT.Vec2f[1]"));
        EXPECT_FLOAT_EQ(11.2f, extractValue<float>("S2.ROOT.Vec2f[2]"));
        EXPECT_FLOAT_EQ(11.1f, extractValue<float>("S2.ROOT.Vec3f[1]"));
        EXPECT_FLOAT_EQ(11.2f, extractValue<float>("S2.ROOT.Vec3f[2]"));
        EXPECT_FLOAT_EQ(11.3f, extractValue<float>("S2.ROOT.Vec3f[3]"));
        EXPECT_FLOAT_EQ(11.1f, extractValue<float>("S2.ROOT.Vec4f[1]"));
        EXPECT_FLOAT_EQ(11.2f, extractValue<float>("S2.ROOT.Vec4f[2]"));
        EXPECT_FLOAT_EQ(11.3f, extractValue<float>("S2.ROOT.Vec4f[3]"));
        EXPECT_FLOAT_EQ(11.4f, extractValue<float>("S2.ROOT.Vec4f[4]"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, OfInvalidTypeToStructCausesError)
    {
        PropertyImpl aStruct(HierarchicalTypeData{ TypeData("S", EPropertyType::Struct), {m_structWithAllPrimitiveTypes} }, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrappedStruct(aStruct);
        m_sol["S"] = std::ref(wrappedStruct);

        const sol::protected_function_result result = run_WithResult(R"(
            S.ROOT = "this is not a table"
        )");

        ASSERT_FALSE(result.valid());
        const sol::error err = result;
        EXPECT_THAT(err.what(), ::testing::HasSubstr("Unexpected type (string) while assigning value of struct field 'ROOT' (expected a table or another struct)"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, OfArrays)
    {
        PropertyImpl array1(HierarchicalTypeData{ TypeData("A1", EPropertyType::Struct), {MakeArray("data", 3, EPropertyType::Float)} }, EPropertySemantics::ScriptOutput);
        PropertyImpl array2(HierarchicalTypeData{ TypeData("A2", EPropertyType::Struct), {MakeArray("data", 3, EPropertyType::Float)} }, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrappedA1(array1);
        WrappedLuaProperty wrappedA2(array2);
        m_sol["A1"] = std::ref(wrappedA1);
        m_sol["A2"] = std::ref(wrappedA2);

        const sol::protected_function_result result = run_WithResult(R"(
            -- Assignment from Lua table converts to array data
            A1.data = { 1.1, 1.2, 1.3 }
            -- Assign whole array A1 -> to A2
            A2.data = A1.data
        )");

        ASSERT_TRUE(result.valid());

        EXPECT_FLOAT_EQ(1.1f, extractValue<float>("A2.data[1]"));
        EXPECT_FLOAT_EQ(1.2f, extractValue<float>("A2.data[2]"));
        EXPECT_FLOAT_EQ(1.3f, extractValue<float>("A2.data[3]"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, OfInvalidTypeToArrayCausesError)
    {
        PropertyImpl array(HierarchicalTypeData{ TypeData("A", EPropertyType::Struct), {MakeArray("data", 3, EPropertyType::Float)} }, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrappedA(array);
        m_sol["A"] = std::ref(wrappedA);

        const sol::protected_function_result result = run_WithResult(R"(
            A.data = 5
        )");

        ASSERT_FALSE(result.valid());
        const sol::error err = result;
        EXPECT_THAT(err.what(), ::testing::HasSubstr("Unexpected type (number) while assigning value of array field 'data' (expected a table or another array)"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, OfInvalidTypeToPrimitiveCausesError)
    {
        for (auto fieldType : std::vector<EPropertyType>{ EPropertyType::Float, EPropertyType::Int32, EPropertyType::Int64, EPropertyType::String, EPropertyType::Bool })
        {
            PropertyImpl root(HierarchicalTypeData{ TypeData("A", EPropertyType::Struct), {MakeType("field", fieldType)} }, EPropertySemantics::ScriptOutput);
            WrappedLuaProperty wrapped(root);
            m_sol["A"] = std::ref(wrapped);

            const sol::protected_function_result result = run_WithResult(R"(
                A.field = {this = "is not a primitive type"}
            )");

            ASSERT_FALSE(result.valid());
            const sol::error err = result;
            EXPECT_THAT(err.what(), ::testing::HasSubstr(fmt::format("Assigning table to '{}' output 'field'", GetLuaPrimitiveTypeName(fieldType))));
        }
    }

    TEST_F(AWrappedLuaProperty_Assignment, OfVectorComponentsCausesError)
    {
        for (auto fieldType : std::vector<EPropertyType>{ EPropertyType::Vec2f, EPropertyType::Vec3f, EPropertyType::Vec4f, EPropertyType::Vec2i, EPropertyType::Vec3i, EPropertyType::Vec4i })
        {
            PropertyImpl root(HierarchicalTypeData{ TypeData("IN", EPropertyType::Struct), {MakeType("vec", fieldType)} }, EPropertySemantics::ScriptOutput);
            WrappedLuaProperty wrapped(root);
            m_sol["IN"] = std::ref(wrapped);

            const sol::protected_function_result result = run_WithResult(R"(
                IN.vec[1] = 1
            )");

            ASSERT_FALSE(result.valid());
            const sol::error err = result;
            EXPECT_THAT(err.what(), ::testing::HasSubstr(fmt::format("Error while writing to 'vec'. Can't assign individual components of vector types, must assign the whole vector")));
        }
    }

    // Can't be triggered from user code, this is purely internal unit test
    TEST_F(AWrappedLuaProperty_Assignment, OfUnexpectedUsertypeCausesError)
    {
        class AnotherUserType
        {
        } anotherUserObject;

        m_sol.new_usertype<AnotherUserType>("AnotherUserType");
        m_sol["anotherUserObject"] = std::ref(anotherUserObject);

        PropertyImpl root(HierarchicalTypeData{ TypeData("A", EPropertyType::Struct), {MakeType("field", EPropertyType::Float)} }, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrapped(root);
        m_sol["A"] = std::ref(wrapped);

        const sol::protected_function_result result = run_WithResult(R"(
            A.field = anotherUserObject
        )");

        ASSERT_FALSE(result.valid());
        const sol::error err = result;
        EXPECT_THAT(err.what(), ::testing::HasSubstr("Implementation error: Unexpected userdata"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, CatchesErrorWhenTryingToAssignStructInputFields)
    {
        PropertyImpl nestedStruct(HierarchicalTypeData{ TypeData("ROOT", EPropertyType::Struct), {m_structWithAllPrimitiveTypes} }, EPropertySemantics::ScriptInput);
        WrappedLuaProperty wrapped(nestedStruct);
        m_sol["ROOT"] = std::ref(wrapped);

        const sol::protected_function_result result = run_WithResult(R"(
            ROOT.ROOT = {Int32 = 5, Float = 4.4}
        )");

        ASSERT_FALSE(result.valid());
        const sol::error err = result;
        EXPECT_THAT(err.what(), ::testing::HasSubstr("Error while writing to 'ROOT'. Writing input values is not allowed, only outputs!"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, CatchesErrorWhenTryingToAssignArrayInputFields)
    {
        PropertyImpl nestedStruct(HierarchicalTypeData{ TypeData("ROOT", EPropertyType::Struct), {MakeArray("array", 2, EPropertyType::Float)}}, EPropertySemantics::ScriptInput);
        WrappedLuaProperty wrapped(nestedStruct);
        m_sol["ROOT"] = std::ref(wrapped);

        const sol::protected_function_result result = run_WithResult(R"(
            ROOT.array[1] = 5
        )");

        ASSERT_FALSE(result.valid());
        const sol::error err = result;
        EXPECT_THAT(err.what(), ::testing::HasSubstr("Error while writing to 'idx: 1'. Writing input values is not allowed, only outputs!"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, AppliesNumericChecks_StructFields)
    {
        PropertyImpl root(m_structWithAllPrimitiveTypes, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrapped(root);
        m_sol["ROOT"] = std::ref(wrapped);

        sol::error err = run_WithResult(R"(
            ROOT.Int32 = 1000000000000000000000000000000000000000
        )");

        EXPECT_THAT(err.what(),
            ::testing::HasSubstr("Error during assignment of property 'Int32'! Error while extracting integer: integral part too large to fit in a signed 32-bit integer"));

        err = run_WithResult(R"(
            ROOT.Int64 = 1000000000000000000000000000000000000000
        )");

        EXPECT_THAT(err.what(),
            ::testing::HasSubstr("Error during assignment of property 'Int64'! Error while extracting integer: integral part too large to fit in a signed 64-bit integer"));

        err = run_WithResult(R"(
            ROOT.Float = 1000000000000000000000000000000000000000.0
        )");

        EXPECT_THAT(err.what(),
            ::testing::HasSubstr("Error during assignment of property 'Float'! Error while extracting floating point number: value would cause overflow in float"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, AppliesNumericChecks_StructFields_Vec)
    {
        PropertyImpl root(m_structWithAllPrimitiveTypes, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrapped(root);
        m_sol["ROOT"] = std::ref(wrapped);

        sol::error err = run_WithResult(R"(
            ROOT.Vec2f = {1000000000000000000000000000000000000000.0, 0}
        )");

        EXPECT_THAT(err.what(),
            ::testing::HasSubstr(
                "lua: error: Error while assigning output Vec2 property 'Vec2f'. "
                "Error while extracting array: unexpected value (type: 'number') at array element # 1! "
                "Reason: Error while extracting floating point number: value would cause overflow in float"));
    }

    TEST_F(AWrappedLuaProperty_Assignment, AppliesNumericChecks_Array)
    {
        PropertyImpl nestedStruct(HierarchicalTypeData{ TypeData("ROOT", EPropertyType::Struct), {MakeArray("array", 2, EPropertyType::Int32)} }, EPropertySemantics::ScriptOutput);
        WrappedLuaProperty wrapped(nestedStruct);
        m_sol["ROOT"] = std::ref(wrapped);

        sol::error err = run_WithResult(R"(
            ROOT.array = {1.5, 0}
        )");

        EXPECT_THAT(err.what(),
            ::testing::HasSubstr(
                "lua: error: Error during assignment of property ''! Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
    }


}
