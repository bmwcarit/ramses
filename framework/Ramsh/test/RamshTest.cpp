//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/Ramsh.h"
#include "Ramsh/RamshCommandArguments.h"
#include "Ramsh/RamshCommandArgumentsOptionSet.h"
#include "Ramsh/RamshCommunicationChannelConsole.h"
#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "Ramsh/RamshTools.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/RamsesLogger.h"


namespace ramses_internal
{
    using namespace testing;

    template<typename T1, typename T2, typename T3, typename T4>
    struct TestTuple
    {
        T1 a1;
        T2 a2;
        T3 a3;
        T4 a4;
    };

    template<typename T1, typename T2, typename T3, typename T4>
    struct TypedTestCommand
        : public RamshCommandArgs < T1, T2, T3, T4 >
    {
        TypedTestCommand()
            : data(new TestTuple<T1, T2, T3, T4>())
        {
        }

        ~TypedTestCommand()
        {
            delete data;
        }

        TestTuple<T1, T2, T3, T4>* data;

        using RamshCommandArgs<T1, T2, T3, T4>::getArgument;

        Bool execute(T1& arg1, T2& arg2, T3& arg3, T4& arg4) const
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
        virtual Bool executeInput(const RamshInput& /*input*/)
        {
            return true;
        }

    };

    class RamshAPI : public ::testing::Test
    {
    protected:
        Ramsh ramsh;
        DummyRamshCommand cmd;
        RamshInput input;
    };

    TEST_F(RamshAPI, printHelp)
    {
        // help is built-in command of Ramsh
        // trigger command @ Ramsh
        input.append("help");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithOneKeyword)
    {
        // register required commands @ Ramsh
        cmd.registerKeyword("ABC");
        ramsh.add(cmd);

        // trigger command @ Ramsh
        input.append("ABC");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithMultipleKeywordsUsingFirstKeyword)
    {
        // register required commands @ Ramsh
        cmd.registerKeyword("long");
        cmd.registerKeyword("thisisalongername");
        cmd.registerKeyword("X");
        ramsh.add(cmd);

        // trigger command @ Ramsh
        input.append("long");
        input.append("arg1");
        input.append("arg2");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithMultipleKeywordsUsingMiddleKeyword)
    {
        // register required commands @ Ramsh
        cmd.registerKeyword("long");
        cmd.registerKeyword("thisisalongername");
        cmd.registerKeyword("X");
        ramsh.add(cmd);

        // trigger command @ Ramsh
        input.append("thisisalongername");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerCommandWithMultipleKeywordsUsingLastKeyword)
    {
        // register required commands @ Ramsh
        cmd.registerKeyword("long");
        cmd.registerKeyword("thisisalongername");
        cmd.registerKeyword("X");
        ramsh.add(cmd);

        // trigger command @ Ramsh
        input.append("X");
        EXPECT_TRUE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, triggerInvalidCommand)
    {
        // trigger command @ Ramsh
        input.append("invalid");
        EXPECT_FALSE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, parseCommandStringTest)
    {
        String in1 = "  test    ' a b c'  -arg ''  ";
        RamshInput result1 = RamshTools::parseCommandString(in1);
        EXPECT_EQ(String("test"), result1[0]);
        EXPECT_EQ(String(" a b c"), result1[1]);
        EXPECT_EQ(String("-arg"), result1[2]);
        EXPECT_EQ(String(), result1[3]);

        String in2 = "  \"te-' 'st \" abc' ' 'def'a   ";
        RamshInput result2 = RamshTools::parseCommandString(in2);
        EXPECT_EQ(String("te-' 'st "), result2[0]);
        EXPECT_EQ(String("abc'"), result2[1]);
        EXPECT_EQ(String("'"), result2[2]);
        EXPECT_EQ(String("'def'a"), result2[3]);

        String in3 = "abc -arg \"test argument\" 'test argument' 'test \"argument\"' \"test 'argument'\"";
        RamshInput result3 = RamshTools::parseCommandString(in3);
        EXPECT_EQ(String("abc"), result3[0]);
        EXPECT_EQ(String("-arg"), result3[1]);
        EXPECT_EQ(String("test argument"), result3[2]);
        EXPECT_EQ(String("test argument"), result3[3]);
        EXPECT_EQ(String("test \"argument\""), result3[4]);
        EXPECT_EQ(String("test 'argument'"), result3[5]);
    }


    TEST_F(RamshAPI, typedCommand)
    {
        TypedTestCommand<BoolLiteral, Bool, String, Float> typedCmd;

        typedCmd.registerKeyword("typed");

        typedCmd.getArgument<0>()
            .registerKeyword("bool-literal");

        typedCmd.getArgument<1>()
            .registerKeyword("bool");

        typedCmd.getArgument<2>()
            .registerKeyword("string");

        typedCmd.getArgument<3>()
            .registerKeyword("float");

        ramsh.add(typedCmd);

        input.append("typed");

        // arguments without flags
        input.append("true");
        input.append("-bool");
        input.append("foobar");
        input.append("1.337");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_TRUE(typedCmd.data->a1);
        EXPECT_TRUE(typedCmd.data->a2);

        EXPECT_EQ(String("foobar"), typedCmd.data->a3);

        EXPECT_FLOAT_EQ(1.337f, typedCmd.data->a4);

        input.clear();

        input.append("typed");

        // scrambled arguments
        input.append("-string");
        input.append("foo");
        input.append("-bool");
        input.append("-float");
        input.append("-1337");
        input.append("-bool-literal");
        input.append("false");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_FALSE(typedCmd.data->a1);
        EXPECT_TRUE(typedCmd.data->a2);

        EXPECT_EQ(String("foo"), typedCmd.data->a3);

        EXPECT_FLOAT_EQ(-1337.f, typedCmd.data->a4);

        input.clear();

        input.append("typed");

        // invalid argument
        input.append("true");
        input.append("-bool");
        input.append("foobar");
        input.append("-floata");
        input.append("abcde");

        EXPECT_FALSE(ramsh.execute(input));

        input.clear();

        input.append("typed");

        // missing argument
        input.append("-bool");
        input.append("foobar");
        input.append("-pair");
        input.append("1.337;1,3,3,7");

        EXPECT_FALSE(ramsh.execute(input));
    }

    TEST_F(RamshAPI, typedCommandWithDefaultValues)
    {
        TypedTestCommand<BoolLiteral, Bool, String, Float> typedCmd;

        typedCmd.registerKeyword("typed");

        typedCmd.getArgument<0>()
            .registerKeyword("bool-literal")
            .setDefaultValue(false);

        typedCmd.getArgument<1>()
            .registerKeyword("bool")
            .setDefaultValue(true);

        typedCmd.getArgument<2>()
            .registerKeyword("string")
            .setDefaultValue(String("abcdef"));

        typedCmd.getArgument<3>()
            .registerKeyword("float")
            .setDefaultValue(-13.37f);

        ramsh.add(typedCmd);

        // no input, just default values
        input.append("typed");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_FALSE(typedCmd.data->a1);
        EXPECT_TRUE(typedCmd.data->a2);

        EXPECT_EQ(String("abcdef"), typedCmd.data->a3);

        EXPECT_FLOAT_EQ(-13.37f, typedCmd.data->a4);

        input.clear();

        // flipped bool
        input.append("typed");
        input.append("-bool");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_FALSE(typedCmd.data->a1);
        EXPECT_FALSE(typedCmd.data->a2);

        EXPECT_EQ(String("abcdef"), typedCmd.data->a3);

        EXPECT_FLOAT_EQ(-13.37f, typedCmd.data->a4);

        input.clear();

        // partially set arguments
        input.append("typed");

        input.append("-float");
        input.append("-1337.1337");
        input.append("-string");
        input.append("foo");
        input.append("-bool-literal");
        input.append("false");

        EXPECT_TRUE(ramsh.execute(input));

        EXPECT_FALSE(typedCmd.data->a1);
        EXPECT_TRUE(typedCmd.data->a2);

        EXPECT_EQ(String("foo"), typedCmd.data->a3);

        EXPECT_FLOAT_EQ(-1337.1337f, typedCmd.data->a4);
    }

    class MockRamsh : public Ramsh
    {
    public:
        MOCK_METHOD1(execute, Bool(RamshInput& input));
    };

    class RamshCommunicationChannelConsoleTest : public ::testing::Test
    {
    public:
        RamshCommunicationChannelConsoleTest()
        {
            inputProvider.registerRamsh(ramsh);
        }

    protected:
        NiceMock<MockRamsh> ramsh;
        RamshCommunicationChannelConsole inputProvider;
    };

    class RamshCommunicationChannelConsoleTestThread : public ramses_internal::Runnable
    {
    public:
        explicit RamshCommunicationChannelConsoleTestThread()
        {
            GetRamsesLogger();
        }

        ~RamshCommunicationChannelConsoleTestThread()
        {
        }

        virtual void run()
        {
            logSomeMessage();
        }

    protected:
        void logSomeMessage()
        {
            StringOutputStream stream;
            stream << "This is some very interesting log message used for testing purpose";
            LogMessage message(CONTEXT_RAMSH, ELogLevel::Info, stream);
            GetRamsesLogger().log(message);
        }
    };

    TEST_F(RamshCommunicationChannelConsoleTest, processInput)
    {
        String input = "help\nbla";
        RamshInput expectedRamshInput;
        expectedRamshInput.append("help"); //read stops at enter

        EXPECT_CALL(ramsh, execute(Eq(expectedRamshInput))).Times(1);

        for (UInt i = 0; i < input.getLength(); i++)
        {
            inputProvider.processInput(input[i]);
        }
    }

    // The test does not contain any expectation, it's purpose is to
    // trigger a problematic deadlock behaviour which can be found by
    // thread_sanitizer
    TEST_F(RamshCommunicationChannelConsoleTest, checkPossibleLogDeadlock)
    {
        ramses_internal::PlatformThread thread("RamshConTest");
        RamshCommunicationChannelConsoleTestThread runnable;

        thread.start(runnable);

        String input = "help\n";
        for (UInt i = 0; i < input.getLength(); i++)
        {
            inputProvider.processInput(input[i]);
        }

        thread.join();
    }
}
