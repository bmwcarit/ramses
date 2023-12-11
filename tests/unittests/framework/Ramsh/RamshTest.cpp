//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Ramsh/Ramsh.h"
#include "internal/Ramsh/RamshCommandArguments.h"
#include "internal/Ramsh/RamshCommunicationChannelConsole.h"
#include "internal/Ramsh/RamshCommandExit.h"
#include "internal/Ramsh/RamshCommandArgumentsConverter.h"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include "internal/Ramsh/RamshTools.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "impl/RamsesLoggerImpl.h"

namespace ramses::internal
{
    using namespace testing;

    template<typename T1, typename T2, typename T3, typename T4>
    struct TestTuple
    {
        T1 a1{};
        T2 a2{};
        T3 a3{};
        T4 a4{};
    };

    template<typename T1, typename T2, typename T3, typename T4>
    struct TypedTestCommand
        : public RamshCommandArgs < T1, T2, T3, T4 >
    {
        TypedTestCommand()
            : data(new TestTuple<T1, T2, T3, T4>())
        {
        }

        ~TypedTestCommand() override
        {
            delete data;
        }

        TestTuple<T1, T2, T3, T4>* data;

        using RamshCommandArgs<T1, T2, T3, T4>::getArgument;

        bool execute(T1& arg1, T2& arg2, T3& arg3, T4& arg4) const override
        {
            data->a1 = arg1;
            data->a2 = arg2;
            data->a3 = arg3;
            data->a4 = arg4;
            return true;
        }
    };

    class DummyRamshCommand : public RamshCommand
    {
        bool executeInput(const std::vector<std::string>& /*input*/) override
        {
            return true;
        }

    };

    class RamshAPI : public ::testing::Test
    {
    protected:
        Ramsh ramsh;
        std::shared_ptr<DummyRamshCommand> cmd = std::make_shared<DummyRamshCommand>();
        std::vector<std::string> input;
    };

    TEST_F(RamshAPI, printHelp)
    {
        // help is built-in command of Ramsh
        // trigger command @ Ramsh
        input.emplace_back("help");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithOneKeyword)
    {
        // register required commands @ Ramsh
        cmd->registerKeyword("ABC");
        EXPECT_TRUE(ramsh.add(cmd));

        // trigger command @ Ramsh
        input.emplace_back("ABC");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithMultipleKeywordsUsingFirstKeyword)
    {
        // register required commands @ Ramsh
        cmd->registerKeyword("long");
        cmd->registerKeyword("thisisalongername");
        cmd->registerKeyword("X");
        EXPECT_TRUE(ramsh.add(cmd));

        // trigger command @ Ramsh
        input.emplace_back("long");
        input.emplace_back("arg1");
        input.emplace_back("arg2");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithMultipleKeywordsUsingMiddleKeyword)
    {
        // register required commands @ Ramsh
        cmd->registerKeyword("long");
        cmd->registerKeyword("thisisalongername");
        cmd->registerKeyword("X");
        EXPECT_TRUE(ramsh.add(cmd));

        // trigger command @ Ramsh
        input.emplace_back("thisisalongername");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithMultipleKeywordsUsingLastKeyword)
    {
        // register required commands @ Ramsh
        cmd->registerKeyword("long");
        cmd->registerKeyword("thisisalongername");
        cmd->registerKeyword("X");
        EXPECT_TRUE(ramsh.add(cmd));

        // trigger command @ Ramsh
        input.emplace_back("X");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerInvalidCommand)
    {
        // trigger command @ Ramsh
        input.emplace_back("invalid");
        EXPECT_FALSE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, parseCommandStringTest)
    {
        std::string in1 = "  test    ' a b c'  -arg ''  ";
        std::vector<std::string> result1 = RamshTools::parseCommandString(in1);
        EXPECT_EQ(std::string("test"), result1[0]);
        EXPECT_EQ(std::string(" a b c"), result1[1]);
        EXPECT_EQ(std::string("-arg"), result1[2]);
        EXPECT_EQ(std::string(), result1[3]);

        std::string in2 = R"(  "te-' 'st " abc' ' 'def'a   )";
        std::vector<std::string> result2 = RamshTools::parseCommandString(in2);
        EXPECT_EQ(std::string("te-' 'st "), result2[0]);
        EXPECT_EQ(std::string("abc'"), result2[1]);
        EXPECT_EQ(std::string("'"), result2[2]);
        EXPECT_EQ(std::string("'def'a"), result2[3]);

        std::string in3 = R"(abc -arg "test argument" 'test argument' 'test "argument"' "test 'argument'")";
        std::vector<std::string> result3 = RamshTools::parseCommandString(in3);
        EXPECT_EQ(std::string("abc"), result3[0]);
        EXPECT_EQ(std::string("-arg"), result3[1]);
        EXPECT_EQ(std::string("test argument"), result3[2]);
        EXPECT_EQ(std::string("test argument"), result3[3]);
        EXPECT_EQ(std::string("test \"argument\""), result3[4]);
        EXPECT_EQ(std::string("test 'argument'"), result3[5]);
    }

    TEST_F(RamshAPI, parseUInt16)
    {
        uint16_t value = 0;
        EXPECT_TRUE(ArgumentConverter<uint16_t>::tryConvert("-1", value));
        EXPECT_EQ(std::numeric_limits<uint16_t>::max(), value);
        EXPECT_TRUE(ArgumentConverter<uint16_t>::tryConvert("0", value));
        EXPECT_EQ(0u, value);
        EXPECT_TRUE(ArgumentConverter<uint16_t>::tryConvert("65535", value));
        EXPECT_EQ(std::numeric_limits<uint16_t>::max(), value);
    }

    TEST_F(RamshAPI, parseInt16)
    {
        int16_t value = 0;
        EXPECT_TRUE(ArgumentConverter<int16_t>::tryConvert("-1", value));
        EXPECT_EQ(-1, value);
        EXPECT_TRUE(ArgumentConverter<int16_t>::tryConvert("0", value));
        EXPECT_EQ(0, value);
        EXPECT_TRUE(ArgumentConverter<int16_t>::tryConvert("32767", value));
        EXPECT_EQ(std::numeric_limits<int16_t>::max(), value);
        EXPECT_TRUE(ArgumentConverter<int16_t>::tryConvert("-32768", value));
        EXPECT_EQ(std::numeric_limits<int16_t>::min(), value);
    }

    TEST_F(RamshAPI, parseUInt32)
    {
        uint32_t value = 0;
        EXPECT_TRUE(ArgumentConverter<uint32_t>::tryConvert("-1", value));
        EXPECT_EQ(std::numeric_limits<uint32_t>::max(), value);
        EXPECT_TRUE(ArgumentConverter<uint32_t>::tryConvert("0", value));
        EXPECT_EQ(0u, value);
        EXPECT_TRUE(ArgumentConverter<uint32_t>::tryConvert("4294967295", value));
        EXPECT_EQ(std::numeric_limits<uint32_t>::max(), value);
    }

    TEST_F(RamshAPI, parseInt32)
    {
        int32_t value = 0;
        EXPECT_TRUE(ArgumentConverter<int32_t>::tryConvert("-1", value));
        EXPECT_EQ(-1, value);
        EXPECT_TRUE(ArgumentConverter<int32_t>::tryConvert("0", value));
        EXPECT_EQ(0, value);
        EXPECT_TRUE(ArgumentConverter<int32_t>::tryConvert("2147483647", value));
        EXPECT_EQ(std::numeric_limits<int32_t>::max(), value);
        EXPECT_TRUE(ArgumentConverter<int32_t>::tryConvert("-2147483648", value));
        EXPECT_EQ(std::numeric_limits<int32_t>::min(), value);
    }

    TEST_F(RamshAPI, parseUInt64)
    {
        uint64_t value = 0;
        EXPECT_TRUE(ArgumentConverter<uint64_t>::tryConvert("-1", value));
        EXPECT_EQ(std::numeric_limits<uint64_t>::max(), value);
        EXPECT_TRUE(ArgumentConverter<uint64_t>::tryConvert("0", value));
        EXPECT_EQ(0u, value);
        EXPECT_TRUE(ArgumentConverter<uint64_t>::tryConvert("18446744073709551615", value));
        EXPECT_EQ(std::numeric_limits<uint64_t>::max(), value);
    }

    TEST_F(RamshAPI, parseInt64)
    {
        int64_t value = 0;
        EXPECT_TRUE(ArgumentConverter<int64_t>::tryConvert("-1", value));
        EXPECT_EQ(-1, value);
        EXPECT_TRUE(ArgumentConverter<int64_t>::tryConvert("0", value));
        EXPECT_EQ(0, value);
        EXPECT_TRUE(ArgumentConverter<int64_t>::tryConvert("9223372036854775807", value));
        EXPECT_EQ(std::numeric_limits<int64_t>::max(), value);
        EXPECT_TRUE(ArgumentConverter<int64_t>::tryConvert("-9223372036854775808", value));
        EXPECT_EQ(std::numeric_limits<int64_t>::min(), value);
    }

    TEST_F(RamshAPI, typedCommand)
    {
        auto typedCmd = std::make_shared<TypedTestCommand<uint32_t, bool, std::string, float>>();

        typedCmd->registerKeyword("typed");

        typedCmd->getArgument<0>()
            .registerKeyword("int");

        typedCmd->getArgument<1>()
            .registerKeyword("bool");

        typedCmd->getArgument<2>()
            .registerKeyword("string");

        typedCmd->getArgument<3>()
            .registerKeyword("float");

        EXPECT_TRUE(ramsh.add(typedCmd));

        input.emplace_back("typed");

        // arguments without flags
        input.emplace_back("44");
        input.emplace_back("-bool");
        input.emplace_back("foobar");
        input.emplace_back("1.337");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_EQ(44u, typedCmd->data->a1);
        EXPECT_TRUE(typedCmd->data->a2);

        EXPECT_EQ(std::string("foobar"), typedCmd->data->a3);

        EXPECT_FLOAT_EQ(1.337f, typedCmd->data->a4);

        input.clear();

        input.emplace_back("typed");

        // scrambled arguments
        input.emplace_back("-string");
        input.emplace_back("foo");
        input.emplace_back("-bool");
        input.emplace_back("-float");
        input.emplace_back("-1337");
        input.emplace_back("-int");
        input.emplace_back("123");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_EQ(123u, typedCmd->data->a1);
        EXPECT_TRUE(typedCmd->data->a2);

        EXPECT_EQ(std::string("foo"), typedCmd->data->a3);

        EXPECT_FLOAT_EQ(-1337.f, typedCmd->data->a4);

        input.clear();

        input.emplace_back("typed");

        // invalid argument
        input.emplace_back("true");
        input.emplace_back("-bool");
        input.emplace_back("foobar");
        input.emplace_back("-floata");
        input.emplace_back("abcde");

        EXPECT_FALSE(ramsh.execute(input));

        input.clear();

        input.emplace_back("typed");

        // missing argument
        input.emplace_back("-bool");
        input.emplace_back("foobar");
        input.emplace_back("-pair");
        input.emplace_back("1.337;1,3,3,7");

        EXPECT_FALSE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, typedCommandWithDefaultValues)
    {
        auto typedCmd = std::make_shared<TypedTestCommand<int32_t, bool, std::string, float>>();

        typedCmd->registerKeyword("typed");

        typedCmd->getArgument<0>()
            .registerKeyword("int")
            .setDefaultValue(-1);

        typedCmd->getArgument<1>()
            .registerKeyword("bool")
            .setDefaultValue(true);

        typedCmd->getArgument<2>()
            .registerKeyword("string")
            .setDefaultValue(std::string("abcdef"));

        typedCmd->getArgument<3>()
            .registerKeyword("float")
            .setDefaultValue(-13.37f);

        EXPECT_TRUE(ramsh.add(typedCmd));

        // no input, just default values
        input.emplace_back("typed");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_EQ(-1, typedCmd->data->a1);
        EXPECT_TRUE(typedCmd->data->a2);

        EXPECT_EQ(std::string("abcdef"), typedCmd->data->a3);

        EXPECT_FLOAT_EQ(-13.37f, typedCmd->data->a4);

        input.clear();

        // flipped bool
        input.emplace_back("typed");
        input.emplace_back("-bool");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_EQ(-1, typedCmd->data->a1);
        EXPECT_FALSE(typedCmd->data->a2);

        EXPECT_EQ(std::string("abcdef"), typedCmd->data->a3);

        EXPECT_FLOAT_EQ(-13.37f, typedCmd->data->a4);

        input.clear();

        // partially set arguments
        input.emplace_back("typed");

        input.emplace_back("-float");
        input.emplace_back("-1337.1337");
        input.emplace_back("-string");
        input.emplace_back("foo");
        input.emplace_back("-int");
        input.emplace_back("90000");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_EQ(90000, typedCmd->data->a1);
        EXPECT_TRUE(typedCmd->data->a2);

        EXPECT_EQ(std::string("foo"), typedCmd->data->a3);

        EXPECT_FLOAT_EQ(-1337.1337f, typedCmd->data->a4);
    }

    TEST_F(RamshAPI, commandIsNotAccessedAnymoreWhenSharedPtrGone)
    {
        struct RefCommand : public RamshCommand
        {
            explicit RefCommand(bool& b_)
                : b(b_)
            {}

            bool executeInput(const std::vector<std::string>& /*input*/) override
            {
                b = true;
                return true;
            }

            bool& b;
        };

        bool value = false;
        input.emplace_back("c");
        {
            auto refCmd = std::make_shared<RefCommand>(value);
            refCmd->registerKeyword("c");
            EXPECT_TRUE(ramsh.add(refCmd));

            EXPECT_FALSE(value);
            EXPECT_TRUE(ramsh.execute(input));
            EXPECT_TRUE(value);
            value = false;
        }
        EXPECT_FALSE(ramsh.execute(input));
        EXPECT_FALSE(value);
    }

    class MockRamsh : public Ramsh
    {
    public:
        MOCK_METHOD(bool, execute, (const std::vector<std::string>& input), (override));
    };

    class RamshCommunicationChannelConsoleTest : public ::testing::Test
    {
    public:
        RamshCommunicationChannelConsoleTest()
            : inputProvider(RamshCommunicationChannelConsole::Construct(ramsh, "noname", false))
        {
        }

    protected:
        NiceMock<MockRamsh> ramsh;
        std::unique_ptr<RamshCommunicationChannelConsole> inputProvider;
    };

    class RamshCommunicationChannelConsoleTestThread : public Runnable
    {
    public:
        explicit RamshCommunicationChannelConsoleTestThread()
        {
            GetRamsesLogger();
        }

        ~RamshCommunicationChannelConsoleTestThread() override = default;

        void run() override
        {
            LogSomeMessage();
        }

    protected:
        static void LogSomeMessage()
        {
            std::string s = "This is some very interesting log message used for testing purpose";
            GetRamsesLogger().log(LogMessage{ CONTEXT_RAMSH, ELogLevel::Info, std::move(s) });
        }
    };

    TEST_F(RamshCommunicationChannelConsoleTest, processInput)
    {
        std::string input = "help\nbla";
        EXPECT_CALL(ramsh, execute(std::vector<std::string>{"help"}));

        for (char i : input)
        {
            inputProvider->processInput(i);
        }
    }

    // The test does not contain any expectation, it's purpose is to
    // trigger a problematic deadlock behaviour which can be found by
    // thread_sanitizer
    TEST_F(RamshCommunicationChannelConsoleTest, checkPossibleLogDeadlock)
    {
        PlatformThread thread("RamshConTest");
        RamshCommunicationChannelConsoleTestThread runnable;

        thread.start(runnable);

        std::string input = "help\n";
        for (char i : input)
        {
            inputProvider->processInput(i);
        }

        thread.join();
    }

    class ARamshAsyncTester : public ::testing::Test
    {
    public:
        static std::vector<std::string> CreateInput(std::initializer_list<const char*> inpList)
        {
            std::vector<std::string> input;
            for (const auto& inp : inpList)
                input.emplace_back(inp);
            return input;
        }

        void TearDown() override
        {
            // reset loglevels back to default
            RamsesLoggerConfig cfg;
            cfg.dltAppId = "RAMS";
            cfg.dltAppDescription = "ramses";
            GetRamsesLogger().initialize(cfg, false, true);
        }

        Ramsh rsh;
    };

    TEST_F(ARamshAsyncTester, canRunSideEffectCommands)
    {
        std::thread([&]() {
            rsh.execute(CreateInput({"help"}));
            rsh.execute(CreateInput({"buildConfig"}));
            rsh.execute(CreateInput({"ramsesVersion"}));
            rsh.execute(CreateInput({"printLogLevels"}));
        }).join();
    }

    TEST_F(ARamshAsyncTester, canSetConsoleLogLevel)
    {
        std::thread([&]() {
            rsh.execute(CreateInput({"setLogLevelConsole", "trace"}));
            EXPECT_EQ(ELogLevel::Trace, GetRamsesLogger().getConsoleLogLevel());
        }).join();
    }

    TEST_F(ARamshAsyncTester, canSetContextLogLevel)
    {
        std::thread([&]() {
            rsh.execute(CreateInput({"setContextLogLevel", "trace"}));
            EXPECT_EQ(ELogLevel::Trace, CONTEXT_FRAMEWORK.getLogLevel());
        }).join();
    }

    TEST_F(ARamshAsyncTester, canSetContextLogLevelFilter)
    {
        std::thread([&]() {
            rsh.execute(CreateInput({"setContextLogLevelFilter", "trace:RFRA,debug:RCLI"}));
            EXPECT_EQ(ELogLevel::Trace, CONTEXT_FRAMEWORK.getLogLevel());
            EXPECT_EQ(ELogLevel::Debug, CONTEXT_CLIENT.getLogLevel());
        }).join();
    }

    TEST_F(ARamshAsyncTester, canTriggerExitCommand)
    {
        auto cmdExit = std::make_shared<RamshCommandExit>();
        rsh.add(cmdExit);
        EXPECT_FALSE(cmdExit->exitRequested());
        std::thread t([&]() {
            rsh.execute(CreateInput({"exit"}));
        });

        cmdExit->waitForExitRequest();
        EXPECT_TRUE(cmdExit->exitRequested());

        t.join();
    }

    TEST_F(RamshAPI, addCommandWithInvalidKeywordFails)
    {
        std::shared_ptr<DummyRamshCommand> cmd_1 = std::make_shared<DummyRamshCommand>();
        EXPECT_FALSE(ramsh.add(cmd_1));

        std::shared_ptr<DummyRamshCommand> cmd_2 = std::make_shared<DummyRamshCommand>();
        cmd->registerKeyword("");
        EXPECT_FALSE(ramsh.add(cmd_2));

        std::shared_ptr<DummyRamshCommand> cmd_3 = std::make_shared<DummyRamshCommand>();
        cmd->registerKeyword("a b");
        EXPECT_FALSE(ramsh.add(cmd_3));

        std::shared_ptr<DummyRamshCommand> cmd_4 = std::make_shared<DummyRamshCommand>();
        cmd->registerKeyword("a\nb");
        EXPECT_FALSE(ramsh.add(cmd_4));
    }

    TEST_F(RamshAPI, addingCommandWithDuplicateKeywordsFailsUnlessRequested)
    {
        cmd->registerKeyword("foo");
        EXPECT_TRUE(ramsh.add(cmd));

        std::shared_ptr<DummyRamshCommand> otherCmd = std::make_shared<DummyRamshCommand>();
        otherCmd->registerKeyword("foo");
        EXPECT_FALSE(ramsh.add(otherCmd, false));
    }

    TEST_F(RamshAPI, addingCommandWithUnusualCharactersWorks)
    {
        cmd->registerKeyword("a;?foo#%$&!~");
        EXPECT_TRUE(ramsh.add(cmd));
    }
}
