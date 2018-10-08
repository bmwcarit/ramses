//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SceneConfig.h"

// internal
#include "SceneConfigImpl.h"
#include "APILoggingMacros.h"

namespace ramses
{
    SceneConfig::SceneConfig()
        : StatusObject(*new SceneConfigImpl())
        , impl(static_cast<SceneConfigImpl&>(StatusObject::impl))
    {
    }

    SceneConfig::SceneConfig(const SceneConfig& other)
        : StatusObject(*new SceneConfigImpl(other.impl))
        , impl(static_cast<SceneConfigImpl&>(StatusObject::impl))
    {
    }

    SceneConfig::~SceneConfig()
    {
    }

    status_t SceneConfig::setPublicationMode(EScenePublicationMode publicationMode)
    {
        const status_t status = impl.setPublicationMode(publicationMode);
        LOG_HL_CLIENT_API1(status, publicationMode);
        return status;
    }

    status_t SceneConfig::setMaximumLatency(uint32_t maxLatencyInMilliseconds)
    {
        const status_t status = impl.setMaximumLatency(maxLatencyInMilliseconds);
        LOG_HL_CLIENT_API1(status, maxLatencyInMilliseconds);
        return status;
    }
}
