//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/SceneConfig.h"

// internal
#include "impl/SceneConfigImpl.h"
#include "impl/APILoggingMacros.h"

namespace ramses
{
    SceneConfig::SceneConfig()
        : m_impl{ std::make_unique<internal::SceneConfigImpl>() }
    {
    }

    SceneConfig::SceneConfig(sceneId_t sceneId, EScenePublicationMode publicationMode, ERenderBackendCompatibility renderBackendCompatibility)
        : m_impl{ std::make_unique<internal::SceneConfigImpl>() }
    {
        m_impl->setSceneId(sceneId);
        m_impl->setPublicationMode(publicationMode);
        m_impl->setRenderBackendCompatibility(renderBackendCompatibility);
    }

    SceneConfig::~SceneConfig() = default;

    SceneConfig::SceneConfig(const SceneConfig& other)
        : m_impl{ std::make_unique<internal::SceneConfigImpl>(*other.m_impl) }
    {
    }

    SceneConfig::SceneConfig(SceneConfig&& other) noexcept = default;

    SceneConfig& SceneConfig::operator=(const SceneConfig& other)
    {
        m_impl = std::make_unique<internal::SceneConfigImpl>(*other.m_impl);
        return *this;
    }

    SceneConfig& SceneConfig::operator=(SceneConfig&& other) noexcept = default;

    internal::SceneConfigImpl& SceneConfig::impl()
    {
        return *m_impl;
    }

    const internal::SceneConfigImpl& SceneConfig::impl() const
    {
        return *m_impl;
    }

    void SceneConfig::setPublicationMode(EScenePublicationMode publicationMode)
    {
        m_impl->setPublicationMode(publicationMode);
        LOG_HL_CLIENT_API1(true, publicationMode);
    }

    void SceneConfig::setSceneId(sceneId_t sceneId)
    {
        m_impl->setSceneId(sceneId);
        LOG_HL_CLIENT_API1(true, sceneId.getValue());
    }

    void SceneConfig::setMemoryVerificationEnabled(bool enabled)
    {
        m_impl->setMemoryVerificationEnabled(enabled);
        LOG_HL_CLIENT_API1(true, enabled);
    }

    void SceneConfig::setRenderBackendCompatibility(ERenderBackendCompatibility renderBackendCompatibility)
    {
        m_impl->setRenderBackendCompatibility(renderBackendCompatibility);
        LOG_HL_CLIENT_API1(true, renderBackendCompatibility);
    }
}
