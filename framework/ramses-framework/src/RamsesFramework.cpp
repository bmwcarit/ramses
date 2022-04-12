//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"

#include "APILoggingHelper.h"

#include <string>

namespace ramses
{
    RamsesFramework::RamsesFramework()
        : RamsesFramework(0, nullptr)
    {
    }

    RamsesFramework::RamsesFramework(int32_t argc, char const* const* argv)
        : StatusObject(RamsesFrameworkImpl::createImpl(argc, argv))
        , impl(static_cast<RamsesFrameworkImpl&>(StatusObject::impl))
    {
        LOG_HL_CLIENT_API1(LOG_API_VOID, ramses_internal::APILoggingHelper::MakeLoggingString(argc, argv));
    }

    RamsesFramework::RamsesFramework(const RamsesFrameworkConfig& config)
        : StatusObject(RamsesFrameworkImpl::createImpl(config))
        , impl(static_cast<RamsesFrameworkImpl&>(StatusObject::impl))
    {
        LOG_HL_CLIENT_API1(LOG_API_VOID, LOG_API_GENERIC_OBJECT_STRING(config));
    }

    ramses::RamsesRenderer* RamsesFramework::createRenderer(const RendererConfig& config)
    {
        auto result = impl.createRenderer(config);
        LOG_HL_CLIENT_API1(LOG_API_GENERIC_PTR_STRING(result), LOG_API_GENERIC_PTR_STRING(&config));
        return result;
    }

    ramses::RamsesClient* RamsesFramework::createClient(const char* applicationName)
    {
        auto result = impl.createClient(applicationName);
        LOG_HL_CLIENT_API1(LOG_API_GENERIC_PTR_STRING(result), applicationName);
        return result;
    }

    status_t RamsesFramework::destroyRenderer(RamsesRenderer& renderer)
    {
        const status_t result = impl.destroyRenderer(renderer);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_OBJECT_STRING(renderer));
        return result;
    }

    status_t RamsesFramework::destroyClient(RamsesClient& client)
    {
        const status_t result = impl.destroyClient(client);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_OBJECT_STRING(client));
        return result;
    }

    RamsesFramework::~RamsesFramework()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    status_t RamsesFramework::connect()
    {
        const status_t result = impl.connect();
        LOG_HL_CLIENT_API_NOARG(result);
        return result;
    }

    bool RamsesFramework::isConnected() const
    {
        return impl.isConnected();
    }

    status_t RamsesFramework::disconnect()
    {
        const status_t result = impl.disconnect();
        LOG_HL_CLIENT_API_NOARG(result);
        return result;
    }

    DcsmProvider* RamsesFramework::createDcsmProvider()
    {
        DcsmProvider* result = impl.createDcsmProvider();
        LOG_HL_CLIENT_API_NOARG(LOG_API_GENERIC_PTR_STRING(result));
        return result;
    }

    status_t RamsesFramework::destroyDcsmProvider(const DcsmProvider& provider)
    {
        const status_t result = impl.destroyDcsmProvider(provider);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_PTR_STRING(&provider));
        return result;
    }

    DcsmConsumer* RamsesFramework::createDcsmConsumer()
    {
        DcsmConsumer* result = impl.createDcsmConsumer();
        LOG_HL_CLIENT_API_NOARG(LOG_API_GENERIC_PTR_STRING(result));
        return result;
    }

    status_t RamsesFramework::destroyDcsmConsumer(const DcsmConsumer& consumer)
    {
        const status_t result = impl.destroyDcsmConsumer(consumer);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_PTR_STRING(&consumer));
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

    status_t RamsesFramework::addRamshCommand(const std::shared_ptr<IRamshCommand>& command)
    {
        const status_t result = impl.addRamshCommand(command);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_PTR_STRING(command.get()));
        return result;
    }

    status_t RamsesFramework::executeRamshCommand(const std::string& input)
    {
        return impl.executeRamshCommand(input);
    }
}
