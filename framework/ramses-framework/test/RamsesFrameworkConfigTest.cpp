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
    EXPECT_EQ(ERamsesShellType_Default, frameworkConfig.m_impl.get().m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanBeCopyAndMoveConstructed)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Console);

    RamsesFrameworkConfig configCopy{ frameworkConfig };
    EXPECT_EQ(ERamsesShellType_Console, configCopy.m_impl.get().m_shellType);

    RamsesFrameworkConfig configMove{ std::move(frameworkConfig) };
    EXPECT_EQ(ERamsesShellType_Console, configMove.m_impl.get().m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanBeCopyAndMoveAssigned)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Console);

    RamsesFrameworkConfig configCopy;
    configCopy = frameworkConfig;
    EXPECT_EQ(ERamsesShellType_Console, configCopy.m_impl.get().m_shellType);

    RamsesFrameworkConfig configMove;
    configMove = std::move(frameworkConfig);
    EXPECT_EQ(ERamsesShellType_Console, configMove.m_impl.get().m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanBeSelfAssigned)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Console);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    frameworkConfig = frameworkConfig;
    EXPECT_EQ(ERamsesShellType_Console, frameworkConfig.m_impl.get().m_shellType);
    frameworkConfig = std::move(frameworkConfig);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_EQ(ERamsesShellType_Console, frameworkConfig.m_impl.get().m_shellType);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
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
    EXPECT_EQ(ERamsesShellType_Console, frameworkConfig.m_impl.get().m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetShellTypeNone)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_None);
    EXPECT_EQ(ERamsesShellType_None, frameworkConfig.m_impl.get().m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetShellTypeDefault)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Default);
    EXPECT_EQ(ERamsesShellType_Default, frameworkConfig.m_impl.get().m_shellType);
}

TEST_F(ARamsesFrameworkConfig, TestSetandGetApplicationInformation)
{
    const char* application_id = "myap";
    const char* application_description = "mydescription";

    frameworkConfig.setDLTApplicationID(application_id);
    frameworkConfig.setDLTApplicationDescription(application_description);

    EXPECT_EQ(application_id, frameworkConfig.getDLTApplicationID());
    EXPECT_EQ(application_description, frameworkConfig.getDLTApplicationDescription());
}

TEST_F(ARamsesFrameworkConfig, CanSetLogLevel)
{
    EXPECT_FALSE(frameworkConfig.m_impl.get().loggerConfig.logLevel.has_value());
    frameworkConfig.setLogLevel(ramses::ELogLevel::Debug);
    EXPECT_EQ(ramses_internal::ELogLevel::Debug, frameworkConfig.m_impl.get().loggerConfig.logLevel);
    frameworkConfig.setLogLevel(ramses::ELogLevel::Warn);
    EXPECT_EQ(ramses_internal::ELogLevel::Warn, frameworkConfig.m_impl.get().loggerConfig.logLevel);
}

TEST_F(ARamsesFrameworkConfig, CanSetLogLevelContext)
{
    EXPECT_TRUE(frameworkConfig.m_impl.get().loggerConfig.logLevelContexts.empty());
    frameworkConfig.setLogLevel("RRND", ramses::ELogLevel::Debug);
    EXPECT_EQ(ramses_internal::ELogLevel::Debug, frameworkConfig.m_impl.get().loggerConfig.logLevelContexts["RRND"]);
    frameworkConfig.setLogLevel("RRND", ramses::ELogLevel::Error);
    EXPECT_EQ(ramses_internal::ELogLevel::Error, frameworkConfig.m_impl.get().loggerConfig.logLevelContexts["RRND"]);
}

TEST_F(ARamsesFrameworkConfig, CanSetLogLevelConsole)
{
    EXPECT_FALSE(frameworkConfig.m_impl.get().loggerConfig.logLevelConsole.has_value());
    frameworkConfig.setLogLevelConsole(ramses::ELogLevel::Debug);
    EXPECT_EQ(ramses_internal::ELogLevel::Debug, frameworkConfig.m_impl.get().loggerConfig.logLevelConsole);
}

TEST_F(ARamsesFrameworkConfig, CanSetParticipantGuid)
{
    EXPECT_EQ(Guid(), frameworkConfig.m_impl.get().getUserProvidedGuid());
    EXPECT_EQ(StatusOK, frameworkConfig.setParticipantGuid(4400));
    EXPECT_EQ(Guid(4400), frameworkConfig.m_impl.get().getUserProvidedGuid());
    EXPECT_NE(StatusOK, frameworkConfig.setParticipantGuid(1));
    EXPECT_EQ(Guid(1), frameworkConfig.m_impl.get().getUserProvidedGuid());
    EXPECT_NE(StatusOK, frameworkConfig.setParticipantGuid(0));
    EXPECT_EQ(Guid(0), frameworkConfig.m_impl.get().getUserProvidedGuid());
}

TEST_F(ARamsesFrameworkConfig, CanSetParticipantName)
{
    EXPECT_EQ("", frameworkConfig.m_impl.get().getParticipantName());
    EXPECT_EQ(StatusOK, frameworkConfig.setParticipantName("foo"));
    EXPECT_EQ("foo", frameworkConfig.m_impl.get().getParticipantName());
    EXPECT_EQ(StatusOK, frameworkConfig.setParticipantName("foo/bar"));
    EXPECT_EQ("foo/bar", frameworkConfig.m_impl.get().getParticipantName());
}

TEST_F(ARamsesFrameworkConfig, CanSetPeriodicLogInterval)
{
    EXPECT_EQ(2u, frameworkConfig.m_impl.get().periodicLogTimeout);
    EXPECT_TRUE(frameworkConfig.m_impl.get().m_periodicLogsEnabled);
    frameworkConfig.setPeriodicLogInterval(std::chrono::seconds(0));
    EXPECT_EQ(0u, frameworkConfig.m_impl.get().periodicLogTimeout);
    EXPECT_FALSE(frameworkConfig.m_impl.get().m_periodicLogsEnabled);
    frameworkConfig.setPeriodicLogInterval(std::chrono::seconds(3));
    EXPECT_EQ(3u, frameworkConfig.m_impl.get().periodicLogTimeout);
    EXPECT_TRUE(frameworkConfig.m_impl.get().m_periodicLogsEnabled);
}

TEST_F(ARamsesFrameworkConfig, CanSetInterfaceSelectionSocket)
{
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getIPAddress(), "127.0.0.1");
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getPort(), 0);
    frameworkConfig.setInterfaceSelectionIPForTCPCommunication("192.168.1.1");
    frameworkConfig.setInterfaceSelectionPortForTCPCommunication(5543);
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getIPAddress(), "192.168.1.1");
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getPort(), 5543);
}

TEST_F(ARamsesFrameworkConfig, CanSetDaemonSocket)
{
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getDaemonIPAddress(), "127.0.0.1");
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getDaemonPort(), 5999);
    frameworkConfig.setDaemonIPForTCPCommunication("192.168.1.1");
    frameworkConfig.setDaemonPortForTCPCommunication(5543);
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getDaemonIPAddress(), "192.168.1.1");
    EXPECT_EQ(frameworkConfig.m_impl.get().m_tcpConfig.getDaemonPort(), 5543);
}

TEST_F(ARamsesFrameworkConfig, CanSetConnectionSystem)
{
    EXPECT_EQ(EConnectionProtocol::TCP, frameworkConfig.m_impl.get().getUsedProtocol());
    EXPECT_EQ(StatusOK, frameworkConfig.setConnectionSystem(EConnectionSystem::Off));
    EXPECT_EQ(EConnectionProtocol::Off, frameworkConfig.m_impl.get().getUsedProtocol());
    EXPECT_EQ(StatusOK, frameworkConfig.setConnectionSystem(EConnectionSystem::TCP));
    EXPECT_EQ(EConnectionProtocol::TCP, frameworkConfig.m_impl.get().getUsedProtocol());
}

TEST_F(ARamsesFrameworkConfig, CanSetTCPKeepAlive)
{
    EXPECT_EQ(std::chrono::milliseconds(300), frameworkConfig.m_impl.get().m_tcpConfig.getAliveInterval());
    EXPECT_EQ(std::chrono::milliseconds(300*6), frameworkConfig.m_impl.get().m_tcpConfig.getAliveTimeout());
    frameworkConfig.setConnectionKeepaliveSettings(std::chrono::milliseconds(250), std::chrono::milliseconds(9000));
    EXPECT_EQ(std::chrono::milliseconds(250), frameworkConfig.m_impl.get().m_tcpConfig.getAliveInterval());
    EXPECT_EQ(std::chrono::milliseconds(9000), frameworkConfig.m_impl.get().m_tcpConfig.getAliveTimeout());
}

