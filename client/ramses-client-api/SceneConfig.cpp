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
        : StatusObject{ std::make_unique<SceneConfigImpl>() }
        , m_impl{ static_cast<SceneConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    SceneConfig::~SceneConfig() = default;

    SceneConfig::SceneConfig(const SceneConfig& other)
        : StatusObject{ std::make_unique<SceneConfigImpl>(other.m_impl) }
        , m_impl{ static_cast<SceneConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    SceneConfig::SceneConfig(SceneConfig&& other) noexcept
        : StatusObject{ std::move(other.StatusObject::m_impl) }
        , m_impl{ static_cast<SceneConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    SceneConfig& SceneConfig::operator=(const SceneConfig& other)
    {
        StatusObject::m_impl = std::make_unique<SceneConfigImpl>(other.m_impl);
        m_impl = static_cast<SceneConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    SceneConfig& SceneConfig::operator=(SceneConfig&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<SceneConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    status_t SceneConfig::setPublicationMode(EScenePublicationMode publicationMode)
    {
        const status_t status = m_impl.get().setPublicationMode(publicationMode);
        LOG_HL_CLIENT_API1(status, publicationMode);
        return status;
    }
}
