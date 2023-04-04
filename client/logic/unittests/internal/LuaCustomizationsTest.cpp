//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "ramses-logic/Property.h"
#include "impl/PropertyImpl.h"
#include "internals/LuaCustomizations.h"
#include "internals/WrappedLuaProperty.h"
#include "internals/LuaCompilationUtils.h"
#include "internals/ErrorReporting.h"
#include "internals/PropertyTypeExtractor.h"
#include <numeric>

namespace rlogic::internal
{
    enum class EWrappedType
    {
        RuntimeProperty,
        Extractor
    };

    class TheLuaCustomizations: public ::testing::Test
    {
        protected:
            TheLuaCustomizations()
            {
                m_sol.open_libraries();
                LuaCustomizations::RegisterTypes(m_sol);

                m_interfaceEnvironment = sol::environment(m_sol, sol::create, m_sol.globals());
                PropertyTypeExtractor::RegisterTypes(m_interfaceEnvironment);

                m_structProp.getChild("field1")->m_impl->setValue(5);
                m_structProp.getChild("field2")->m_impl->setValue(6);

                m_arrayProp.getChild(0)->m_impl->setValue(11);
                m_arrayProp.getChild(1)->m_impl->setValue(12);
                m_arrayProp.getChild(2)->m_impl->setValue(13);
            }

            sol::protected_function_result run_WithResult(std::string_view source)
            {
                sol::protected_function loaded = m_sol.load(source);
                assert(loaded.valid());
                return loaded();
            }

            sol::protected_function_result run_WithResult_InEnv(std::string_view source)
            {
                sol::protected_function loaded = m_sol.load(source);
                m_interfaceEnvironment.set_on(loaded);
                assert(loaded.valid());
                return loaded();
            }

            void expectNoErrors(std::string_view source)
            {
                sol::protected_function_result loaded = run_WithResult(source);
                ASSERT_TRUE (loaded.valid());
            }

            void expectError(std::string_view source, std::string_view errorSubstring)
            {
                sol::protected_function_result loaded = run_WithResult(source);
                ASSERT_FALSE(loaded.valid());
                sol::error e = loaded;
                EXPECT_THAT(e.what(), ::testing::HasSubstr(errorSubstring));
            }

            void expectNoErrors_WithEnv(std::string_view source)
            {
                sol::protected_function_result loaded = run_WithResult_InEnv(source);
                ASSERT_TRUE (loaded.valid());
            }

            void expectError_WithEnv(std::string_view source, std::string_view errorSubstring)
            {
                sol::protected_function_result loaded = run_WithResult_InEnv(source);
                ASSERT_FALSE(loaded.valid());
                sol::error e = loaded;
                EXPECT_THAT(e.what(), ::testing::HasSubstr(errorSubstring));
            }

            void createTestStruct(const std::string& name, EWrappedType wrappedType)
            {
                switch (wrappedType)
                {
                case EWrappedType::RuntimeProperty:
                    m_sol[name] = std::ref(m_wrappedStruct);
                    break;
                case EWrappedType::Extractor:
                    m_interfaceEnvironment[name] = std::ref(m_structExtractor);
                    break;
                }
            }

            void createTestArray(const std::string& name)
            {
                m_sol[name] = std::ref(m_wrappedArray);
            }

            template <EPropertyType TYPE, typename VALUE>
            void createTestProperty(const std::string& name, const VALUE& value)
            {
                HierarchicalTypeData type = { MakeType("V", TYPE) };
                auto prop = std::make_unique<PropertyImpl>(type, EPropertySemantics::ScriptInput);
                auto wrappedProp = std::make_unique<WrappedLuaProperty>(*prop);
                m_sol[name] = std::ref(*wrappedProp);

                prop->setValue(value);

                m_propertyImpls.push_back(std::move(prop));
                m_wrappedProperties.push_back(std::move(wrappedProp));
            }

            template <EPropertyType VECTYPE>
            void createTestVec(const std::string& name)
            {
                using VecClassType = typename PropertyEnumToType<VECTYPE>::TYPE;
                VecClassType vecData;
                std::iota(vecData.begin(), vecData.end(), static_cast<typename VecClassType::value_type>(100));

                createTestProperty<VECTYPE>(name, vecData);
            }

            sol::state m_sol;
            sol::environment m_interfaceEnvironment;
            // Initialize test content with dummy data
            HierarchicalTypeData m_structType = { TypeData("S", EPropertyType::Struct), {MakeType("field1", EPropertyType::Int32), MakeType("field2", EPropertyType::Int32)} };
            HierarchicalTypeData m_arrayType = { MakeArray("A", 3, EPropertyType::Int32) };
            PropertyImpl m_structProp = {m_structType, EPropertySemantics::ScriptInput};
            PropertyImpl m_arrayProp = {m_arrayType, EPropertySemantics::ScriptInput};
            WrappedLuaProperty m_wrappedStruct = WrappedLuaProperty{ m_structProp };
            WrappedLuaProperty m_wrappedArray = WrappedLuaProperty{ m_arrayProp };
            PropertyTypeExtractor m_structExtractor = { "S", EPropertyType::Struct };
            // to manage ownership
            std::vector<std::unique_ptr<PropertyImpl>> m_propertyImpls;
            std::vector<std::unique_ptr<WrappedLuaProperty>> m_wrappedProperties;
    };

    TEST_F(TheLuaCustomizations, RegistersFunctions)
    {
        EXPECT_TRUE(m_sol["rl_len"].valid());
        EXPECT_TRUE(m_sol["rl_next"].valid());
        EXPECT_TRUE(m_sol["rl_pairs"].valid());
        EXPECT_TRUE(m_sol["rl_ipairs"].valid());
    }

    class TheLuaCustomizations_Len : public TheLuaCustomizations
    {
    protected:
    };

    TEST_F(TheLuaCustomizations_Len, ComputesLengthOfStruct_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);
        expectNoErrors(R"(
            assert(rl_len(S) == 2)
        )");
    }

    TEST_F(TheLuaCustomizations_Len, ComputesLengthOfArray_DuringRuntime)
    {
        createTestArray("A");
        expectNoErrors(R"(
            assert(rl_len(A) == 3)
        )");
    }

    TEST_F(TheLuaCustomizations_Len, ComputesLengthOfVec_DuringRuntime)
    {
        createTestVec<EPropertyType::Vec4i>("V");
        expectNoErrors(R"(
            assert(rl_len(V) == 4)
        )");
    }

    TEST_F(TheLuaCustomizations_Len, ComputesLengthOfStruct_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            assert(rl_len(S) == 0)
            S.newField = Type:Int32()
            assert(rl_len(S) == 1)
        )");
    }

    TEST_F(TheLuaCustomizations_Len, ComputesLengthOfArray_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            S.array1 = Type:Array(3, Type:Float())
            assert(rl_len(S.array1) == 3)
            S.array2 = Type:Array(2, {a = Type:Int32(), b = Type:Float()})
            assert(rl_len(S.array2) == 2)
        )");
    }

    TEST_F(TheLuaCustomizations_Len, ComputesGetsLengthOfStandardTables)
    {
        expectNoErrors(R"(
            assert(rl_len({1, 2, 3}) == 3)
        )");
    }

    TEST_F(TheLuaCustomizations_Len, ProducesErrorWhenCallingCustomLengthFunctionOnBadTypes)
    {
        expectError("rl_len(5)", "lua: error: rl_len() called on an unsupported type 'number'");
        expectError("rl_len(\"a string\")", "lua: error: rl_len() called on an unsupported type 'string'");
        expectError("rl_len(true)", "lua: error: rl_len() called on an unsupported type 'bool'");
    }

    TEST_F(TheLuaCustomizations_Len, ProducesErrorWhenCallingCustomLengthFunctionOnUnsupportedInterfaceTypes)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.v = Type:Int32()
            print(rl_len(S.v))
        )", "rl_len() called on an unsupported type 'Int32'");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.vec = Type:Vec2i()
            print(rl_len(S.vec))
        )", "rl_len() called on an unsupported type 'Vec2i'");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.s = Type:String()
            print(rl_len(S.s))
        )", "rl_len() called on an unsupported type 'String'");
    }

    class TheLuaCustomizations_Next : public TheLuaCustomizations
    {
    };

    TEST_F(TheLuaCustomizations_Next, IteratesOverStruct_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);
        expectNoErrors(R"(
            k,v = rl_next(S)
            assert(k == 'field1')
            assert(v == 5)
            k,v = rl_next(S, 'field1')
            assert(k == 'field2')
            assert(v == 6)
            k,v = rl_next(S, 'field2')
            assert(k == nil)
            assert(v == nil)
        )");
    }

    TEST_F(TheLuaCustomizations_Next, CanBeUsedOnStructs_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            S.field1 = Type:Int32()
            S.field2 = Type:Float()

            k,v = rl_next(S)
            assert(k == 'field1')
            k,v = rl_next(S, 'field1')
            assert(k == 'field2')
            k,v = rl_next(S, 'field2')
            assert(k == nil)
            assert(v == nil)
        )");
    }

    TEST_F(TheLuaCustomizations_Next, CanBeUsedOnArrays_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            S.array1 = Type:Array(2, Type:Float())

            k,v = rl_next(S.array1)
            assert(k == 1)
            k,v = rl_next(S.array1, 1)
            assert(k == 2)
            k,v = rl_next(S.array1, 2)
            assert(k == nil)
            assert(v == nil)
        )");
    }

    TEST_F(TheLuaCustomizations_Next, IteratesOverArray_DuringRuntime)
    {
        createTestArray("A");
        expectNoErrors(R"(
            k,v = rl_next(A)
            assert(k == 1)
            assert(v == 11)
            k,v = rl_next(A, 1)
            assert(k == 2)
            assert(v == 12)
            k,v = rl_next(A, 2)
            assert(k == 3)
            assert(v == 13)
            k,v = rl_next(A, 3)
            assert(k == nil)
            assert(v == nil)
        )");
    }

    TEST_F(TheLuaCustomizations_Next, IteratesOverVec_DuringRuntime)
    {
        createTestVec<EPropertyType::Vec2i>("V");
        expectNoErrors(R"(
            k,v = rl_next(V)
            assert(k == 1)
            assert(v == 100)
            k,v = rl_next(V, 1)
            assert(k == 2)
            assert(v == 101)
            k,v = rl_next(V, 2)
            assert(k == nil)
            assert(v == nil)
        )");

        createTestVec<EPropertyType::Vec3i>("V");
        expectNoErrors(R"(
            k,v = rl_next(V)
            assert(k == 1)
            assert(v == 100)
            k,v = rl_next(V, 1)
            assert(k == 2)
            assert(v == 101)
            k,v = rl_next(V, 2)
            assert(k == 3)
            assert(v == 102)
            k,v = rl_next(V, 3)
            assert(k == nil)
            assert(v == nil)
        )");

        createTestVec<EPropertyType::Vec4i>("V");
        expectNoErrors(R"(
            k,v = rl_next(V)
            assert(k == 1)
            assert(v == 100)
            k,v = rl_next(V, 1)
            assert(k == 2)
            assert(v == 101)
            k,v = rl_next(V, 2)
            assert(k == 3)
            assert(v == 102)
            k,v = rl_next(V, 3)
            assert(k == 4)
            assert(v == 103)
            k,v = rl_next(V, 4)
            assert(k == nil)
            assert(v == nil)
        )");

        createTestVec<EPropertyType::Vec2f>("V");
        expectNoErrors(R"(
            k,v = rl_next(V)
            assert(k == 1)
            assert(v == 100)
            k,v = rl_next(V, 1)
            assert(k == 2)
            assert(v == 101)
            k,v = rl_next(V, 2)
            assert(k == nil)
            assert(v == nil)
        )");

        createTestVec<EPropertyType::Vec3f>("V");
        expectNoErrors(R"(
            k,v = rl_next(V)
            assert(k == 1)
            assert(v == 100)
            k,v = rl_next(V, 1)
            assert(k == 2)
            assert(v == 101)
            k,v = rl_next(V, 2)
            assert(k == 3)
            assert(v == 102)
            k,v = rl_next(V, 3)
            assert(k == nil)
            assert(v == nil)
        )");

        createTestVec<EPropertyType::Vec4f>("V");
        expectNoErrors(R"(
            k,v = rl_next(V)
            assert(k == 1)
            assert(v == 100)
            k,v = rl_next(V, 1)
            assert(k == 2)
            assert(v == 101)
            k,v = rl_next(V, 2)
            assert(k == 3)
            assert(v == 102)
            k,v = rl_next(V, 3)
            assert(k == 4)
            assert(v == 103)
            k,v = rl_next(V, 4)
            assert(k == nil)
            assert(v == nil)
        )");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsWhenCalledOnWrongType)
    {
        expectError("rl_next('string')", "lua: error: rl_next() called on an unsupported type 'string'");
        expectError("rl_next(true)", "lua: error: rl_next() called on an unsupported type 'bool'");
        expectError("rl_next(next)", "lua: error: rl_next() called on an unsupported type 'function'");
        expectError("rl_next(rl_next)", "lua: error: rl_next() called on an unsupported type 'function'");
    }

    TEST_F(TheLuaCustomizations_Next, IteratesOverEmptyContainers_DuringInterfaceExtraction)
    {
        PropertyTypeExtractor structExtractorEmpty = { "S", EPropertyType::Struct };
        PropertyTypeExtractor arrayExtractorEmpty = { "A", EPropertyType::Array };

        m_interfaceEnvironment["S"] = std::ref(structExtractorEmpty);
        m_interfaceEnvironment["A"] = std::ref(arrayExtractorEmpty);

        expectNoErrors_WithEnv(R"(
            k,v = rl_next(A)
            assert(k == nil)
            assert(v == nil)

            k,v = rl_next(S)
            assert(k == nil)
            assert(v == nil)
        )");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsWhenBadArrayIndexGiven_DuringRuntime)
    {
        createTestArray("A");

        expectError("rl_next(A, 0)", "Invalid key value '0' for rl_next(). Expected a number in the range [1, 3]!");
        expectError("rl_next(A, 4)", "Invalid key value '4' for rl_next(). Expected a number in the range [1, 3]!");
        expectError("rl_next(A, 'string')", "Invalid key to rl_next() of type: Error while extracting integer: expected a number, received 'string'");
        expectError("rl_next(A, {})", "Invalid key to rl_next() of type: Error while extracting integer: expected a number, received 'table'");
        expectError("rl_next(A, true)", "Invalid key to rl_next() of type: Error while extracting integer: expected a number, received 'bool'");
        expectError("rl_next(A, 1.5)", "Invalid key to rl_next() of type: Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)");
        expectError("rl_next(A, 1.001)", "Invalid key to rl_next() of type: Error while extracting integer: implicit rounding (fractional part '0.0009999999999998899' is not negligible)");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsWhenBadVecIndexGiven_DuringRuntime)
    {
        createTestVec<EPropertyType::Vec2f>("V");

        expectError("rl_next(V, 0)", "Index out of range! Expected 0 < index <= 2 but received index == 0");
        expectError("rl_next(V, 3)", "Index out of range! Expected 0 < index <= 2 but received index == 3");
        expectError("rl_next(V, 'string')", "Bad access to property 'V'! Error while extracting integer: expected a number, received 'string'");
        expectError("rl_next(V, {})", "Bad access to property 'V'! Error while extracting integer: expected a number, received 'table'");
        expectError("rl_next(V, true)", "Bad access to property 'V'! Error while extracting integer: expected a number, received 'bool'");
        expectError("rl_next(V, 1.5)", "Bad access to property 'V'! Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)");
        expectError("rl_next(V, 1.001)", "Bad access to property 'V'! Error while extracting integer: implicit rounding (fractional part '0.0009999999999998899' is not negligible)");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsWhenBadArrayIndexGiven_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv("S.array = Type:Array(2, Type:Float())");

        expectError_WithEnv("rl_next(S.array, 0)", "lua: error: Invalid key value '0' for rl_next(). Expected a number in the range [1, 2]!");
        expectError_WithEnv("rl_next(S.array, 3)", "lua: error: Invalid key value '3' for rl_next(). Expected a number in the range [1, 2]!");
        expectError_WithEnv("rl_next(S.array, 'not a number')", "lua: error: Invalid key to rl_next() of type: Error while extracting integer: expected a number, received 'string'");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsWhenBadStructsIndexGivenToCustomRlNextFunction_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);

        expectError("rl_next(S, 0)", "Bad access to property 'S'! Expected a string but got object of type number instead!");
        expectError("rl_next(S, 1)", "Bad access to property 'S'! Expected a string but got object of type number instead!");
        expectError("rl_next(S, {})", "Bad access to property 'S'! Expected a string but got object of type table instead!");
        expectError("rl_next(S, true)", "Bad access to property 'S'! Expected a string but got object of type bool instead!");
        expectError("rl_next(S, 1.5)", "Bad access to property 'S'! Expected a string but got object of type number instead!");
        expectError("rl_next(S, 1.001)", "Bad access to property 'S'! Expected a string but got object of type number instead!");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsWhenBadStructsIndexGivenToCustomRlNextFunction_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv("S.field = Type:Int32()");
        expectError_WithEnv("rl_next(S, 0)", "lua: error: Invalid key to rl_next(): Expected a string but got object of type number instead!");
        expectError_WithEnv("rl_next(S, 1)", "lua: error: Invalid key to rl_next(): Expected a string but got object of type number instead!");
        expectError_WithEnv("rl_next(S, {})", "lua: error: Invalid key to rl_next(): Expected a string but got object of type table instead!");
        expectError_WithEnv("rl_next(S, true)", "lua: error: Invalid key to rl_next(): Expected a string but got object of type bool instead!");
        expectError_WithEnv("rl_next(S, 1.5)", "lua: error: Invalid key to rl_next(): Expected a string but got object of type number instead!");
        expectError_WithEnv("rl_next(S, 1.001)", "lua: error: Invalid key to rl_next(): Expected a string but got object of type number instead!");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsForUnexistingPropertyInStruct_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);
        expectError("rl_next(S, 'no such field')", "lua: error: Tried to access undefined struct property 'no such field'");
    }

    TEST_F(TheLuaCustomizations_Next, ReportsErrorsForUnexistingPropertyInStruct_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv("S.field = Type:Int32()");
        expectError_WithEnv("rl_next(S, 'no such field')", "lua: error: Could not find field named 'no such field' in struct object 'S'");
    }

    TEST_F(TheLuaCustomizations_Next, ProducesErrorWhenUsedOnUnsupportedInterfaceTypes)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.v = Type:Int32()
            rl_next(S.v)
        )", "rl_next() called on an unsupported type 'Int32'");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.vec = Type:Vec2i()
            rl_next(S.vec)
        )", "rl_next() called on an unsupported type 'Vec2i'");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.s = Type:String()
            rl_next(S.s)
        )", "rl_next() called on an unsupported type 'String'");
    }

    TEST_F(TheLuaCustomizations_Next, WorksForWriteProtectedModules)
    {
        SolState solState;
        ErrorReporting errors;

        // Compile test module
        std::optional<LuaCompiledModule> mod = LuaCompilationUtils::CompileModuleOrImportPrecompiled(solState, {}, {}, R"(
            local mod = {}
            mod.mytable = {nested = {a = 42}}
            return mod
            )", "", errors, {}, EFeatureLevel_01, false);
        ASSERT_TRUE(errors.getErrors().empty());

        sol::load_result loadResult = solState.loadScript(R"(
            -- module has one key/value pair - a table named 'mytable'
            k, mytable = rl_next(mod)
            assert(k == 'mytable')
            assert(type(mytable) == 'table')

            -- next after 'mytable' is nil
            k,v = rl_next(mod, 'mytable')
            assert(k == nil)
            assert(v == nil)

            k,nested = rl_next(mytable)
            assert(k == 'nested')
            assert(type(nested) == 'table')

            k,v = rl_next(nested)
            assert(k == 'a')
            assert(v == 42)

            k,v = rl_next(nested, 'a')
            assert(k == nil)
            assert(v == nil)
        )", "");

        ASSERT_TRUE (loadResult.valid());

        // Apply environment, same as for real modules
        sol::protected_function mainFunction = loadResult;
        sol::environment env = solState.createEnvironment({EStandardModule::Base}, {}, false);
        env.set_on(mainFunction);
        env["mod"] = mod->moduleTable;

        sol::protected_function_result main_result = mainFunction();
        ASSERT_TRUE (main_result.valid());
    }

    class TheLuaCustomizations_Pairs : public TheLuaCustomizations
    {
    };

    TEST_F(TheLuaCustomizations_Pairs, IteratesOverStructFields_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(S) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == 'field1,field2,')
            assert(values == '5,6,')
        )");
    }

    TEST_F(TheLuaCustomizations_Pairs, IteratesOverArrayFields_DuringRuntime)
    {
        createTestArray("A");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(A) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,')
            assert(values == '11,12,13,')
        )");
    }

    TEST_F(TheLuaCustomizations_Pairs, IteratesOverVecElements_DuringRuntime)
    {
        createTestVec<EPropertyType::Vec2i>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,')
            assert(values == '100,101,')
        )");

        createTestVec<EPropertyType::Vec3i>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,')
            assert(values == '100,101,102,')
        )");

        createTestVec<EPropertyType::Vec4i>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,4,')
            assert(values == '100,101,102,103,')
        )");

        createTestVec<EPropertyType::Vec2f>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,')
            assert(values == '100,101,')
        )");

        createTestVec<EPropertyType::Vec3f>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,')
            assert(values == '100,101,102,')
        )");

        createTestVec<EPropertyType::Vec4f>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_pairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,4,')
            assert(values == '100,101,102,103,')
        )");
    }

    TEST_F(TheLuaCustomizations_Pairs, IteratesOverStructFields_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            -- Define some test fields
            S.field1 = Type:Int32()
            S.field2 = Type:String()

            local keys = ""
            local values = ""
            for k,v in rl_pairs(S) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == 'field1,field2,')
            assert(values == '4,10,')   -- The IDs of the type labels Type:Int32()/Type:String()
        )");
    }

    TEST_F(TheLuaCustomizations_Pairs, IteratesOverArrayFields_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            -- Define some test fields
            S.array = Type:Array(2, Type:Bool())

            local keys = ""
            local values = ""
            for k,v in rl_pairs(S.array) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,')
            assert(values == '11,11,') -- 11 is the enum value behind Type:Bool()
        )");
    }

    TEST_F(TheLuaCustomizations_Pairs, IteratesOverComplexArrays_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            -- Define some test fields
            S.array = Type:Array(2, {a=Type:Int32(), b=Type:Float()})

            local keys = ""
            local values = ""
            for k,v in rl_pairs(S.array) do
                for nk,nv in rl_pairs(v) do
                    keys = keys .. tostring(nk) .. ","
                    values = values .. tostring(nv) .. ","
                end
            end
            assert(keys == 'a,b,a,b,')
            assert(values == '4,0,4,0,')
        )");
    }

    TEST_F(TheLuaCustomizations_Pairs, WorksForWriteprotectedModules)
    {
        SolState solState;
        ErrorReporting errors;

        // Compile test module
        std::optional<LuaCompiledModule> mod = LuaCompilationUtils::CompileModuleOrImportPrecompiled(solState, {}, {}, R"(
            local mod = {}
            mod.mytable = {
                nested = {a = 11, b = 12}}
            return mod
            )", "", errors, {}, EFeatureLevel_01, false);
        ASSERT_TRUE(errors.getErrors().empty());

        sol::load_result loadResult = solState.loadScript(R"(
            for k,v in rl_pairs(mod.mytable.nested) do
                if k == 'a' then
                    valueOfA = v
                elseif k == 'b' then
                    valueOfB = v
                else
                    assert(false)
                end
            end
            assert(valueOfA == 11)
            assert(valueOfB == 12)
        )", "");

        ASSERT_TRUE(loadResult.valid());

        // Apply environment, same as for real modules
        sol::protected_function mainFunction = loadResult;
        sol::environment env = solState.createEnvironment({ EStandardModule::Base }, {}, false);
        env.set_on(mainFunction);
        env["mod"] = mod->moduleTable;

        sol::protected_function_result main_result = mainFunction();
        EXPECT_TRUE (main_result.valid());
    }

    TEST_F(TheLuaCustomizations_Pairs, ReportsErrorWhenUsedOnNotUserdataTypes_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);
        expectError("rl_pairs('string')", "lua: error: rl_pairs() called on an unsupported type 'string'. Use only with user types like IN/OUT, modules etc.!");
        expectError("rl_pairs(true)", "lua: error: rl_pairs() called on an unsupported type 'bool'. Use only with user types like IN/OUT, modules etc.!");
        expectError("rl_pairs(1.5)", "lua: error: rl_pairs() called on an unsupported type 'number'. Use only with user types like IN/OUT, modules etc.!");
    }

    TEST_F(TheLuaCustomizations_Pairs, ProducesErrorWhenUsedOnUnsupportedInterfaceTypes)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.v = Type:Int32()
            for k,v in rl_pairs(S.v) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
        )", "rl_next() called on an unsupported type 'Int32'");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.vec = Type:Vec2i()
            for k,v in rl_pairs(S.vec) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
        )", "rl_next() called on an unsupported type 'Vec2i'");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.s = Type:String()
            for k,v in rl_pairs(S.s) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
        )", "rl_next() called on an unsupported type 'String'");
    }


    class TheLuaCustomizations_IPairs : public TheLuaCustomizations
    {
    };

    TEST_F(TheLuaCustomizations_IPairs, IteratesOverArrayFields_DuringRuntime)
    {
        createTestArray("A");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_ipairs(A) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,')
            assert(values == '11,12,13,')
        )");
    }

    TEST_F(TheLuaCustomizations_IPairs, IteratesOverVecElements_DuringRuntime)
    {
        createTestVec<EPropertyType::Vec2i>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_ipairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,')
            assert(values == '100,101,')
        )");

        createTestVec<EPropertyType::Vec3i>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_ipairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,')
            assert(values == '100,101,102,')
        )");

        createTestVec<EPropertyType::Vec4i>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_ipairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,4,')
            assert(values == '100,101,102,103,')
        )");

        createTestVec<EPropertyType::Vec2f>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_ipairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,')
            assert(values == '100,101,')
        )");

        createTestVec<EPropertyType::Vec3f>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_ipairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,')
            assert(values == '100,101,102,')
        )");

        createTestVec<EPropertyType::Vec4f>("V");
        expectNoErrors(R"(
            local keys = ""
            local values = ""
            for k,v in rl_ipairs(V) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            assert(keys == '1,2,3,4,')
            assert(values == '100,101,102,103,')
        )");
    }

    TEST_F(TheLuaCustomizations_IPairs, IteratesOverArrayFields_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            -- Define some test fields
            S.array = Type:Array(3, Type:Int64())

            local keys = ""
            local values = ""
            for k,v in rl_ipairs(S.array) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
            print(keys)
            print(values)
            assert(keys == '1,2,3,')
            assert(values == '5,5,5,') -- 5 is the enum value behind Type:Int64()
        )");
    }

    TEST_F(TheLuaCustomizations_IPairs, IteratesOverComplexArrays_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectNoErrors_WithEnv(R"(
            -- Define some test fields
            S.array = Type:Array(2, {a=Type:Int32(), b=Type:Float()})

            local keys = ""
            for k,v in rl_ipairs(S.array) do
                keys = keys .. tostring(k) .. ","
                assert(type(v) == 'userdata')
            end
            assert(keys == '1,2,')
        )");
    }

    TEST_F(TheLuaCustomizations_IPairs, WorksForWriteprotectedModules)
    {
        SolState solState;
        ErrorReporting errors;

        // Compile test module
        std::optional<LuaCompiledModule> mod = LuaCompilationUtils::CompileModuleOrImportPrecompiled(solState, {}, {}, R"(
            local mod = {}
            mod.mytable = {
                nested = {[1] = 11, [2] = 12}}
            return mod
            )", "", errors, {}, EFeatureLevel_01, false);
        ASSERT_TRUE(errors.getErrors().empty());

        // Check that iterating over custom indexed table works and the order is the same (ascending by numeric index)
        sol::load_result loadResult = solState.loadScript(R"(
            local expected = {[1] = 11, [2] = 12}
            for k,v in rl_ipairs(mod.mytable.nested) do
                assert(expected[k] == v)
            end
        )", "");

        ASSERT_TRUE(loadResult.valid());

        // Apply environment, same as for real modules
        sol::protected_function mainFunction = loadResult;
        sol::environment env = solState.createEnvironment({ EStandardModule::Base }, {}, false);
        env.set_on(mainFunction);
        env["mod"] = mod->moduleTable;

        sol::protected_function_result main_result = mainFunction();
        ASSERT_TRUE(main_result.valid());
    }

    TEST_F(TheLuaCustomizations_IPairs, ReportsErrorWhenUsedOnStruct_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);
        expectError("rl_ipairs(S)", "rl_ipairs() called on an unsupported type 'Struct'. Use only with array-like built-in types or modules!");
    }

    TEST_F(TheLuaCustomizations_IPairs, ReportsErrorWhenUsedOnStruct_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv("rl_ipairs(S)", "rl_ipairs() called on an unsupported type 'Struct'. Use only with array-like built-in types or modules!");
    }

    TEST_F(TheLuaCustomizations_IPairs, ReportsErrorWhenUsedOnNotUserdataTypes_DuringRuntime)
    {
        createTestStruct("S", EWrappedType::RuntimeProperty);
        expectError("rl_ipairs('string')", "lua: error: rl_ipairs() called on an unsupported type 'string'. Use only with user types like IN/OUT, modules etc.!");
        expectError("rl_ipairs(true)", "lua: error: rl_ipairs() called on an unsupported type 'bool'. Use only with user types like IN/OUT, modules etc.!");
        expectError("rl_ipairs(1.5)", "lua: error: rl_ipairs() called on an unsupported type 'number'. Use only with user types like IN/OUT, modules etc.!");
    }

    TEST_F(TheLuaCustomizations_Pairs, ProducesErrorWhenUsedUnsupportedInterfaceTypes_DuringInterfaceExtraction)
    {
        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.v = Type:Int32()
            for k,v in rl_ipairs(S.v) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
        )", "rl_ipairs() called on an unsupported type 'Int32'. Use only with array-like built-in types or modules!");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.vec = Type:Vec2i()
            for k,v in rl_ipairs(S.vec) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
        )", "rl_ipairs() called on an unsupported type 'Vec2i'. Use only with array-like built-in types or modules!");

        createTestStruct("S", EWrappedType::Extractor);
        expectError_WithEnv(R"(
            S.s = Type:String()
            for k,v in rl_ipairs(S.s) do
                keys = keys .. tostring(k) .. ","
                values = values .. tostring(v) .. ","
            end
        )", "rl_ipairs() called on an unsupported type 'String'. Use only with array-like built-in types or modules!");
    }
}
