//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/Property.h"

#include "WithTempDirectory.h"

namespace rlogic
{
    class ALogicEngine_ErrorHandling : public ALogicEngine
    {
    protected:
        const std::string_view m_linkable_script = R"(
            function interface(IN,OUT)
                IN.input = Type:Bool()
                OUT.output = Type:Bool()
            end
            function run(IN,OUT)
            end
        )";
    };

    TEST_F(ALogicEngine_ErrorHandling, ClearsErrorsOnCreateNewLuaScript)
    {
        auto script = m_logicEngine.createLuaScript("somefile.txt");
        ASSERT_EQ(nullptr, script);
        EXPECT_FALSE(m_logicEngine.getErrors().empty());

        script = m_logicEngine.createLuaScript(m_valid_empty_script);
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_F(ALogicEngine_ErrorHandling, ReturnsOnFirstError)
    {
        auto script = m_logicEngine.createLuaScript(m_invalid_empty_script);
        ASSERT_EQ(nullptr, script);
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
    }

    TEST_F(ALogicEngine_ErrorHandling, ClearsErrorsOnUpdate)
    {
        auto script = m_logicEngine.createLuaScript(m_invalid_empty_script);
        ASSERT_EQ(nullptr, script);
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);
    }

    TEST_F(ALogicEngine_ErrorHandling, ClearsErrorsOnCreateNewRamsesNodeBinding)
    {
        LogicEngine otherLogicEngine;
        auto        ramsesNodeBinding = otherLogicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");

        ASSERT_FALSE(m_logicEngine.destroy(*ramsesNodeBinding));

        EXPECT_FALSE(m_logicEngine.getErrors().empty());
        auto anotherNodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        EXPECT_NE(nullptr, anotherNodeBinding);
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_F(ALogicEngine_ErrorHandling, ClearsErrorsOnSaveAndLoadFromFile)
    {
        WithTempDirectory tempFolder;

        m_logicEngine.createLuaScript(m_valid_empty_script);

        // Generate error, so that we can test it's cleared by saveToFile()
        m_logicEngine.createLuaScript(m_invalid_empty_script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);

        EXPECT_TRUE(SaveToFileWithoutValidation(m_logicEngine, "logic.bin"));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);

        // Generate error, so that we can test it's cleared by loadFromFile()
        m_logicEngine.createLuaScript(m_invalid_empty_script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);

        EXPECT_TRUE(m_logicEngine.loadFromFile("logic.bin"));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);
    }

    TEST_F(ALogicEngine_ErrorHandling, ClearsErrorsOnLinkAndUnlink)
    {
        LuaScript* script1 = m_logicEngine.createLuaScript(m_linkable_script);
        LuaScript* script2 = m_logicEngine.createLuaScript(m_linkable_script);

        // Generate error, so that we can test it's cleared by link()
        m_logicEngine.createLuaScript(m_invalid_empty_script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);

        EXPECT_TRUE(m_logicEngine.link(*script1->getOutputs()->getChild("output"), *script2->getInputs()->getChild("input")));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);

        // Generate error, so that we can test it's cleared by unlink()
        m_logicEngine.createLuaScript(m_invalid_empty_script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);

        EXPECT_TRUE(m_logicEngine.unlink(*script1->getOutputs()->getChild("output"), *script2->getInputs()->getChild("input")));
        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);
    }
}

