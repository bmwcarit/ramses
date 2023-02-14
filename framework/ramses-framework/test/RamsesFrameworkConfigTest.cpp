//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "RamsesFrameworkConfigImpl.h"
#include "CLI/CLI.hpp"

using namespace ramses;
using namespace ramses_internal;

class ARamsesFrameworkConfig : public testing::Test
{
protected:
    RamsesFrameworkConfig frameworkConfig;
};

TEST_F(ARamsesFrameworkConfig, IsInitializedCorrectly)
{
    EXPECT_EQ(EFeatureLevel_01, frameworkConfig.getFeatureLevel());
    EXPECT_EQ(ERamsesShellType_Default, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetFeatureLevel)
{
    EXPECT_EQ(StatusOK, frameworkConfig.setFeatureLevel(EFeatureLevel_Latest));
    EXPECT_EQ(EFeatureLevel_Latest, frameworkConfig.getFeatureLevel());
}

TEST_F(ARamsesFrameworkConfig, FailsToSetUnsupportedFeatureLevel)
{
    EXPECT_NE(StatusOK, frameworkConfig.setFeatureLevel(static_cast<EFeatureLevel>(99)));
    EXPECT_EQ(EFeatureLevel_01, frameworkConfig.getFeatureLevel());
}

TEST_F(ARamsesFrameworkConfig, CanSetShellConsoleType)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Console);
    EXPECT_EQ(ERamsesShellType_Console, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetShellTypeNone)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_None);
    EXPECT_EQ(ERamsesShellType_None, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetShellTypeDefault)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Default);
    EXPECT_EQ(ERamsesShellType_Default, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanEnableRamshFromCommandLine)
{
    const char* args[] = { "framework", "--ramsh" };
    RamsesFrameworkConfig config(2, args);
    EXPECT_EQ(ERamsesShellType_Console, config.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, TestSetandGetApplicationInformation)
{
    const char* application_id = "myap";
    const char* application_description = "mydescription";

    frameworkConfig.setDLTApplicationID(application_id);
    frameworkConfig.setDLTApplicationDescription(application_description);

    EXPECT_STREQ(application_id, frameworkConfig.getDLTApplicationID());
    EXPECT_STREQ(application_description, frameworkConfig.getDLTApplicationDescription());
}

TEST_F(ARamsesFrameworkConfig, cliConnection)
{
    EXPECT_EQ(EConnectionProtocol::TCP, frameworkConfig.impl.getUsedProtocol());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--connection"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--connection=off"});
    EXPECT_EQ(EConnectionProtocol::Fake, frameworkConfig.impl.getUsedProtocol());
    cli.parse(std::vector<std::string>{"--connection=tcp"});
    EXPECT_EQ(EConnectionProtocol::TCP, frameworkConfig.impl.getUsedProtocol());
}

TEST_F(ARamsesFrameworkConfig, cliRamsh)
{
    EXPECT_EQ(ERamsesShellType::ERamsesShellType_Default, frameworkConfig.impl.m_shellType);
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    cli.parse(std::vector<std::string>{"--ramsh"});
    EXPECT_EQ(ERamsesShellType::ERamsesShellType_Console, frameworkConfig.impl.m_shellType);
    cli.parse(std::vector<std::string>{"--no-ramsh"});
    EXPECT_EQ(ERamsesShellType::ERamsesShellType_None, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, cliGuid)
{
    EXPECT_EQ(Guid(), frameworkConfig.impl.getUserProvidedGuid());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--guid"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--guid=foo"});
    EXPECT_EQ(Guid("foo"), frameworkConfig.impl.getUserProvidedGuid());
}

TEST_F(ARamsesFrameworkConfig, cliIp)
{
    EXPECT_EQ("127.0.0.1", frameworkConfig.impl.m_tcpConfig.getIPAddress());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ip"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--ip=localhost"});
    EXPECT_EQ("localhost", frameworkConfig.impl.m_tcpConfig.getIPAddress());
}

TEST_F(ARamsesFrameworkConfig, cliPort)
{
    EXPECT_EQ(0u, frameworkConfig.impl.m_tcpConfig.getPort());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--port"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--port=8937"});
    EXPECT_EQ(8937u, frameworkConfig.impl.m_tcpConfig.getPort());
}

TEST_F(ARamsesFrameworkConfig, cliDaemonIp)
{
    EXPECT_EQ("127.0.0.1", frameworkConfig.impl.m_tcpConfig.getDaemonIPAddress());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--daemon-ip"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--daemon-ip=localhost"});
    EXPECT_EQ("localhost", frameworkConfig.impl.m_tcpConfig.getDaemonIPAddress());
}

TEST_F(ARamsesFrameworkConfig, cliDaemonPort)
{
    EXPECT_EQ(5999u, frameworkConfig.impl.m_tcpConfig.getDaemonPort());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--daemon-port"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--daemon-port=8937"});
    EXPECT_EQ(8937u, frameworkConfig.impl.m_tcpConfig.getDaemonPort());
}

TEST_F(ARamsesFrameworkConfig, cliTcpAlive)
{
    EXPECT_EQ(std::chrono::milliseconds(300u), frameworkConfig.impl.m_tcpConfig.getAliveInterval());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--tcp-alive"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--tcp-alive=500"});
    EXPECT_EQ(std::chrono::milliseconds(500u), frameworkConfig.impl.m_tcpConfig.getAliveInterval());
}

TEST_F(ARamsesFrameworkConfig, cliTcpAliveTimeout)
{
    EXPECT_EQ(std::chrono::milliseconds(1800u), frameworkConfig.impl.m_tcpConfig.getAliveTimeout());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--tcp-alive-timeout"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--tcp-alive-timeout=5000"});
    EXPECT_EQ(std::chrono::milliseconds(5000u), frameworkConfig.impl.m_tcpConfig.getAliveTimeout());
}

TEST_F(ARamsesFrameworkConfig, cliPeriodicLogs)
{
    EXPECT_TRUE(frameworkConfig.impl.m_periodicLogsEnabled);
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    cli.parse(std::vector<std::string>{"--no-logp"});
    EXPECT_FALSE(frameworkConfig.impl.m_periodicLogsEnabled);
    cli.parse(std::vector<std::string>{"--logp"});
    EXPECT_TRUE(frameworkConfig.impl.m_periodicLogsEnabled);
}

TEST_F(ARamsesFrameworkConfig, cliPeriodicLogTimeout)
{
    EXPECT_EQ(2u, frameworkConfig.impl.periodicLogTimeout);
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--periodic-log-timeout"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--periodic-log-timeout=27"});
    EXPECT_EQ(27u, frameworkConfig.impl.periodicLogTimeout);
}

TEST_F(ARamsesFrameworkConfig, cliLogLevel)
{
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level=foo"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--log-level=trace"});
    EXPECT_EQ(ramses_internal::ELogLevel::Trace, frameworkConfig.impl.loggerConfig.logLevel.value());
    cli.parse(std::vector<std::string>{"--log-level=1"});
    EXPECT_EQ(ramses_internal::ELogLevel::Fatal, frameworkConfig.impl.loggerConfig.logLevel.value());
}

TEST_F(ARamsesFrameworkConfig, cliLogLevelConsole)
{
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level-console"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level-console=foo"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--log-level-console=trace"});
    EXPECT_EQ(ramses_internal::ELogLevel::Trace, frameworkConfig.impl.loggerConfig.logLevelConsole.value());
    cli.parse(std::vector<std::string>{"--log-level-console=1"});
    EXPECT_EQ(ramses_internal::ELogLevel::Fatal, frameworkConfig.impl.loggerConfig.logLevelConsole.value());
}

TEST_F(ARamsesFrameworkConfig, cliLogContexts)
{
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-contexts"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--log-contexts=foo:bar,foo:bar1"});
    EXPECT_EQ("foo:bar,foo:bar1", frameworkConfig.impl.loggerConfig.logLevelContextsStr.stdRef());
}

TEST_F(ARamsesFrameworkConfig, cliDltAppId)
{
    EXPECT_STREQ("RAMS", frameworkConfig.impl.getDLTApplicationID());
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--dlt-app-id"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--dlt-app-id=RROO"});
    EXPECT_STREQ("RROO", frameworkConfig.impl.getDLTApplicationID());
}

TEST_F(ARamsesFrameworkConfig, cliDltAppDescription)
{
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--dlt-app-description"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--dlt-app-description=foo"});
    EXPECT_STREQ("foo", frameworkConfig.impl.getDLTApplicationDescription());
}

TEST_F(ARamsesFrameworkConfig, cliEnableSmokeTestContext)
{
    EXPECT_FALSE(frameworkConfig.impl.loggerConfig.enableSmokeTestContext);
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    cli.parse(std::vector<std::string>{"--log-test"});
    EXPECT_TRUE(frameworkConfig.impl.loggerConfig.enableSmokeTestContext);
}
