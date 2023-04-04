//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WithTempDirectory.h"
#include "FeatureLevelTestValues.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LuaModule.h"
#include "impl/LuaModuleImpl.h"
#include "internals/ErrorReporting.h"
#include "internals/SolState.h"
#include "internals/DeserializationMap.h"
#include "internals/SerializationMap.h"

#include "generated/LuaModuleGen.h"

#include <fstream>

namespace rlogic::internal
{
    class ALuaModuleFixture
    {
    protected:
        explicit ALuaModuleFixture(EFeatureLevel featureLevel)
            : m_logicEngine(featureLevel)
            , m_featureLevel(featureLevel)
        {
        }

        const std::string_view m_moduleSourceCode = R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b)
            end
            return mymath
        )";

        LuaConfig createDeps(const std::vector<std::pair<std::string_view, std::string_view>>& dependencies)
        {
            LuaConfig config;
            for (const auto& [alias, moduleSrc] : dependencies)
            {
                LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
                config.addDependency(alias, *mod);
            }

            return config;
        }

        LogicEngine m_logicEngine;
        EFeatureLevel m_featureLevel;
    };

    class ALuaModule : public ALuaModuleFixture, public ::testing::Test
    {
    protected:
        ALuaModule()
            : ALuaModuleFixture(EFeatureLevel_01)
        {
        }
    };

    TEST_F(ALuaModule, IsCreated)
    {
        const auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_EQ("mymodule", module->getName());
        EXPECT_EQ(module->getId(), 1u);
    }

    TEST_F(ALuaModule, ChangesName)
    {
        const auto module = m_logicEngine.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        module->setName("mm");
        EXPECT_EQ("mm", module->getName());
        EXPECT_EQ(module, this->m_logicEngine.findByName<LuaModule>("mm"));
        EXPECT_TRUE(this->m_logicEngine.getErrors().empty());
    }

    TEST_F(ALuaModule, FailsCreationIfSourceInvalid)
    {
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule("!", {}));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("unexpected symbol near '!'"));

        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b)
            return mymath
        )"));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("expected (to close 'function'"));

        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b
            end
            return mymath
        )"));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("expected (to close '('"));
    }

    TEST_F(ALuaModule, ProducesErrorMessageCorrectlyNotConflictingWithFmtFormatSyntax)
    {
        auto* module = m_logicEngine.createLuaModule(R"(
            local coalaModule = {}
            coalaModule.coalaStruct = {
                oink1
                oink2
            }
            return coalaModule
        )", {}, "missingComma");

        ASSERT_EQ(nullptr, module);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("'}' expected (to close '{'"));
    }

    class ALuaModule_SerializationLifecycle : public ALuaModuleFixture, public ::testing::TestWithParam<EFeatureLevel>
    {
    protected:
        ALuaModule_SerializationLifecycle()
            : ALuaModuleFixture(GetParam())
        {
        }

        SolState                                        m_solState;
        ErrorReporting                                  m_errorReporting;
        flatbuffers::FlatBufferBuilder                  m_flatBufferBuilder;
        DeserializationMap                              m_deserializationMap;
    };

    INSTANTIATE_TEST_SUITE_P(
        ALuaModule_SerializationLifecycleTests,
        ALuaModule_SerializationLifecycle,
        GetFeatureLevelTestValues());

    TEST_P(ALuaModule_SerializationLifecycle, CanBeSerialized)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine logic{ m_featureLevel };
            logic.createLuaModule(m_moduleSourceCode, {}, "mymodule");
            EXPECT_TRUE(logic.saveToFile("module.tmp"));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("module.tmp"));
        const auto module = m_logicEngine.findByName<LuaModule>("mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_EQ("mymodule", module->getName());
        EXPECT_EQ(module->getId(), 1u);
    }

    TEST_P(ALuaModule_SerializationLifecycle, StoresDuplicateByteCodeOnce)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        SerializationMap serializationMap;
        LogicEngine logic{ m_featureLevel };

        auto module1 = logic.createLuaModule(m_moduleSourceCode, {}, "mymodule1");
        auto module2 = logic.createLuaModule(m_moduleSourceCode, {}, "mymodule2");

        auto serOffset1 = LuaModuleImpl::Serialize(module1->m_impl, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        m_flatBufferBuilder.Finish(serOffset1);
        const auto& serialized1 = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        auto serOffset2 = LuaModuleImpl::Serialize(module2->m_impl, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        m_flatBufferBuilder.Finish(serOffset2);
        const auto& serialized2 = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_EQ(serialized1.luaByteCode(), serialized2.luaByteCode());
    }

    TEST_P(ALuaModule_SerializationLifecycle, ProducesErrorWhenNameMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    0, // no name
                    0)
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing name!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of LuaModule from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_P(ALuaModule_SerializationLifecycle, ProducesErrorWhenIdMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    0) // no id
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(this->m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(2u, this->m_errorReporting.getErrors().size());
        EXPECT_EQ("Fatal error during loading of LogicObject base from serialized data: missing or invalid ID!", this->m_errorReporting.getErrors()[0].message);
        EXPECT_EQ("Fatal error during loading of LuaModule from serialized data: missing name and/or ID!", this->m_errorReporting.getErrors()[1].message);
    }

    TEST_P(ALuaModule_SerializationLifecycle, ProducesErrorWhenBothsourceAndBytecodeMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0, // no source code
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>()),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>()),
                0 // no bytecode
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaModule from serialized data: has neither Lua source code nor bytecode!");
    }

    TEST_P(ALuaModule_SerializationLifecycle, ProducesErrorWhensourceAndBytecodeEmpty)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(""), // empty source code
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>()),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>()),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{}) // bytecode empty
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaModule from serialized data: has neither Lua source code nor bytecode!");
    }

    TEST_P(ALuaModule_SerializationLifecycle, ProducesErrorWhenDependenciesMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_moduleSourceCode),
                0, // missing dependencies
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>()),
                m_featureLevel == EFeatureLevel_01 ? 0 : m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaModule from serialized data: missing dependencies!");
    }

    TEST_P(ALuaModule_SerializationLifecycle, ProducesErrorWhenByteCodeInvalidAndNoSourceAvailable)
    {
        if (m_featureLevel < EFeatureLevel_02)
            GTEST_SKIP();

        {
            const std::vector<uint8_t> invalidByteCode(10, 0);

            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                0,
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>()),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>()),
                m_flatBufferBuilder.CreateVector(invalidByteCode)
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 2u);
        EXPECT_THAT(m_errorReporting.getErrors()[0].message, ::testing::HasSubstr("Fatal error during loading of LuaModule 'name': failed loading pre-compiled byte code"));
        EXPECT_THAT(m_errorReporting.getErrors()[1].message, ::testing::HasSubstr("Fatal error during loading of LuaModule 'name' from serialized data"));
    }

    TEST_P(ALuaModule_SerializationLifecycle, WillTryToRecompileModuleFromSourceWhenByteCodeInvalid)
    {
        if (m_featureLevel < EFeatureLevel_02)
            GTEST_SKIP();

        const std::vector<uint8_t> invalidByteCode(10, 0);
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                rlogic_serialization::CreateLogicObject(m_flatBufferBuilder,
                    m_flatBufferBuilder.CreateString("name"),
                    1u),
                m_flatBufferBuilder.CreateString(m_moduleSourceCode),
                m_flatBufferBuilder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>()),
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>()),
                m_flatBufferBuilder.CreateVector(invalidByteCode)
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);
        // check that script was recompiled successfully
        EXPECT_TRUE(deserialized);

        // serialize again and check that new byte code is produced
        {
            flatbuffers::FlatBufferBuilder builder;
            SerializationMap serializationMap;
            const auto fbModule = LuaModuleImpl::Serialize(*deserialized, builder, serializationMap, ELuaSavingMode::SourceAndByteCode);
            builder.Finish(fbModule);
            const auto serializedWithValidByteCode = flatbuffers::GetRoot<rlogic_serialization::LuaModule>(builder.GetBufferPointer());
            ASSERT_TRUE(serializedWithValidByteCode);
            ASSERT_TRUE(serializedWithValidByteCode->source());
            EXPECT_EQ(m_moduleSourceCode, serializedWithValidByteCode->source()->c_str());

            EXPECT_TRUE(serializedWithValidByteCode->luaByteCode());
            EXPECT_TRUE(serializedWithValidByteCode->luaByteCode()->size() > 0);
            EXPECT_NE(serializedWithValidByteCode->luaByteCode()->size(), invalidByteCode.size());
        }
    }

    TEST_P(ALuaModule_SerializationLifecycle, SerializesSourceCodeOnly_InSourceOnlyMode)
    {
        auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(module->m_impl, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceCodeOnly);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_FALSE(serialized.luaByteCode());
        ASSERT_TRUE(serialized.source());
        EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());
    }

    TEST_P(ALuaModule_SerializationLifecycle, SerializesSourceCodeOnly_InBytecodeOnlyMode_IfNoBytecodeAvailable)
    {
        // only way to simulate no bytecode available is by serializing in feature level 01 mode
        auto module = std::make_unique<LuaModuleImpl>(*LuaCompilationUtils::CompileModuleOrImportPrecompiled(
            m_solState, {}, {}, std::string{ m_moduleSourceCode }, "module", m_errorReporting, {}, EFeatureLevel_01, false), "module", 1u);

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(*module, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_FALSE(serialized.luaByteCode());
        ASSERT_TRUE(serialized.source());
        EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());
    }

    TEST_P(ALuaModule_SerializationLifecycle, SerializesSourceCodeOnly_InBothSourcAndBytecodeMode_IfNoBytecodeAvailable)
    {
        // only way to simulate no bytecode available is by serializing in feature level 01 mode
        auto module = std::make_unique<LuaModuleImpl>(*LuaCompilationUtils::CompileModuleOrImportPrecompiled(
            m_solState, {}, {}, std::string{ m_moduleSourceCode }, "module", m_errorReporting, {}, EFeatureLevel_01, false), "module", 1u);

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(*module, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_FALSE(serialized.luaByteCode());
        ASSERT_TRUE(serialized.source());
        EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());
    }

    TEST_P(ALuaModule_SerializationLifecycle, SerializesBytecodeOnly_InBytecodeOnlyMode)
    {
        auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(module->m_impl, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serialized.luaByteCode());
            EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
            EXPECT_FALSE(serialized.source());
        }
        else
        {
            // feature level 01 does not support byte code -> same as above test with bytecode not available
            EXPECT_FALSE(serialized.luaByteCode());
            ASSERT_TRUE(serialized.source());
            EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());
        }
    }

    TEST_P(ALuaModule_SerializationLifecycle, SerializesBytecodeOnly_InSourceCodeOnlyMode_IfNoSourceAvailable)
    {
        // simulate module with no source code by serializing it with bytecode only and deserializing again
        std::unique_ptr<LuaModuleImpl> moduleWithNoSourceCode;
        {
            auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");

            SerializationMap serializationMap;
            const auto fbModule = LuaModuleImpl::Serialize(module->m_impl, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
            m_flatBufferBuilder.Finish(fbModule);
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
            moduleWithNoSourceCode = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);
            ASSERT_TRUE(moduleWithNoSourceCode);
            m_flatBufferBuilder.Clear();
        }

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(*moduleWithNoSourceCode, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceCodeOnly);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serialized.luaByteCode());
            EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
            EXPECT_FALSE(serialized.source());
        }
        else
        {
            // feature level 01 does not support byte code -> same as above test with bytecode not available
            EXPECT_FALSE(serialized.luaByteCode());
            ASSERT_TRUE(serialized.source());
            EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());
        }
    }

    TEST_P(ALuaModule_SerializationLifecycle, SerializesBytecodeOnly_InBothSourceAndBytecodeMode_IfNoSourceAvailable)
    {
        // simulate module with no source code by serializing it with bytecode only and deserializing again
        std::unique_ptr<LuaModuleImpl> moduleWithNoSourceCode;
        {
            auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");

            SerializationMap serializationMap;
            const auto fbModule = LuaModuleImpl::Serialize(module->m_impl, m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
            m_flatBufferBuilder.Finish(fbModule);
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
            moduleWithNoSourceCode = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap, m_featureLevel);
            ASSERT_TRUE(moduleWithNoSourceCode);
            m_flatBufferBuilder.Clear();
        }

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(*moduleWithNoSourceCode, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        if (GetParam() >= EFeatureLevel_02)
        {
            ASSERT_TRUE(serialized.luaByteCode());
            EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
            EXPECT_FALSE(serialized.source());
        }
        else
        {
            // feature level 01 does not support byte code -> same as above test with bytecode not available
            EXPECT_FALSE(serialized.luaByteCode());
            ASSERT_TRUE(serialized.source());
            EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());
        }
    }

    TEST_P(ALuaModule_SerializationLifecycle, SerializesBothSourceAndBytecode)
    {
        auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(module->m_impl, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serialized.source());
        EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());

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

    TEST_P(ALuaModule_SerializationLifecycle, DoesNotContainDebugLogFunctionsAfterDeserialization)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine logic{ m_featureLevel };
            logic.createLuaModule(m_moduleSourceCode, {}, "mymodule");
            EXPECT_TRUE(logic.saveToFile("module.tmp"));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("module.tmp"));
        const auto module = m_logicEngine.findByName<LuaModule>("mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_FALSE(module->m_impl.hasDebugLogFunctions());
    }

    class ALuaModuleWithDependency : public ALuaModule
    {
    protected:
        std::string_view m_mathSrc = R"(
            local mymath = {}
            function mymath.add(a,b)
                return a+b
            end
            return mymath
        )";

        std::string_view m_quadsSrc = R"(
            modules("mymath")
            local quads = {}
            function quads.create(a,b)
                return {math.sin(a), math.cos(b), mymath.add(a, b)}
            end
            return quads
        )";
    };

    TEST_F(ALuaModuleWithDependency, IsCreated)
    {
        const auto quadsMod = m_logicEngine.createLuaModule(m_quadsSrc, createDeps({{"mymath", m_mathSrc}}), "quadsMod");
        ASSERT_NE(nullptr, quadsMod);
        EXPECT_EQ("quadsMod", quadsMod->getName());
        EXPECT_EQ(quadsMod->getId(), 2u); // module dependency has id 1u
    }

    TEST_F(ALuaModuleWithDependency, HasTwoDependencies)
    {
        std::string_view mathSubSrc = R"(
            local mymath = {}
            function mymath.sub(a,b)
                return a-b
            end
            return mymath
        )";

        std::string_view mathCombinedSrc = R"(
            modules("_mathAdd", "_mathSub")

            local mymath = {}
            mymath.add=_mathAdd.add
            mymath.sub=_mathSub.sub
            return mymath
        )";
        const auto mathCombined = m_logicEngine.createLuaModule(mathCombinedSrc, createDeps({ {"_mathAdd", m_mathSrc}, {"_mathSub", mathSubSrc} }));

        LuaConfig config;
        config.addDependency("_math", *mathCombined);
        const auto script = m_logicEngine.createLuaScript(R"(
            modules("_math")
            function interface(IN,OUT)
                OUT.added = Type:Int32()
                OUT.subbed = Type:Int32()
            end

            function run(IN,OUT)
                OUT.added = _math.add(1,2)
                OUT.subbed = _math.sub(15,5)
            end
        )", config);
        ASSERT_NE(nullptr, script);

        m_logicEngine.update();
        EXPECT_EQ(3, *script->getOutputs()->getChild("added")->get<int32_t>());
        EXPECT_EQ(10, *script->getOutputs()->getChild("subbed")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, UsesSameModuleUnderMultipleNames)
    {
        std::string_view mathCombinedSrc = R"(
            modules("math1", "math2")
            local math = {}
            math.add1=math1.add
            math.add2=math2.add
            return math
        )";
        const auto mathCombined = m_logicEngine.createLuaModule(mathCombinedSrc, createDeps({ {"math1", m_mathSrc}, {"math2", m_mathSrc} }));

        LuaConfig config;
        config.addDependency("mathAll", *mathCombined);
        const auto script = m_logicEngine.createLuaScript(R"(
            modules("mathAll")

            function interface(IN,OUT)
                OUT.result = Type:Int32()
            end

            function run(IN,OUT)
                OUT.result = mathAll.add1(1,2) + mathAll.add2(100,10)
            end
        )", config);
        ASSERT_NE(nullptr, script);

        m_logicEngine.update();
        EXPECT_EQ(113, *script->getOutputs()->getChild("result")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, TwoModulesDependOnTheSameModule)
    {
        const auto config = createDeps({ {"mymath", m_mathSrc} });

        const auto mathUser1 = m_logicEngine.createLuaModule(R"(
            modules("mymath")
            local mathUser1 = {}
            function mathUser1.add(a, b)
                return mymath.add(a + 1, b + 1)
            end
            return mathUser1
        )", config);

        const auto mathUser2 = m_logicEngine.createLuaModule(R"(
            modules("mymath")
            local mathUser2 = {}
            function mathUser2.add(a, b)
                return mymath.add(a + 10, b + 10)
            end
            return mathUser2
        )", config);

        LuaConfig scriptConfig;
        scriptConfig.addDependency("math1", *mathUser1);
        scriptConfig.addDependency("math2", *mathUser2);
        const auto script = m_logicEngine.createLuaScript(R"(
            modules("math1", "math2")
            function interface(IN,OUT)
                OUT.result1 = Type:Int32()
                OUT.result2 = Type:Int32()
            end

            function run(IN,OUT)
                OUT.result1 = math1.add(1,2)
                OUT.result2 = math2.add(1,2)
            end
        )", scriptConfig);
        ASSERT_NE(nullptr, script);

        m_logicEngine.update();
        EXPECT_EQ(5, *script->getOutputs()->getChild("result1")->get<int32_t>());
        EXPECT_EQ(23, *script->getOutputs()->getChild("result2")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, CanBeSerialized)
    {
        WithTempDirectory tmpDir;
        {
            LogicEngine logic;
            LuaConfig config;
            config.addDependency("mymath", *logic.createLuaModule(m_mathSrc, {}, "mathMod"));
            logic.createLuaModule(m_quadsSrc, config, "quadsMod");
            EXPECT_TRUE(logic.saveToFile("dep_modules.tmp"));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("dep_modules.tmp"));

        const LuaModule* mathMod = m_logicEngine.findByName<LuaModule>("mathMod");
        auto quadsMod = m_logicEngine.findByName<LuaModule>("quadsMod");
        ASSERT_NE(mathMod, nullptr);
        ASSERT_NE(quadsMod, nullptr);
        EXPECT_EQ(mathMod->getId(), 1u);
        EXPECT_EQ(quadsMod->getId(), 2u);

        EXPECT_THAT(quadsMod->m_impl.getDependencies(), ::testing::ElementsAre(std::pair<std::string, const LuaModule*>({"mymath", mathMod})));
    }

    TEST_F(ALuaModuleWithDependency, UpdatesWithoutIssues_WhenModulesWithRuntimeErrors_AreNeverCalled)
    {
        std::string_view errors = R"(
            local mymath = {}
            function mymath.add(a,b)
                error("this fails always")
                return a+b
            end
            return mymath
        )";
        const auto quadsMod = m_logicEngine.createLuaModule(m_quadsSrc, createDeps({ {"mymath", errors} }));
        ASSERT_NE(nullptr, quadsMod);

        // This works fine, because neither the the quads module nor the math modules are ever called
        EXPECT_TRUE(m_logicEngine.update());
    }

    class ALuaModuleDependencyMatch : public ALuaModuleWithDependency
    {
    };

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_NotProvidedButDeclared)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep2")
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(mathExt, createDeps({ {"dep2", m_moduleSourceCode} })));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep2"));
    }

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ProvidedButNotDeclared)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep2")
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(mathExt, createDeps({ {"dep1", m_moduleSourceCode}, {"dep2", m_moduleSourceCode}, {"dep3", m_moduleSourceCode} })));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep1, dep2, dep3"));
    }

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ExractionError)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep1") -- duplicate dependency
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(mathExt, createDeps({ {"dep1", m_moduleSourceCode} })));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while extracting module dependencies: 'dep1' appears more than once in dependency list"));
    }
}
