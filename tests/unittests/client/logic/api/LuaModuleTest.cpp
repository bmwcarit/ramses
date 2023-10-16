//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "LogicEngineTest_Base.h"

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LuaModule.h"
#include "impl/logic/LuaModuleImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/SolState.h"
#include "internal/logic/DeserializationMap.h"
#include "internal/logic/SerializationMap.h"

#include "internal/logic/flatbuffers/generated/LuaModuleGen.h"

#include <fstream>

namespace ramses::internal
{
    class ALuaModule : public ALogicEngine
    {
    protected:
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
                LuaModule* mod = m_logicEngine->createLuaModule(moduleSrc);
                config.addDependency(alias, *mod);
            }

            return config;
        }
    };

    TEST_F(ALuaModule, IsCreated)
    {
        const auto module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_EQ("mymodule", module->getName());
        EXPECT_EQ(module->getSceneObjectId().getValue(), 9u);
    }

    TEST_F(ALuaModule, ChangesName)
    {
        const auto module = m_logicEngine->createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        module->setName("mm");
        EXPECT_EQ("mm", module->getName());
        EXPECT_EQ(module, this->m_logicEngine->findObject<LuaModule>("mm"));
        expectNoError();
    }

    TEST_F(ALuaModule, FailsCreationIfSourceInvalid)
    {
        EXPECT_EQ(nullptr, m_logicEngine->createLuaModule("!", {}));
        auto msg = getLastErrorMessage();
        EXPECT_THAT(msg, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(msg, ::testing::HasSubstr("unexpected symbol near '!'"));

        EXPECT_EQ(nullptr, m_logicEngine->createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b)
            return mymath
        )"));
        msg = getLastErrorMessage();
        EXPECT_THAT(msg, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(msg, ::testing::HasSubstr("expected (to close 'function'"));

        EXPECT_EQ(nullptr, m_logicEngine->createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b
            end
            return mymath
        )"));
        msg = getLastErrorMessage();
        EXPECT_THAT(msg, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(msg, ::testing::HasSubstr("expected (to close '('"));
    }

    TEST_F(ALuaModule, ProducesErrorMessageCorrectlyNotConflictingWithFmtFormatSyntax)
    {
        auto* module = m_logicEngine->createLuaModule(R"(
            local coalaModule = {}
            coalaModule.coalaStruct = {
                oink1
                oink2
            }
            return coalaModule
        )", {}, "missingComma");

        ASSERT_EQ(nullptr, module);
        EXPECT_THAT(getLastErrorMessage(), ::testing::HasSubstr("'}' expected (to close '{'"));
    }

    class ALuaModule_SerializationLifecycle : public ALuaModule
    {
    protected:
        SolState                                        m_solState;
        ErrorReporting                                  m_errorReporting;
        flatbuffers::FlatBufferBuilder                  m_flatBufferBuilder;
        DeserializationMap                              m_deserializationMap{ m_scene->impl() };
    };

    TEST_F(ALuaModule_SerializationLifecycle, CanBeSerialized)
    {
        withTempDirectory();

        {
            LogicEngine& logic = *m_logicEngine;
            logic.createLuaModule(m_moduleSourceCode, {}, "mymodule");
            EXPECT_TRUE(saveToFileWithoutValidation("module.tmp"));
        }

        ASSERT_TRUE(recreateFromFile("module.tmp"));
        const auto module = m_logicEngine->findObject<LuaModule>("mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_EQ("mymodule", module->getName());
        EXPECT_EQ(module->getSceneObjectId().getValue(), 9u);
    }

    TEST_F(ALuaModule_SerializationLifecycle, StoresDuplicateByteCodeOnce)
    {
        SerializationMap serializationMap;
        LogicEngine& logic = *m_scene->createLogicEngine();

        auto module1 = logic.createLuaModule(m_moduleSourceCode, {}, "mymodule1");
        auto module2 = logic.createLuaModule(m_moduleSourceCode, {}, "mymodule2");

        auto serOffset1 = LuaModuleImpl::Serialize(module1->impl(), m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        m_flatBufferBuilder.Finish(serOffset1);
        const auto& serialized1 = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        auto serOffset2 = LuaModuleImpl::Serialize(module2->impl(), m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        m_flatBufferBuilder.Finish(serOffset2);
        const auto& serialized2 = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_EQ(serialized1.luaByteCode(), serialized2.luaByteCode());
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenNameMissing)
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
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(this->m_errorReporting.getError().has_value());
        EXPECT_EQ("Fatal error during loading of LuaModule from serialized data: missing name and/or ID!", this->m_errorReporting.getError()->message);
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenIdMissing)
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
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(this->m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(this->m_errorReporting.getError().has_value());
        EXPECT_EQ("Fatal error during loading of LuaModule from serialized data: missing name and/or ID!", this->m_errorReporting.getError()->message);
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenBothsourceAndBytecodeMissing)
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
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of LuaModule from serialized data: has neither Lua source code nor bytecode!");
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhensourceAndBytecodeEmpty)
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
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of LuaModule from serialized data: has neither Lua source code nor bytecode!");
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenDependenciesMissing)
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
                m_flatBufferBuilder.CreateVector(std::vector<uint8_t>{1, 0})
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_EQ(m_errorReporting.getError()->message, "Fatal error during loading of LuaModule from serialized data: missing dependencies!");
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenByteCodeInvalidAndNoSourceAvailable)
    {
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
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_TRUE(m_errorReporting.getError().has_value());
        EXPECT_THAT(m_errorReporting.getError()->message, ::testing::HasSubstr("Fatal error during loading of LuaModule 'name': failed loading pre-compiled byte code"));
    }

    TEST_F(ALuaModule_SerializationLifecycle, WillTryToRecompileModuleFromSourceWhenByteCodeInvalid)
    {
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
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);
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

    TEST_F(ALuaModule_SerializationLifecycle, SerializesSourceCodeOnly_InSourceOnlyMode)
    {
        auto module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(module->impl(), m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceCodeOnly);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        EXPECT_FALSE(serialized.luaByteCode());
        ASSERT_TRUE(serialized.source());
        EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());
    }

    TEST_F(ALuaModule_SerializationLifecycle, SerializesBytecodeOnly_InBytecodeOnlyMode)
    {
        auto module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(module->impl(), m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serialized.luaByteCode());
        EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
        EXPECT_FALSE(serialized.source());
    }

    TEST_F(ALuaModule_SerializationLifecycle, SerializesBytecodeOnly_InSourceCodeOnlyMode_IfNoSourceAvailable)
    {
        // simulate module with no source code by serializing it with bytecode only and deserializing again
        std::unique_ptr<LuaModuleImpl> moduleWithNoSourceCode;
        {
            auto module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");

            SerializationMap serializationMap;
            const auto fbModule = LuaModuleImpl::Serialize(module->impl(), m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
            m_flatBufferBuilder.Finish(fbModule);
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
            moduleWithNoSourceCode = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);
            ASSERT_TRUE(moduleWithNoSourceCode);
            m_flatBufferBuilder.Clear();
        }

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(*moduleWithNoSourceCode, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceCodeOnly);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serialized.luaByteCode());
        EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
        EXPECT_FALSE(serialized.source());
    }

    TEST_F(ALuaModule_SerializationLifecycle, SerializesBytecodeOnly_InBothSourceAndBytecodeMode_IfNoSourceAvailable)
    {
        // simulate module with no source code by serializing it with bytecode only and deserializing again
        std::unique_ptr<LuaModuleImpl> moduleWithNoSourceCode;
        {
            auto module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");

            SerializationMap serializationMap;
            const auto fbModule = LuaModuleImpl::Serialize(module->impl(), m_flatBufferBuilder, serializationMap, ELuaSavingMode::ByteCodeOnly);
            m_flatBufferBuilder.Finish(fbModule);
            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
            moduleWithNoSourceCode = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);
            ASSERT_TRUE(moduleWithNoSourceCode);
            m_flatBufferBuilder.Clear();
        }

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(*moduleWithNoSourceCode, m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serialized.luaByteCode());
        EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
        EXPECT_FALSE(serialized.source());
    }

    TEST_F(ALuaModule_SerializationLifecycle, SerializesBothSourceAndBytecode)
    {
        auto module = m_logicEngine->createLuaModule(m_moduleSourceCode, {}, "mymodule");

        SerializationMap serializationMap;
        const auto fbModule = LuaModuleImpl::Serialize(module->impl(), m_flatBufferBuilder, serializationMap, ELuaSavingMode::SourceAndByteCode);
        m_flatBufferBuilder.Finish(fbModule);
        const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());

        ASSERT_TRUE(serialized.source());
        EXPECT_EQ(m_moduleSourceCode, serialized.source()->string_view());

        ASSERT_TRUE(serialized.luaByteCode());
        EXPECT_TRUE(serialized.luaByteCode()->size() > 0);
    }

    TEST_F(ALuaModule_SerializationLifecycle, DoesNotContainDebugLogFunctionsAfterDeserialization)
    {
        withTempDirectory();

        {
            LogicEngine& logic = *m_logicEngine;
            logic.createLuaModule(m_moduleSourceCode, {}, "mymodule");
            EXPECT_TRUE(saveToFileWithoutValidation("module.tmp"));
        }

        ASSERT_TRUE(recreateFromFile("module.tmp"));
        const auto module = m_logicEngine->findObject<LuaModule>("mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_FALSE(module->impl().hasDebugLogFunctions());
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
        const auto quadsMod = m_logicEngine->createLuaModule(m_quadsSrc, createDeps({{"mymath", m_mathSrc}}), "quadsMod");
        ASSERT_NE(nullptr, quadsMod);
        EXPECT_EQ("quadsMod", quadsMod->getName());
        EXPECT_EQ(quadsMod->getSceneObjectId().getValue(), 10u); // module dependency has id 8u
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
        const auto mathCombined = m_logicEngine->createLuaModule(mathCombinedSrc, createDeps({ {"_mathAdd", m_mathSrc}, {"_mathSub", mathSubSrc} }));

        LuaConfig config;
        config.addDependency("_math", *mathCombined);
        const auto script = m_logicEngine->createLuaScript(R"(
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

        m_logicEngine->update();
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
        const auto mathCombined = m_logicEngine->createLuaModule(mathCombinedSrc, createDeps({ {"math1", m_mathSrc}, {"math2", m_mathSrc} }));

        LuaConfig config;
        config.addDependency("mathAll", *mathCombined);
        const auto script = m_logicEngine->createLuaScript(R"(
            modules("mathAll")

            function interface(IN,OUT)
                OUT.result = Type:Int32()
            end

            function run(IN,OUT)
                OUT.result = mathAll.add1(1,2) + mathAll.add2(100,10)
            end
        )", config);
        ASSERT_NE(nullptr, script);

        m_logicEngine->update();
        EXPECT_EQ(113, *script->getOutputs()->getChild("result")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, TwoModulesDependOnTheSameModule)
    {
        const auto config = createDeps({ {"mymath", m_mathSrc} });

        const auto mathUser1 = m_logicEngine->createLuaModule(R"(
            modules("mymath")
            local mathUser1 = {}
            function mathUser1.add(a, b)
                return mymath.add(a + 1, b + 1)
            end
            return mathUser1
        )", config);

        const auto mathUser2 = m_logicEngine->createLuaModule(R"(
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
        const auto script = m_logicEngine->createLuaScript(R"(
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

        m_logicEngine->update();
        EXPECT_EQ(5, *script->getOutputs()->getChild("result1")->get<int32_t>());
        EXPECT_EQ(23, *script->getOutputs()->getChild("result2")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, CanBeSerialized)
    {
        withTempDirectory();
        {
            LogicEngine& logic = *m_logicEngine;
            LuaConfig config;
            config.addDependency("mymath", *logic.createLuaModule(m_mathSrc, {}, "mathMod"));
            logic.createLuaModule(m_quadsSrc, config, "quadsMod");
            EXPECT_TRUE(saveToFileWithoutValidation("dep_modules.tmp"));
        }

        ASSERT_TRUE(recreateFromFile("dep_modules.tmp"));

        const LuaModule* mathMod = m_logicEngine->findObject<LuaModule>("mathMod");
        auto quadsMod = m_logicEngine->findObject<LuaModule>("quadsMod");
        ASSERT_NE(mathMod, nullptr);
        ASSERT_NE(quadsMod, nullptr);
        EXPECT_EQ(mathMod->getSceneObjectId().getValue(), 9u);
        EXPECT_EQ(quadsMod->getSceneObjectId().getValue(), 10u);

        EXPECT_THAT(quadsMod->impl().getDependencies(), ::testing::ElementsAre(std::pair<std::string, const LuaModule*>({"mymath", mathMod})));
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
        const auto quadsMod = m_logicEngine->createLuaModule(m_quadsSrc, createDeps({ {"mymath", errors} }));
        ASSERT_NE(nullptr, quadsMod);

        // This works fine, because neither the the quads module nor the math modules are ever called
        EXPECT_TRUE(m_logicEngine->update());
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
        EXPECT_EQ(nullptr, m_logicEngine->createLuaModule(mathExt, createDeps({ {"dep2", m_moduleSourceCode} })));
        EXPECT_THAT(getLastErrorMessage(), ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep2"));
    }

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ProvidedButNotDeclared)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep2")
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine->createLuaModule(mathExt, createDeps({ {"dep1", m_moduleSourceCode}, {"dep2", m_moduleSourceCode}, {"dep3", m_moduleSourceCode} })));
        EXPECT_THAT(getLastErrorMessage(), ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep1, dep2, dep3"));
    }

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ExractionError)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep1") -- duplicate dependency
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine->createLuaModule(mathExt, createDeps({ {"dep1", m_moduleSourceCode} })));
        EXPECT_THAT(getLastErrorMessage(), ::testing::HasSubstr("Error while extracting module dependencies: 'dep1' appears more than once in dependency list"));
    }
}
