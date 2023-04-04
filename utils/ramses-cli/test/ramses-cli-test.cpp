//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "gmock/gmock.h"
#include "ramses-cli.h"
#include "RamsesFrameworkConfigImpl.h"
#include "DisplayConfigImpl.h"
#include "RendererConfigImpl.h"

using namespace ramses;
using namespace ramses_internal;

class ARamsesFrameworkConfig : public testing::Test
{
public:
    ARamsesFrameworkConfig()
    {
        ramses::registerOptions(cli, config);
    }

protected:
    CLI::App cli;
    RamsesFrameworkConfig config;
};

class ARendererConfig : public testing::Test
{
public:
    ARendererConfig()
    {
        ramses::registerOptions(cli, config);
    }

protected:
    CLI::App cli;
    ramses::RendererConfig config;
};

class ADisplayConfig : public testing::Test
{
public:
    ADisplayConfig()
    {
        ramses::registerOptions(cli, config);
    }

protected:
    CLI::App cli;
    ramses::DisplayConfig config;
};


TEST_F(ARamsesFrameworkConfig, cliConnection)
{
    EXPECT_EQ(EConnectionProtocol::TCP, config.impl.getUsedProtocol());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--connection"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--connection=off"});
    EXPECT_EQ(EConnectionProtocol::Off, config.impl.getUsedProtocol());
    cli.parse(std::vector<std::string>{"--connection=tcp"});
    EXPECT_EQ(EConnectionProtocol::TCP, config.impl.getUsedProtocol());
}

TEST_F(ARamsesFrameworkConfig, cliRamsh)
{
    EXPECT_EQ(ERamsesShellType::ERamsesShellType_Default, config.impl.m_shellType);
    cli.parse(std::vector<std::string>{"--ramsh"});
    EXPECT_EQ(ERamsesShellType::ERamsesShellType_Console, config.impl.m_shellType);
    cli.parse(std::vector<std::string>{"--no-ramsh"});
    EXPECT_EQ(ERamsesShellType::ERamsesShellType_None, config.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, cliGuid)
{
    EXPECT_EQ(Guid(), config.impl.getUserProvidedGuid());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--guid"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--guid=foo"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--guid=266"});
    EXPECT_EQ(Guid(266), config.impl.getUserProvidedGuid());
}

TEST_F(ARamsesFrameworkConfig, cliIp)
{
    EXPECT_EQ("127.0.0.1", config.impl.m_tcpConfig.getIPAddress());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ip"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--ip=localhost"});
    EXPECT_EQ("localhost", config.impl.m_tcpConfig.getIPAddress());
}

TEST_F(ARamsesFrameworkConfig, cliPort)
{
    EXPECT_EQ(0u, config.impl.m_tcpConfig.getPort());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--port"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--port=8937"});
    EXPECT_EQ(8937u, config.impl.m_tcpConfig.getPort());
}

TEST_F(ARamsesFrameworkConfig, cliDaemonIp)
{
    EXPECT_EQ("127.0.0.1", config.impl.m_tcpConfig.getDaemonIPAddress());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--daemon-ip"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--daemon-ip=localhost"});
    EXPECT_EQ("localhost", config.impl.m_tcpConfig.getDaemonIPAddress());
}

TEST_F(ARamsesFrameworkConfig, cliDaemonPort)
{
    EXPECT_EQ(5999u, config.impl.m_tcpConfig.getDaemonPort());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--daemon-port"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--daemon-port=8937"});
    EXPECT_EQ(8937u, config.impl.m_tcpConfig.getDaemonPort());
}

TEST_F(ARamsesFrameworkConfig, cliTcpAlive)
{
    EXPECT_EQ(std::chrono::milliseconds(300u), config.impl.m_tcpConfig.getAliveInterval());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--tcp-alive"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--tcp-alive=500"}), CLI::ParseError);
    cli.parse("--tcp-alive=500 5000");
    EXPECT_EQ(std::chrono::milliseconds(500u), config.impl.m_tcpConfig.getAliveInterval());
    EXPECT_EQ(std::chrono::milliseconds(5000u), config.impl.m_tcpConfig.getAliveTimeout());
}

TEST_F(ARamsesFrameworkConfig, cliPeriodicLogTimeout)
{
    EXPECT_EQ(2u, config.impl.periodicLogTimeout);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--logp"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--logp=27"});
    EXPECT_EQ(27u, config.impl.periodicLogTimeout);
    EXPECT_TRUE(config.impl.m_periodicLogsEnabled);
    cli.parse(std::vector<std::string>{"--logp=0"});
    EXPECT_EQ(0u, config.impl.periodicLogTimeout);
    EXPECT_FALSE(config.impl.m_periodicLogsEnabled);
}

TEST_F(ARamsesFrameworkConfig, cliLogLevel)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level=foo"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--log-level=trace"});
    EXPECT_EQ(ramses_internal::ELogLevel::Trace, config.impl.loggerConfig.logLevel.value());
    cli.parse(std::vector<std::string>{"--log-level=1"});
    EXPECT_EQ(ramses_internal::ELogLevel::Fatal, config.impl.loggerConfig.logLevel.value());
    cli.parse(std::vector<std::string>{"-linfo"});
    EXPECT_EQ(ramses_internal::ELogLevel::Info, config.impl.loggerConfig.logLevel.value());
    cli.parse(std::vector<std::string>{"-l0"});
    EXPECT_EQ(ramses_internal::ELogLevel::Off, config.impl.loggerConfig.logLevel.value());
}

TEST_F(ARamsesFrameworkConfig, cliLogLevelConsole)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level-console"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-level-console=foo"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--log-level-console=trace"});
    EXPECT_EQ(ramses_internal::ELogLevel::Trace, config.impl.loggerConfig.logLevelConsole.value());
    cli.parse(std::vector<std::string>{"--log-level-console=1"});
    EXPECT_EQ(ramses_internal::ELogLevel::Fatal, config.impl.loggerConfig.logLevelConsole.value());
}

TEST_F(ARamsesFrameworkConfig, cliLogContexts)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-context"}), CLI::ParseError);
    EXPECT_THAT([&]() { cli.parse(std::vector<std::string>{"--log-context=badformat"}); },
                testing::ThrowsMessage<CLI::ParseError>(testing::HasSubstr("':' missing. Expected: CONTEXT:LOGLEVEL")));
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--log-context=ctx:42"}), CLI::ParseError);
    cli.parse("--log-context=ctx1:trace");
    EXPECT_EQ(ramses_internal::ELogLevel::Trace, config.impl.loggerConfig.logLevelContexts["ctx1"]);
    cli.parse("--log-context=ctx1:trace ctx1:debug ctx2:info");
    EXPECT_EQ(ramses_internal::ELogLevel::Debug, config.impl.loggerConfig.logLevelContexts["ctx1"]);
    EXPECT_EQ(ramses_internal::ELogLevel::Info, config.impl.loggerConfig.logLevelContexts["ctx2"]);
    cli.parse("--log-context=ctx1:0 ctx2:FaTAL");
    EXPECT_EQ(ramses_internal::ELogLevel::Off, config.impl.loggerConfig.logLevelContexts["ctx1"]);
    EXPECT_EQ(ramses_internal::ELogLevel::Fatal, config.impl.loggerConfig.logLevelContexts["ctx2"]);
    cli.parse("--log-context=ctx1:trace --log-context ctx2:info");
    EXPECT_EQ(ramses_internal::ELogLevel::Trace, config.impl.loggerConfig.logLevelContexts["ctx1"]);
    EXPECT_EQ(ramses_internal::ELogLevel::Info, config.impl.loggerConfig.logLevelContexts["ctx2"]);
}

TEST_F(ARamsesFrameworkConfig, cliDltAppId)
{
    EXPECT_EQ("RAMS", config.impl.getDLTApplicationID());
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--dlt-app-id"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--dlt-app-id=RROO"});
    EXPECT_EQ("RROO", config.impl.getDLTApplicationID());
}

TEST_F(ARamsesFrameworkConfig, cliDltAppDescription)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--dlt-app-description"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--dlt-app-description=foo"});
    EXPECT_EQ("foo", config.impl.getDLTApplicationDescription());
}

TEST_F(ARamsesFrameworkConfig, cliParticipantName)
{
    EXPECT_EQ("", config.impl.getParticipantName());
    cli.parse("foo", true);
    EXPECT_EQ("foo", config.impl.getParticipantName());
    cli.parse("path/to/bar", true);
    EXPECT_EQ("bar", config.impl.getParticipantName());
    cli.parse(R"(c:\path\to\foobar.exe)", true);
    EXPECT_EQ("foobar.exe", config.impl.getParticipantName());
}

TEST_F(ADisplayConfig, cliWindowRectangle)
{
    cli.parse(std::vector<std::string>{"--xpos=5", "--ypos=10", "--width=420", "--height=998"});
    int32_t x = 0u;
    int32_t y = 0u;
    uint32_t w = 0u;
    uint32_t h = 0u;
    config.impl.getWindowRectangle(x, y, w, h);
    EXPECT_EQ(5, x);
    EXPECT_EQ(10, y);
    EXPECT_EQ(420u, w);
    EXPECT_EQ(998u, h);
}

TEST_F(ADisplayConfig, cliFullscreen)
{
    cli.parse(std::vector<std::string>{"--fullscreen"});
    EXPECT_TRUE(config.isWindowFullscreen());
    config.setWindowFullscreen(false);
    cli.parse(std::vector<std::string>{"-f"});
    EXPECT_TRUE(config.isWindowFullscreen());
}

TEST_F(ADisplayConfig, cliBorderless)
{
    cli.parse(std::vector<std::string>{"--borderless"});
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().getBorderlessState());
}

TEST_F(ADisplayConfig, cliResizable)
{
    cli.parse(std::vector<std::string>{"--resizable"});
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().isResizable());
}

TEST_F(ADisplayConfig, cliDeleteEffects)
{
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().getKeepEffectsUploaded());
    cli.parse(std::vector<std::string>{"--delete-effects"});
    EXPECT_FALSE(config.impl.getInternalDisplayConfig().getKeepEffectsUploaded());
}

TEST_F(ADisplayConfig, cliMsaa)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--msaa"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--msaa=7"}), CLI::ParseError);
    EXPECT_EQ(1u, config.impl.getInternalDisplayConfig().getAntialiasingSampleCount());
    cli.parse(std::vector<std::string>{"--msaa=2"});
    EXPECT_EQ(2u, config.impl.getInternalDisplayConfig().getAntialiasingSampleCount());
    cli.parse(std::vector<std::string>{"--msaa=4"});
    EXPECT_EQ(4u, config.impl.getInternalDisplayConfig().getAntialiasingSampleCount());
    cli.parse(std::vector<std::string>{"--msaa=8"});
    EXPECT_EQ(8u, config.impl.getInternalDisplayConfig().getAntialiasingSampleCount());
}

TEST_F(ADisplayConfig, cliIviVisible)
{
    EXPECT_FALSE(config.impl.getInternalDisplayConfig().getStartVisibleIvi());
    cli.parse(std::vector<std::string>{"--ivi-visible"});
    EXPECT_TRUE(config.impl.getInternalDisplayConfig().getStartVisibleIvi());
}

TEST_F(ADisplayConfig, cliIviLayer)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ivi-layer"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--ivi-layer=42"});
    EXPECT_EQ(42u, config.impl.getInternalDisplayConfig().getWaylandIviLayerID().getValue());
}

TEST_F(ADisplayConfig, cliIviSurface)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ivi-surface"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--ivi-surface=78"});
    EXPECT_EQ(78u, config.impl.getInternalDisplayConfig().getWaylandIviSurfaceID().getValue());
}

TEST_F(ADisplayConfig, cliClear)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--clear"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--clear=1,0,0.4,0.7"});
    EXPECT_FLOAT_EQ(1.f, config.impl.getInternalDisplayConfig().getClearColor()[0]);
    EXPECT_FLOAT_EQ(0.f, config.impl.getInternalDisplayConfig().getClearColor()[1]);
    EXPECT_FLOAT_EQ(0.4f, config.impl.getInternalDisplayConfig().getClearColor()[2]);
    EXPECT_FLOAT_EQ(0.7f, config.impl.getInternalDisplayConfig().getClearColor()[3]);
}

TEST_F(ADisplayConfig, cliThrowsErrorsForExtras)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--foo"}), CLI::ExtrasError);
}

TEST_F(ADisplayConfig, cliEcDisplay)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ec-display"}), CLI::ParseError);
    cli.parse(std::vector<std::string>{"--ec-display=wse"});
    EXPECT_EQ("wse", config.getWaylandEmbeddedCompositingSocketName());
}

TEST_F(ADisplayConfig, cliEcGroup)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ec-socket-group"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ec-socket-group=grp"}), CLI::RequiresError);
    cli.parse(std::vector<std::string>{"--ec-socket-group=grp", "--ec-display=wse"});
    EXPECT_EQ("grp", config.impl.getWaylandSocketEmbeddedGroup());
}

TEST_F(ADisplayConfig, cliEcPermissions)
{
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ec-socket-permissions"}), CLI::ParseError);
    EXPECT_THROW(cli.parse(std::vector<std::string>{"--ec-socket-permissions=0744"}), CLI::RequiresError);
    cli.parse(std::vector<std::string>{"--ec-socket-permissions=0744", "--ec-display=wse"});
    EXPECT_EQ(0744u, config.impl.getWaylandSocketEmbeddedPermissions());
}

TEST_F(ARendererConfig, cliIviControl)
{
    EXPECT_FALSE(config.impl.getInternalRendererConfig().getSystemCompositorControlEnabled());
    cli.parse(std::vector<std::string>{"--ivi-control"});
    EXPECT_TRUE(config.impl.getInternalRendererConfig().getSystemCompositorControlEnabled());
}
