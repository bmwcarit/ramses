//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/framework/RamsesFramework.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/APILoggingMacros.h"
#include "internal/PlatformAbstraction/PlatformTime.h"

#include "APILoggingHelper.h"

#include <string>

namespace ramses
{
    RamsesFramework::RamsesFramework(const RamsesFrameworkConfig& config)
        : m_impl{ internal::RamsesFrameworkImpl::CreateImpl(config) }
    {
        m_impl->setHLFramework(*this);
        LOG_HL_CLIENT_API1(LOG_API_VOID, LOG_API_GENERIC_OBJECT_STRING(config));
    }

    RamsesRenderer* RamsesFramework::createRenderer(const RendererConfig& config)
    {
        auto result = m_impl->createRenderer(config);
        LOG_HL_CLIENT_API1(LOG_API_GENERIC_PTR_STRING(result), LOG_API_GENERIC_PTR_STRING(&config));
        return result;
    }

    RamsesClient* RamsesFramework::createClient(std::string_view applicationName)
    {
        auto result = m_impl->createClient(applicationName);
        LOG_HL_CLIENT_API1(LOG_API_GENERIC_PTR_STRING(result), applicationName);
        return result;
    }

    bool RamsesFramework::destroyRenderer(RamsesRenderer& renderer)
    {
        const bool result = m_impl->destroyRenderer(renderer);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_OBJECT_STRING(renderer));
        return result;
    }

    bool RamsesFramework::destroyClient(RamsesClient& client)
    {
        const bool result = m_impl->destroyClient(client);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_OBJECT_STRING(client));
        return result;
    }

    std::optional<Issue> RamsesFramework::getLastError()
    {
        return m_impl->getLastError();
    }

    RamsesFramework::~RamsesFramework()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    bool RamsesFramework::connect()
    {
        const bool result = m_impl->connect();
        LOG_HL_CLIENT_API_NOARG(result);
        return result;
    }

    bool RamsesFramework::isConnected() const
    {
        return m_impl->isConnected();
    }

    bool RamsesFramework::disconnect()
    {
        const bool result = m_impl->disconnect();
        LOG_HL_CLIENT_API_NOARG(result);
        return result;
    }

    EFeatureLevel RamsesFramework::getFeatureLevel() const
    {
        return m_impl->getFeatureLevel();
    }

    void RamsesFramework::SetLogHandler(const LogHandlerFunc& logHandlerFunc)
    {
        internal::RamsesFrameworkImpl::SetLogHandler(logHandlerFunc);
    }

    uint64_t RamsesFramework::GetSynchronizedClockMilliseconds()
    {
        return ramses::internal::PlatformTime::GetMillisecondsSynchronized();
    }

    bool RamsesFramework::addRamshCommand(const std::shared_ptr<IRamshCommand>& command)
    {
        const bool result = m_impl->addRamshCommand(command);
        LOG_HL_CLIENT_API1(result, LOG_API_GENERIC_PTR_STRING(command.get()));
        return result;
    }

    bool RamsesFramework::executeRamshCommand(const std::string& input)
    {
        return m_impl->executeRamshCommand(input);
    }

    internal::RamsesFrameworkImpl& RamsesFramework::impl()
    {
        return *m_impl;
    }

    const internal::RamsesFrameworkImpl& RamsesFramework::impl() const
    {
        return *m_impl;
    }
}
