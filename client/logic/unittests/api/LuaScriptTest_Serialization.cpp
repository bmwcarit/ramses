//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "impl/LuaScriptImpl.h"
#include "impl/LogicEngineImpl.h"
#include "impl/PropertyImpl.h"
#include "impl/LuaModuleImpl.h"
#include "internals/ErrorReporting.h"

#include "generated/LuaScriptGen.h"

#include "SerializationTestUtils.h"
#include "FeatureLevelTestValues.h"

namespace rlogic::internal
{
    // Serialization unit tests only. For higher-order tests, check ALuaScript_LifecycleWithFiles
    class ALuaScript_Serialization : public ::testing::TestWithParam<EFeatureLevel>
    {
    protected:
        std::unique_ptr<LuaScriptImpl> createTestScript(std::string_view source, std::string_view scriptName = "")
        {
            return std::make_unique<LuaScriptImpl>(
                *LuaCompilationUtils::CompileScriptOrImportPrecompiled(m_solState, {}, {}, std::string{ source }, scriptName, m_errorReporting, {}, {}, {}, m_featureLevel, false),
                scriptName, 1u);
        }

        std::vector<uint8_t> static GetByteCodeForSource(std::string_view source)
        {
            sol::state solState;
            sol::load_result loadResult = solState.load(source);
            sol::protected_function function = loadResult;
            sol::bytecode byteCode = function.dump();
            std::vector<uint8_t> result;
            std::transform(byteCode.cbegin(), byteCode.cend(), std::back_inserter(result), [](auto v) {return uint8_t(v); });
            return result;
        }

        std::string_view m_minimalScript = R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end
        )";

        SolState m_solState;
        ErrorReporting m_errorReporting;
        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils{ m_flatBufferBuilder };
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
        EFeatureLevel m_featureLevel = GetParam();
    };

    INSTANTIATE_TEST_SUITE_P(
        ALuaScript_SerializationTests,
        ALuaScript_Serialization,
        GetFeatureLevelTestValues());

    // More unit tests with inputs/outputs declared in LogicNode (base class) serialization tests
    TEST_P(ALuaScript_Serialization, RemembersBaseClassData)
    {
        // Serialize
        {
            std::unique_ptr<LuaScriptImpl> script = createTestScript(m_minimalScript, "name");
            (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, m_serializationMap, ELuaSavingMode::ByteCodeOnly);
        }

        // Inspect flatbuffers data
        const auto& serializedScript = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serializedScript.base());
        ASSERT_TRUE(serializedScript.base()->name());
        EXPECT_EQ(serializedScript.base()->name()->string_view(), "name");
        EXPECT_EQ(serializedScript.base()->id(), 1u);

        ASSERT_TRUE(serializedScript.rootInput());
        EXPECT_EQ(serializedScript.rootInput()->rootType(), rlogic_serialization::EPropertyRootType::Struct);
        ASSERT_TRUE(serializedScript.rootInput()->children());
        EXPECT_EQ(serializedScript.rootInput()->children()->size(), 0u);

        ASSERT_TRUE(serializedScript.rootOutput());
        EXPECT_EQ(serializedScript.rootOutput()->rootType(), rlogic_serialization::EPropertyRootType::Struct);
        ASSERT_TRUE(serializedScript.rootOutput()->children());
        EXPECT_EQ(serializedScript.rootOutput()->children()->size(), 0u);

        // Deserialize
        {
            std::unique_ptr<LuaScriptImpl> deserializedScript = LuaScriptImpl::Deserialize(m_solState, serializedScript, m_errorReporting, m_deserializationMap, m_featureLevel);

            ASSERT_TRUE(deserializedScript);
            EXPECT_TRUE(m_errorReporting.getErrors().empty());

            EXPECT_EQ(deserializedScript->getName(), "name");
            EXPECT_EQ(deserializedScript->getId(), 1u);
        }
    }

    TEST_P(ALuaScript_Serialization, SerializesLuaSourceCode)
    {
        {
            std::unique_ptr<LuaScriptImpl> script = createTestScript(m_minimalScript, "");
            (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, m_serializationMap, ELuaSavingMode::SourceCodeOnly);
        }

        const auto& serializedScript = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        ASSERT_TRUE(serializedScript.luaSourceCode());
        EXPECT_EQ(serializedScript.luaSourceCode()->string_view(), m_minimalScript);
    }

    TEST_P(ALuaScript_Serialization, SerializesLuaByteCode)
    {
        {
            std::unique_ptr<LuaScriptImpl> script = createTestScript(m_minimalScript, "");
            (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, m_serializationMap, ELuaSavingMode::ByteCodeOnly);
        }

        const auto& serializedScript = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        if (m_featureLevel >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serializedScript.luaByteCode());
            EXPECT_TRUE(serializedScript.luaByteCode()->size() > 0);
        }
        else
        {
            ASSERT_FALSE(serializedScript.luaByteCode());
        }
    }

    TEST_P(ALuaScript_Serialization, BackwardsCompatiblity_CanDeserializeFromLuaSourceCode)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                0 // no byte code
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);
        EXPECT_TRUE(deserialized);
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_P(ALuaScript_Serialization, StoresDuplicateByteCodeOnce)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        SerializationMap serializationMap;

        auto script1 = createTestScript(m_minimalScript, "script");
        auto script2 = createTestScript(m_minimalScript, "script2");

        (void)LuaScriptImpl::Serialize(*script1, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        const auto& serialized1 = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        const auto byteCode1Offset = serialized1.luaByteCode();

        (void)LuaScriptImpl::Serialize(*script2, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        const auto& serialized2 = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        const auto byteCode2Offset = serialized2.luaByteCode();

        EXPECT_EQ(byteCode1Offset, byteCode2Offset);
    }

    TEST_P(ALuaScript_Serialization, DoesNotContainDebugLogFunctionsAfterDeserialization)
    {
        // Serialize
        {
            std::unique_ptr<LuaScriptImpl> script = createTestScript(m_minimalScript, "name");
            (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, m_serializationMap, ELuaSavingMode::ByteCodeOnly);
        }

        // Deserialize
        const auto& serializedScript = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserializedScript = LuaScriptImpl::Deserialize(m_solState, serializedScript, m_errorReporting, m_deserializationMap, m_featureLevel);
        ASSERT_TRUE(deserializedScript);
        EXPECT_FALSE(deserializedScript->hasDebugLogFunctions());
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenNameMissing)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    0, // no name
                    1u)
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing name!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of LuaScript from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenIdMissing)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    0) // no id (id gets checked before name)
                );
            m_flatBufferBuilder.Finish(script);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing or invalid ID!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of LuaScript from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenBothLuaSourceCodeAndBytecodeMissing)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0, // no source code
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                0 // no bytecode
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript from serialized data: has neither Lua source code nor bytecode!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenBothLuaSourceCodeAndBytecodeEmpty)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(""), // source code empty
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}) // bytecode empty
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript from serialized data: has neither Lua source code nor bytecode!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenUserModulesMissing)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                0, // no user modules
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        EXPECT_EQ(nullptr, LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel));
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript from serialized data: missing user module dependencies!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenStandardModulesMissing)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                0, // no standard modules
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        EXPECT_EQ(nullptr, LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel));
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript from serialized data: missing standard module dependencies!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenRootInputMissing)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                0, // no root input
                m_testUtils.serializeTestProperty(""),
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript from serialized data: missing root input!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenRootOutputMissing)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                0, // no root output
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript from serialized data: missing root output!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenRootInputHasErrors)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty("", rlogic_serialization::EPropertyRootType::Struct, true, true), // create root input with errors
                m_testUtils.serializeTestProperty(""),
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenRootOutputHasErrors)
    {
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty("", rlogic_serialization::EPropertyRootType::Struct, true, true), // create root output with errors
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of Property from serialized data: missing name!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenByteCodeInvalidAndNoSourceAvailable)
    {
        if (m_featureLevel < EFeatureLevel_02)
            GTEST_SKIP();

        {
            std::vector<uint8_t> invalidByteCode(10, 0);

            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_flatBufferBuilder.CreateVector(invalidByteCode)
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_THAT(m_errorReporting.getErrors()[0].message, ::testing::HasSubstr("Fatal error during loading of LuaScript 'name': failed loading pre-compiled byte code"));
        EXPECT_THAT(m_errorReporting.getErrors()[1].message, ::testing::HasSubstr("Fatal error during loading of LuaScript 'name' from serialized data"));
    }

    TEST_P(ALuaScript_Serialization, WillTryToRecompileScriptFromSourceWhenByteCodeInvalid)
    {
        if (m_featureLevel < EFeatureLevel_02)
            GTEST_SKIP();

        const std::vector<uint8_t> invalidByteCode(1, 0);
        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_flatBufferBuilder.CreateVector(invalidByteCode)
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);
        // check that script was recompiled successfully
        EXPECT_TRUE(deserialized);

        // serialize again and check that new byte code is produced
        {
            flatbuffers::FlatBufferBuilder builder;
            (void)LuaScriptImpl::Serialize(*deserialized, builder, m_serializationMap, ELuaSavingMode::SourceAndByteCode);
            const auto serializedWithValidByteCode = flatbuffers::GetRoot<rlogic_serialization::LuaScript>(builder.GetBufferPointer());
            ASSERT_TRUE(serializedWithValidByteCode);
            ASSERT_TRUE(serializedWithValidByteCode->luaSourceCode());
            EXPECT_EQ(m_minimalScript, serializedWithValidByteCode->luaSourceCode()->c_str());

            EXPECT_TRUE(serializedWithValidByteCode->luaByteCode());
            EXPECT_TRUE(serializedWithValidByteCode->luaByteCode()->size() > 0);
            EXPECT_NE(serializedWithValidByteCode->luaByteCode()->size(), invalidByteCode.size());
        }
    }

    TEST_P(ALuaScript_Serialization, SerializesSourceCodeOnly_InSourceOnlyMode)
    {
        auto script = createTestScript(m_minimalScript, "script");

        SerializationMap serializationMap;
        (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceCodeOnly);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_FALSE(serialized.luaByteCode());
        ASSERT_TRUE(serialized.luaSourceCode());
        EXPECT_EQ(m_minimalScript, serialized.luaSourceCode()->string_view());
    }

    TEST_P(ALuaScript_Serialization, SerializesSourceCodeOnly_InBytecodeOnlyMode_IfNoBytecodeAvailable)
    {
        // only way to simulate no bytecode available is by serializing in feature level 01 mode
        auto script = std::make_unique<LuaScriptImpl>(*LuaCompilationUtils::CompileScriptOrImportPrecompiled(
            m_solState, {}, {}, std::string{ m_minimalScript }, "script", m_errorReporting, {}, {}, {}, EFeatureLevel_01, false), "script", 1u);

        SerializationMap serializationMap;
        (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_FALSE(serialized.luaByteCode());
        ASSERT_TRUE(serialized.luaSourceCode());
        EXPECT_EQ(m_minimalScript, serialized.luaSourceCode()->string_view());
    }

    TEST_P(ALuaScript_Serialization, SerializesSourceCodeOnly_InBothSourcAndBytecodeMode_IfNoBytecodeAvailable)
    {
        // only way to simulate no bytecode available is by serializing in feature level 01 mode
        auto script = std::make_unique<LuaScriptImpl>(*LuaCompilationUtils::CompileScriptOrImportPrecompiled(
            m_solState, {}, {}, std::string{ m_minimalScript }, "script", m_errorReporting, {}, {}, {}, EFeatureLevel_01, false), "script", 1u);

        SerializationMap serializationMap;
        (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_FALSE(serialized.luaByteCode());
        ASSERT_TRUE(serialized.luaSourceCode());
        EXPECT_EQ(m_minimalScript, serialized.luaSourceCode()->string_view());
    }

    TEST_P(ALuaScript_Serialization, SerializesBytecodeOnly_InBytecodeOnlyMode)
    {
        auto script = createTestScript(m_minimalScript, "script");

        SerializationMap serializationMap;
        (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serialized.luaByteCode());
            EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
            EXPECT_FALSE(serialized.luaSourceCode());
        }
        else
        {
            // feature level 01 does not support byte code -> same as above test with bytecode not available
            EXPECT_FALSE(serialized.luaByteCode());
            ASSERT_TRUE(serialized.luaSourceCode());
            EXPECT_EQ(m_minimalScript, serialized.luaSourceCode()->string_view());
        }
    }

    TEST_P(ALuaScript_Serialization, SerializesBytecodeOnly_InSourceCodeOnlyMode_IfNoSourceAvailable)
    {
        // simulate script with no source code by serializing it with bytecode only and deserializing again
        std::unique_ptr<LuaScriptImpl> scriptWithNoSourceCode;
        {
            auto script = createTestScript(m_minimalScript, "script");

            SerializationMap serializationMap;
            (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
            scriptWithNoSourceCode = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);
            ASSERT_TRUE(scriptWithNoSourceCode);
            m_flatBufferBuilder.Clear();
        }

        SerializationMap serializationMap;
        (void)LuaScriptImpl::Serialize(*scriptWithNoSourceCode, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceCodeOnly);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serialized.luaByteCode());
            EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
            EXPECT_FALSE(serialized.luaSourceCode());
        }
        else
        {
            // feature level 01 does not support byte code -> same as above test with bytecode not available
            EXPECT_FALSE(serialized.luaByteCode());
            ASSERT_TRUE(serialized.luaSourceCode());
            EXPECT_EQ(m_minimalScript, serialized.luaSourceCode()->string_view());
        }
    }

    TEST_P(ALuaScript_Serialization, SerializesBytecodeOnly_InBothSourceAndBytecodeMode_IfNoSourceAvailable)
    {
        // simulate script with no source code by serializing it with bytecode only and deserializing again
        std::unique_ptr<LuaScriptImpl> scriptWithNoSourceCode;
        {
            auto script = createTestScript(m_minimalScript, "script");

            SerializationMap serializationMap;
            (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
            scriptWithNoSourceCode = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);
            ASSERT_TRUE(scriptWithNoSourceCode);
            m_flatBufferBuilder.Clear();
        }

        SerializationMap serializationMap;
        (void)LuaScriptImpl::Serialize(*scriptWithNoSourceCode, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serialized.luaByteCode());
            EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
            EXPECT_FALSE(serialized.luaSourceCode());
        }
        else
        {
            // feature level 01 does not support byte code -> same as above test with bytecode not available
            EXPECT_FALSE(serialized.luaByteCode());
            ASSERT_TRUE(serialized.luaSourceCode());
            EXPECT_EQ(m_minimalScript, serialized.luaSourceCode()->string_view());
        }
    }

    TEST_P(ALuaScript_Serialization, SerializesBothSourceAndBytecode)
    {
        auto script = createTestScript(m_minimalScript, "script");

        SerializationMap serializationMap;
        (void)LuaScriptImpl::Serialize(*script, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serialized.luaSourceCode());
        EXPECT_EQ(m_minimalScript, serialized.luaSourceCode()->string_view());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serialized.luaByteCode());
            EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
        }
        else
        {
            // feature level 01 does not support byte code -> same as above test with bytecode not available
            EXPECT_FALSE(serialized.luaByteCode());
        }
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenScriptDeclaresGlobalVariables)
    {
        {
            const std::string_view src = R"(
                global="this will cause error"
                function interface(IN,OUT)
                end

                function run(IN,OUT)
                    local a = 10
                end
                )";
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_featureLevel == EFeatureLevel_01 ? m_flatBufferBuilder.CreateString(src) : 0,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_featureLevel >= EFeatureLevel_02 ? m_flatBufferBuilder.CreateVector(GetByteCodeForSource(src)) : 0
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_THAT(m_errorReporting.getErrors()[0].message, ::testing::HasSubstr("Declaring global variables is forbidden (exceptions: the functions 'init', 'interface' and 'run')!"));
        EXPECT_THAT(m_errorReporting.getErrors()[1].message, ::testing::HasSubstr("Fatal error during loading of LuaScript 'name' from serialized data!"));
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenLuaScriptSourceHasSyntaxErrors)
    {
        if (m_featureLevel != EFeatureLevel_01)
        {
            GTEST_SKIP();
        }

        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("script"),
                    1u),
                m_flatBufferBuilder.CreateString("this.is.bad.code"),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty("")
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_THAT(m_errorReporting.getErrors()[0].message, ::testing::HasSubstr("Error while loading script. Lua stack trace"));
        EXPECT_THAT(m_errorReporting.getErrors()[1].message, ::testing::HasSubstr("Fatal error during loading of LuaScript 'script' from serialized data!"));
    }

    // Can't happen in normal usage; still, we test this case because we should not rely on correct export during loading
    TEST_P(ALuaScript_Serialization, ProducesErrorWhenLuaScriptSourceBreaksSandbox_WhenLoaded)
    {
        {
            std::string_view brokenScript = R"(
            globalVariable = 5 -- breaks sandbox

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end
        )";
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("script"),
                    1u),
                m_featureLevel == EFeatureLevel_01 ? m_flatBufferBuilder.CreateString(brokenScript) : 0,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_featureLevel >= EFeatureLevel_02 ? m_flatBufferBuilder.CreateVector(GetByteCodeForSource(brokenScript)) : 0
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_THAT(m_errorReporting.getErrors()[0].message,
            ::testing::HasSubstr("Declaring global variables is forbidden (exceptions: the functions 'init', 'interface' and 'run')! (found value of type 'number')"));
        EXPECT_THAT(m_errorReporting.getErrors()[1].message, ::testing::HasSubstr("Fatal error during loading of LuaScript 'script' from serialized data!"));
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenLuaScriptSourceBreaksSandbox_WhenExecuted)
    {
        {
            std::string_view brokenScript = R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
                globalVariable = 5 -- breaks sandbox
            end
        )";
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("script"),
                    1u),
                m_featureLevel == EFeatureLevel_01 ? m_flatBufferBuilder.CreateString(brokenScript) : 0,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_featureLevel >= EFeatureLevel_02 ? m_flatBufferBuilder.CreateVector(GetByteCodeForSource(brokenScript)) : 0
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_TRUE(deserialized);
        auto expectedError = deserialized->update();
        EXPECT_TRUE(expectedError);

        EXPECT_THAT(expectedError->message,
            ::testing::HasSubstr("Unexpected global variable definition 'globalVariable' in run()! Use the init() function to declare global data and functions, or use modules!"));
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenLuaScriptSourceHasRuntimeErrors)
    {
        if (m_featureLevel != EFeatureLevel_01)
        {
            GTEST_SKIP();
        }

        {
            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("script"),
                    1u),
                m_flatBufferBuilder.CreateString("error('This is not going to compile')"),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{ static_cast<uint8_t>(EStandardModule::Base) }),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty("") // create root output with errors
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_THAT(m_errorReporting.getErrors()[0].message, ::testing::HasSubstr("This is not going to compile"));
        EXPECT_THAT(m_errorReporting.getErrors()[1].message, ::testing::HasSubstr("Fatal error during loading of LuaScript 'script' from serialized data!"));
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenUserModuleHasNoName)
    {
        {
            const auto dummyModuleUsageFB = rlogic_serialization::CreateLuaModuleUsage(
                m_flatBufferBuilder,
                0, // no name
                0); // Invalid ID

            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{ dummyModuleUsageFB }),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript 'name' module data: missing name!");
    }

    TEST_P(ALuaScript_Serialization, ProducesErrorWhenUserModuleHasNoData)
    {
        {
            const auto dummyModuleUsageFB = rlogic_serialization::CreateLuaModuleUsage(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("moduleName"),
                42); // invalid module id

            auto script = rlogic_serialization::CreateLuaScript(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_minimalScript),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{ dummyModuleUsageFB }),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}),
                m_testUtils.serializeTestProperty(""),
                m_testUtils.serializeTestProperty(""),
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(script);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaScript>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaScriptImpl> deserialized = LuaScriptImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaScript 'name' module data: could not resolve dependent module with id=42!");
    }
}
