//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"
#include "PlatformAbstraction/PlatformTime.h"

#include "APILoggingHelper.h"

#include <string>

namespace ramses
{
    RamsesFramework::RamsesFramework(const RamsesFrameworkConfig& config)
        : StatusObject{ RamsesFrameworkImpl::CreateImpl(config) }
        , m_impl{ static_cast<RamsesFrameworkImpl&>(*StatusObject::m_impl) }
    {
        LOG_HL_CLIENT_API1(LOG_API_VOID, LOG_API_GENERIC_OBJECT_STRING(config));
    }

    ramses::RamsesRenderer* RamsesFramework::createRenderer(const RendererConfig& config)
    {
        auto result = m_impl.createRenderer(config);
        LOG_HL_CLIENT_API1(LOG_API_GENERIC_PTR_STRING(result), LOG_API_GENERIC_PTR_STRING(&config));
        return result;
    }

    ramses::RamsesClient* RamsesFramework::createClient(const char* applicationName)
    {
        auto result = m_impl.createClient(applicationName);
        LOG_HL_CLIENT_API1(LOG_API_GENERIC_PTR_STRING(result), applicationName);
        return result;
    }

    status_t RamsesFramework::destroyRenderer(RamsesRenderer& renderer)
    {
        const status_t result = m_impl.destroyRenderer(renderer);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_OBJECT_STRING(renderer));
        return result;
    }

    status_t RamsesFramework::destroyClient(RamsesClient& client)
    {
        const status_t result = m_impl.destroyClient(client);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_OBJECT_STRING(client));
        return result;
    }

    RamsesFramework::~RamsesFramework()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    status_t RamsesFramework::connect()
    {
        const status_t result = m_impl.connect();
        LOG_HL_CLIENT_API_NOARG(result);
        return result;
    }

    bool RamsesFramework::isConnected() const
    {
        return m_impl.isConnected();
    }

    status_t RamsesFramework::disconnect()
    {
        const status_t result = m_impl.disconnect();
        LOG_HL_CLIENT_API_NOARG(result);
        return result;
    }

    void RamsesFramework::SetConsoleLogLevel(ELogLevel logLevel)
    {
        RamsesFrameworkImpl::SetConsoleLogLevel(logLevel);
    }

    void RamsesFramework::SetLogHandler(const LogHandlerFunc& logHandlerFunc)
    {
        RamsesFrameworkImpl::SetLogHandler(logHandlerFunc);
    }

    uint64_t RamsesFramework::GetSynchronizedClockMilliseconds()
    {
        return ramses_internal::PlatformTime::GetMillisecondsSynchronized();
    }

    status_t RamsesFramework::addRamshCommand(const std::shared_ptr<IRamshCommand>& command)
    {
        const status_t result = m_impl.addRamshCommand(command);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_PTR_STRING(command.get()));
        return result;
    }

    status_t RamsesFramework::executeRamshCommand(const std::string& input)
    {
        return m_impl.executeRamshCommand(input);
    }
}
