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

namespace ramses
{
    RamsesFramework::RamsesFramework(int32_t argc, const char * argv[])
        : StatusObject(RamsesFrameworkImpl::createImpl(argc, argv))
        , impl(static_cast<RamsesFrameworkImpl&>(StatusObject::impl))
    {
        LOG_HL_CLIENT_API1(LOG_API_VOID, ramses_internal::APILoggingHelper::MakeLoggingString(argc, argv));
    }

    RamsesFramework::RamsesFramework(int32_t argc, char * argv[])
        : StatusObject(RamsesFrameworkImpl::createImpl(argc, const_cast<const char**>(argv)))
        , impl(static_cast<RamsesFrameworkImpl&>(StatusObject::impl))
    {
        LOG_HL_CLIENT_API1(LOG_API_VOID, ramses_internal::APILoggingHelper::MakeLoggingString(argc, const_cast<const char**>(argv)));
    }

    RamsesFramework::RamsesFramework(const RamsesFrameworkConfig& config)
        : StatusObject(RamsesFrameworkImpl::createImpl(config))
        , impl(static_cast<RamsesFrameworkImpl&>(StatusObject::impl))
    {
        LOG_HL_CLIENT_API1(LOG_API_VOID, LOG_API_GENERIC_OBJECT_STRING(config));
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
}
