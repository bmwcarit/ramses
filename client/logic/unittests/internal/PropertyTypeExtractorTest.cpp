//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "internals/PropertyTypeExtractor.h"

namespace rlogic::internal
{
    TEST(ThePropertyTypeExtractorGlobalSymbols, AreVisibleOnlyToSpecifiedEnvironment)
    {
        sol::state sol;
        sol::environment env(sol, sol::create, sol.globals());

        PropertyTypeExtractor::RegisterTypes(env);

        // Environment now has the Type symbols (used to declare data types)
        EXPECT_TRUE(env["Type"].valid());

        // Global lua state doesn't know this symbol
        EXPECT_FALSE(sol["Type"].valid());
    }

    TEST(ThePropertyTypeExtractorGlobalSymbols, UnregisteringSymbolsWillNotKeepAnythingInLuaStack)
    {
        sol::state sol;
        sol::environment env(sol, sol::create, sol.globals());

        PropertyTypeExtractor::RegisterTypes(env);
        EXPECT_TRUE(lua_gettop(sol.lua_state()) >= 0);

        PropertyTypeExtractor::UnregisterTypes(env);
        EXPECT_TRUE(lua_gettop(sol.lua_state()) == 0);
    }

    class APropertyTypeExtractor : public ::testing::Test
    {
    protected:
        APropertyTypeExtractor()
            : m_env(m_sol, sol::create, m_sol.globals())
        {
            PropertyTypeExtractor::RegisterTypes(m_env);
        }

        HierarchicalTypeData extractTypeInfo(std::string_view source)
        {
            auto resultAndType = extractTypeInfo_WithResult(source);
            EXPECT_TRUE(resultAndType.second.valid());
            return resultAndType.first;
        }

        std::pair<HierarchicalTypeData, sol::protected_function_result>  extractTypeInfo_WithResult(std::string_view source)
        {
            return extractTypeInfo_ThroughEnvironment(source, m_env);
        }

        std::pair<HierarchicalTypeData, sol::protected_function_result> extractTypeInfo_ThroughEnvironment(std::string_view source, sol::environment& env)
        {
            // Reference temporary extractor
            PropertyTypeExtractor extractor("IN", EPropertyType::Struct);
            env["IN"] = std::ref(extractor);

            // Load script and apply environment
            sol::protected_function loaded = m_sol.load(source);
            assert(loaded.valid());
            env.set_on(loaded);

            // Execute script, nullify extractor reference, return results
            sol::protected_function_result result = loaded();
            env["IN"] = sol::lua_nil;
            return std::make_pair(extractor.getExtractedTypeData(), std::move(result));
        }

        sol::state m_sol;
        sol::environment m_env;
    };

    TEST_F(APropertyTypeExtractor, ExtractsSinglePrimitiveProperty)
    {
        const HierarchicalTypeData typeInfo = extractTypeInfo(R"(
            IN.newInt = Type:Int32()
        )");

        const HierarchicalTypeData expected  = MakeStruct("IN", {{"newInt", EPropertyType::Int32}});

        EXPECT_EQ(typeInfo, expected);
    }

    TEST_F(APropertyTypeExtractor, ExtractsAllPrimitiveTypes_OrdersByPropertyNameLexicographically)
    {
        const HierarchicalTypeData typeInfo = extractTypeInfo(R"(
            IN.bool = Type:Bool()
            IN.string = Type:String()
            IN.int32 = Type:Int32()
            IN.int64 = Type:Int64()
            IN.vec2i = Type:Vec2i()
            IN.vec3i = Type:Vec3i()
            IN.vec4i = Type:Vec4i()
            IN.float = Type:Float()
            IN.vec2f = Type:Vec2f()
            IN.vec3f = Type:Vec3f()
            IN.vec4f = Type:Vec4f()
        )");

        const HierarchicalTypeData expected = MakeStruct("IN",
            {
                {"bool", EPropertyType::Bool},
                {"float", EPropertyType::Float},
                {"int32", EPropertyType::Int32},
                {"int64", EPropertyType::Int64},
                {"string", EPropertyType::String},
                {"vec2f", EPropertyType::Vec2f},
                {"vec2i", EPropertyType::Vec2i},
                {"vec3f", EPropertyType::Vec3f},
                {"vec3i", EPropertyType::Vec3i},
                {"vec4f", EPropertyType::Vec4f},
                {"vec4i", EPropertyType::Vec4i},
            }
        );

        EXPECT_EQ(typeInfo, expected);
    }

    TEST_F(APropertyTypeExtractor, ExtractsNestedTypes_OrdersByPropertyNameLexicographically_WhenUsingLuaTable)
    {
        const HierarchicalTypeData typeInfo = extractTypeInfo(R"(
            IN.nested = {
                int = Type:Int32(),
                vec4f = Type:Vec4f(),
                vec2i = Type:Vec2i(),
                bool = Type:Bool()
            }
        )");

        const HierarchicalTypeData expected{
            TypeData{"IN", EPropertyType::Struct},
            {
                HierarchicalTypeData{
                TypeData{"nested", EPropertyType::Struct}, {
                    MakeType("bool", EPropertyType::Bool),
                    MakeType("int", EPropertyType::Int32),
                    MakeType("vec2i", EPropertyType::Vec2i),
                    MakeType("vec4f", EPropertyType::Vec4f)
                }
                }
            }
        };

        EXPECT_EQ(typeInfo, expected);
    }

    TEST_F(APropertyTypeExtractor, ExtractsNestedTypes_OrdersLexicographically_WhenDeclaredOneByOne)
    {
        const HierarchicalTypeData typeInfo = extractTypeInfo(R"(
            IN.nested = {}
            IN.nested.s2 = {}
            IN.nested.s2.i2 = Type:Int32()
            IN.nested.s2.i1 = Type:Int32()
            IN.nested.b1 = Type:Bool()
        )");

        const HierarchicalTypeData expected {
            TypeData{"IN", EPropertyType::Struct},  // Root property
            {
                // Child properties
                HierarchicalTypeData{
                    TypeData{"nested", EPropertyType::Struct}, {
                        MakeType("b1", EPropertyType::Bool),
                        MakeStruct("s2", {
                            {"i1", EPropertyType::Int32},
                            {"i2", EPropertyType::Int32}
                            }),
                    }
                }
            }
        };

        EXPECT_EQ(typeInfo, expected);
    }

    class APropertyTypeExtractor_Errors : public APropertyTypeExtractor
    {
    protected:
        sol::error expectErrorDuringTypeExtraction(std::string_view luaCode)
        {
            const sol::protected_function_result result = extractTypeInfo_WithResult(luaCode).second;
            assert(!result.valid());
            sol::error error = result;
            return error;
        }
    };

    TEST_F(APropertyTypeExtractor_Errors, ProducesErrorWhenIndexingUndeclaredProperty)
    {
        const sol::error error = expectErrorDuringTypeExtraction("prop = IN.doesNotExist");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Field 'doesNotExist' does not exist in struct 'IN'"));
    }

    TEST_F(APropertyTypeExtractor_Errors, ProducesErrorWhenDeclaringPropertyTwice)
    {
        const sol::error error = expectErrorDuringTypeExtraction(
            R"(
                IN.property = Type:Int32()
                IN.property = Type:Float()
            )");

        EXPECT_THAT(error.what(), ::testing::HasSubstr("lua: error: Field 'property' already exists! Can't declare the same field twice!"));
    }

    TEST_F(APropertyTypeExtractor_Errors, ProducesErrorWhenTryingToAccessInterfaceProperties_WithNonStringIndex)
    {
        sol::error error = expectErrorDuringTypeExtraction("prop = IN[1]");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Bad index access to struct 'IN': Expected a string but got object of type number instead!"));
        error = expectErrorDuringTypeExtraction("prop = IN[true]");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Bad index access to struct 'IN': Expected a string but got object of type bool instead!"));
        error = expectErrorDuringTypeExtraction("prop = IN[{x=5}]");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Bad index access to struct 'IN': Expected a string but got object of type table instead!"));
        error = expectErrorDuringTypeExtraction("prop = IN[nil]");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Bad index access to struct 'IN': Expected a string but got object of type nil instead!"));
    }

    TEST_F(APropertyTypeExtractor_Errors, ProducesErrorWhenTryingToCreateInterfaceProperties_WithNonStringIndex)
    {
        sol::error error = expectErrorDuringTypeExtraction("IN[1] = Type:Int32()");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid index for new field on struct 'IN': Expected a string but got object of type number instead!"));
        error = expectErrorDuringTypeExtraction("IN[true] = Type:Int32()");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid index for new field on struct 'IN': Expected a string but got object of type bool instead!"));
        error = expectErrorDuringTypeExtraction("IN[{x=5}] = Type:Int32()");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid index for new field on struct 'IN': Expected a string but got object of type table instead!"));
        error = expectErrorDuringTypeExtraction("IN[nil] = Type:Int32()");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid index for new field on struct 'IN': Expected a string but got object of type nil instead!"));
    }

    TEST_F(APropertyTypeExtractor_Errors, InvalidTypeSpecifiers)
    {
        const std::vector<std::string> wrongStatements = {
            "IN.bad_type = nil",
            "IN.bad_type = 'not a type'",
            "IN.bad_type = true",
            "IN.bad_type = 150000",
        };

        for (const auto& statement : wrongStatements)
        {
            const sol::error error = expectErrorDuringTypeExtraction(statement);
            EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid type of field 'bad_type'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type"));
        }
    }

    TEST_F(APropertyTypeExtractor_Errors, InvalidTypeSpecifiers_Nested)
    {
        const std::vector<std::string> wrongStatements = {
            "IN.parent = {bad_type = 'not a type'}",
            "IN.parent = {bad_type = true}",
            "IN.parent = {bad_type = 150000}",
        };

        for (const auto& statement : wrongStatements)
        {
            const sol::error error = expectErrorDuringTypeExtraction(statement);
            EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid type of field 'bad_type'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type"));
        }
    }

    TEST_F(APropertyTypeExtractor_Errors, NoNameProvided_ForNestedProperty)
    {
        const sol::error error1 = expectErrorDuringTypeExtraction("IN.parent = {Type:Int32()}");
        const sol::error error2 = expectErrorDuringTypeExtraction("IN.parent = {5}");
        EXPECT_THAT(error1.what(), ::testing::HasSubstr("Invalid index for new field on struct 'parent': Expected a string but got object of type number instead!"));
        EXPECT_THAT(error2.what(), ::testing::HasSubstr("Invalid index for new field on struct 'parent': Expected a string but got object of type number instead!"));
    }

    TEST_F(APropertyTypeExtractor_Errors, CorrectNameButWrongTypeProvided_ForNestedProperty)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.no_nested_type = { correct_key = 'but wrong type' }");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid type of field 'correct_key'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type 'string' instead"));
    }

    TEST_F(APropertyTypeExtractor_Errors, UserdataAssignedToPropertyCausesError)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.very_wrong = IN");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid type of field 'very_wrong'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type 'userdata' instead"));
    }

    class APropertyTypeExtractor_Arrays : public APropertyTypeExtractor
    {
    protected:
    };

    TEST_F(APropertyTypeExtractor_Arrays, DeclaresArrayOfPrimitives)
    {
        const HierarchicalTypeData typeInfo = extractTypeInfo("IN.primArray = Type:Array(3, Type:Int32())");

        const HierarchicalTypeData arrayType = MakeArray("primArray", 3, EPropertyType::Int32);
        const HierarchicalTypeData expected({"IN", EPropertyType::Struct}, {arrayType});

        EXPECT_EQ(typeInfo, expected);
    }

    TEST_F(APropertyTypeExtractor_Arrays, DeclaresArrayOfStructs)
    {
        const HierarchicalTypeData typeInfo = extractTypeInfo("IN.structArray = Type:Array(3, {a = Type:Int32(), b = Type:Vec3f()})");

        ASSERT_EQ(1u, typeInfo.children.size());
        const HierarchicalTypeData& arrayType = typeInfo.children[0];

        EXPECT_EQ(arrayType.typeData, TypeData("structArray", EPropertyType::Array));

        for (const auto& arrayField : arrayType.children)
        {
            EXPECT_EQ(arrayField.typeData, TypeData("", EPropertyType::Struct));
            EXPECT_THAT(arrayField.children,
                ::testing::ElementsAre(
                    MakeType("a", EPropertyType::Int32),
                    MakeType("b", EPropertyType::Vec3f)
                )
            );
        }

        // Order within a struct is arbitrary, BUT each two structs in the array have the exact same order of child properties!
        ASSERT_EQ(3u, arrayType.children.size());
        EXPECT_EQ(arrayType.children[0], arrayType.children[1]);
        EXPECT_EQ(arrayType.children[1], arrayType.children[2]);
    }

    TEST_F(APropertyTypeExtractor_Arrays, DeclaresArrayOfArrays)
    {
        const HierarchicalTypeData typeInfo = extractTypeInfo("IN.array2d = Type:Array(3, Type:Array(2, Type:Int32()))");

        ASSERT_EQ(1u, typeInfo.children.size());
        const HierarchicalTypeData& arrayType = typeInfo.children[0];

        EXPECT_EQ(arrayType.typeData, TypeData("array2d", EPropertyType::Array));

        for (const auto& arrayField : arrayType.children)
        {
            EXPECT_EQ(arrayField.typeData, TypeData("", EPropertyType::Array));
            EXPECT_THAT(arrayField.children,
                ::testing::ElementsAre(
                    MakeType("", EPropertyType::Int32),
                    MakeType("", EPropertyType::Int32)
                )
            );
        }
    }

    class APropertyTypeExtractor_ArrayErrors : public APropertyTypeExtractor_Errors
    {
    protected:
    };

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayDefinedWithoutArguments)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array()");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with bad size argument! Error while extracting integer: expected a number, received 'nil'"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithFirstArgumentNotANumber)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array('not a number')");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with bad size argument! Error while extracting integer: expected a number, received 'string'"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithoutTypeArgument)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(5)");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with invalid type parameter T!"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithInvalidTypeArgument)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(5, 9000)");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid element type T of array field 'array'! Found a value of type T='number' instead of T=Type:<type>() in call array = Type:Array(N, T)!"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithZeroSize)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(0, Type:Int32())");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with invalid size parameter N=0 (must be in the range [1, 255])!"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithOutOfBoundsSize)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(256, Type:Int32())");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with invalid size parameter N=256 (must be in the range [1, 255])!"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithNegativeSize)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(-1, Type:Int32())");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with bad size argument! Error while extracting integer: expected non-negative number, received '-1'"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithFloatSize)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(1.5, Type:Int32())");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with bad size argument! Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithUserDataInsteadOfSize)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(IN, Type:Int32())");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Type:Array(N, T) invoked with bad size argument! Error while extracting integer: expected a number, received 'userdata'"));
    }

    TEST_F(APropertyTypeExtractor_ArrayErrors, ArrayWithUserDataInsteadOfTypeInfo)
    {
        const sol::error error = expectErrorDuringTypeExtraction("IN.array = Type:Array(5, IN)");
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Invalid element type T of array field 'array'! Found a value of type T='userdata' instead of T=Type:<type>() in call array = Type:Array(N, T)!"));
    }
}
