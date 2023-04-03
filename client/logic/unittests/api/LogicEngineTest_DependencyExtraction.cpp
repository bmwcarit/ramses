//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

namespace rlogic
{
    class ALogicEngine_DependencyExtraction : public ALogicEngine
    {
    protected:
        void extractAndExpect(std::string_view src, const std::vector<std::string>& expected)
        {
            std::vector<std::string> extractedDependencies;
            std::function<void(const std::string&)> callbackFunc = [&extractedDependencies](const std::string& dep) {
                extractedDependencies.push_back(dep);
            };

            ASSERT_TRUE(m_logicEngine.extractLuaDependencies(src, callbackFunc));
            EXPECT_EQ(expected, extractedDependencies);
            EXPECT_TRUE(m_logicEngine.getErrors().empty());
        }

        void extractAndExpectError(std::string_view src, std::string_view errorMsgSubstr)
        {
            std::function<void(const std::string&)> callbackFunc = [](const std::string& /*unused*/) {};
            EXPECT_FALSE(m_logicEngine.extractLuaDependencies(src, callbackFunc));
            ASSERT_EQ(1u, m_logicEngine.getErrors().size());
            EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr(errorMsgSubstr));
        }
    };

    TEST_F(ALogicEngine_DependencyExtraction, extractsModules)
    {
        constexpr std::string_view src = R"(
                modules("foo", "bar")
                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                function run(IN,OUT)
                    bla.bla = bla
                end
            )";

        extractAndExpect(src, { "foo", "bar" });
    }

    TEST_F(ALogicEngine_DependencyExtraction, extractsEmptyModulesIfModulesNotSpecifiedInSource)
    {
        constexpr std::string_view src = R"(
                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                function run(IN,OUT)
                    bla.bla = bla
                end
            )";

        extractAndExpect(src, {});
    }

    TEST_F(ALogicEngine_DependencyExtraction, extractsEmptyModulesIfModulesSpecifiedInScriptButEmpty)
    {
        constexpr std::string_view src = R"(
                modules()
                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                function run(IN,OUT)
                    bla.bla = bla
                end
            )";

        extractAndExpect(src, {});
    }

    TEST_F(ALogicEngine_DependencyExtraction, failsExtractModulesIfWrongArguments)
    {
        constexpr std::string_view src = R"(
                modules("foo", 5, "bar")
                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                function run(IN,OUT)
                    bla.bla = bla
                end
            )";

        extractAndExpectError(src, R"(Error while extracting module dependencies: argument 1 is of type 'number', string must be provided: ex. 'modules("moduleA", "moduleB")')");
    }

    TEST_F(ALogicEngine_DependencyExtraction, failsExtractModulesIfWrongSyntax)
    {
        constexpr std::string_view src = R"(
                modules("foo",
                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                function run(IN,OUT)
                    bla.bla = bla
                end
            )";

        extractAndExpectError(src, "'(' expected near");
    }

    TEST_F(ALogicEngine_DependencyExtraction, extractsModulesEvenIfUnresolvedLabelUsedInGlobalSpace)
    {
        constexpr std::string_view src = R"(
                modules("foo", "bar")

                foo.bla(bar.bla)  -- ILLEGAL

                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                function run(IN,OUT)
                    bla.bla = bla
                end
            )";

        extractAndExpect(src, { "foo", "bar" });
    }

    TEST_F(ALogicEngine_DependencyExtraction, extractsEmptyModulesIfUnresolvedLabelUsedBeforeModulesDeclaration)
    {
        constexpr std::string_view src = R"(
                foo.bla(bar.bla)  -- ILLEGAL

                modules("foo", "bar")  -- modules should be declared before code

                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                function run(IN,OUT)
                    bla.bla = bla
                end
            )";

        // will not fail but will not extract modules
        extractAndExpect(src, {});
    }

    TEST_F(ALogicEngine_DependencyExtraction, failsExtractModulesIfDupliciteModulesUsed)
    {
        extractAndExpectError(R"(modules("foo", "foo"))", "Error while extracting module dependencies: 'foo' appears more than once in dependency list");
        extractAndExpectError(R"(modules("foo", "bar", "foo"))", "Error while extracting module dependencies: 'foo' appears more than once in dependency list");
        extractAndExpectError(R"(modules("bar", "foo", "duck", "foo", "pigeon"))", "Error while extracting module dependencies: 'foo' appears more than once in dependency list");
    }

    TEST_F(ALogicEngine_DependencyExtraction, failsExtractModulesIfModulesMoreThanOnceInExecutedCode)
    {
        constexpr std::string_view src = R"(
                modules("foo", "bar")
                function interface(IN,OUT)
                    bla.bla = Type:Int32()
                end
                modules("foo2", "bar2")
                function run(IN,OUT)
                    bla.bla = bla
                end
                modules("foo3", "bar3")
            )";

        extractAndExpectError(src, "Error while extracting module dependencies: 'modules' function was executed more than once");
    }

    TEST_F(ALogicEngine_DependencyExtraction, extractsModulesFromModuleUsingAnotherModule)
    {
        constexpr std::string_view src = R"(
                modules("foo")
                local mymath = {}
                function mymath.doSth(x)
                    return foo.compute(x)
                end
                mymath.pi = foo.sqrt(2)
                return mymath
            )";

        extractAndExpect(src, { "foo" });
    }

    TEST_F(ALogicEngine_DependencyExtraction, extractsModulesFromLuaInterfaceSource)
    {
        constexpr std::string_view src = R"(
                modules("foo", "bar")
                function interface(IN)
                    IN.bla = foo.type
                end
            )";

        extractAndExpect(src, { "foo", "bar" });
    }
}
