//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// api
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"
#include "ramses/client/SceneConfig.h"

// private
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/SceneConfigImpl.h"
#include "RamsesClientTypesImpl.h"

namespace ramses
{
    RamsesClient::RamsesClient(std::unique_ptr<internal::RamsesClientImpl> impl)
        : RamsesObject{ std::move(impl) }
        , m_impl{ static_cast<internal::RamsesClientImpl&>(*RamsesObject::m_impl) }
    {
        m_impl.setHLObject(this);
    }

    Scene* RamsesClient::createScene(sceneId_t sceneId, std::string_view name)
    {
        return createScene(SceneConfig(sceneId), name);
    }

    Scene* RamsesClient::createScene(const SceneConfig& sceneConfig, std::string_view name)
    {
        Scene* scene =  m_impl.createScene(sceneConfig.impl(), name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), sceneConfig.impl().getSceneId(), name);
        return scene;
    }

    bool RamsesClient::destroy(Scene& scene)
    {
        const bool status = m_impl.destroy(scene);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(scene));
        return status;
    }

    ramses::Scene* RamsesClient::loadSceneFromFile(std::string_view fileName, const SceneConfig& config)
    {
        auto scene = m_impl.loadSceneFromFile(fileName, config.impl());
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(scene), fileName);
        return scene;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ramses::Scene* RamsesClient::loadSceneFromMemory(std::unique_ptr<std::byte[], void(*)(const std::byte*)> data, size_t size, const SceneConfig& config)
    {
        auto scene = m_impl.loadSceneFromMemory(std::move(data), size, config.impl());
        LOG_HL_CLIENT_API1(LOG_API_RAMSESOBJECT_PTR_STRING(scene), size);
        return scene;
    }

    ramses::Scene* RamsesClient::loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, const SceneConfig& config)
    {
        auto scene = m_impl.loadSceneFromFileDescriptor(fd, offset, length, config.impl());
        LOG_HL_CLIENT_API3(LOG_API_RAMSESOBJECT_PTR_STRING(scene), fd, offset, length);
        return scene;
    }

    bool RamsesClient::loadSceneFromFileAsync(std::string_view fileName, const SceneConfig& config)
    {
        auto status = m_impl.loadSceneFromFileAsync(fileName, config.impl());
        LOG_HL_CLIENT_API1(status, fileName);
        return status;
    }

    bool RamsesClient::GetFeatureLevelFromFile(std::string_view fileName, EFeatureLevel& detectedFeatureLevel)
    {
        const bool ret = internal::RamsesClientImpl::GetFeatureLevelFromFile(fileName, detectedFeatureLevel);
        LOG_HL_CLIENT_STATIC_API1(ret, fileName);
        return ret;
    }

    bool RamsesClient::GetFeatureLevelFromFile(int fd, size_t offset, size_t length, EFeatureLevel& detectedFeatureLevel)
    {
        const bool ret = internal::RamsesClientImpl::GetFeatureLevelFromFile(fd, offset, length, detectedFeatureLevel);
        LOG_HL_CLIENT_STATIC_API3(ret, fd, offset, length);
        return ret;
    }

    const Scene* RamsesClient::findSceneByName(std::string_view name) const
    {
        return m_impl.findSceneByName(name);
    }

    Scene* RamsesClient::findSceneByName(std::string_view name)
    {
        return m_impl.findSceneByName(name);
    }

    const Scene* RamsesClient::getScene(sceneId_t sceneId) const
    {
        return m_impl.getScene(sceneId);
    }

    Scene* RamsesClient::getScene(sceneId_t sceneId)
    {
        return m_impl.getScene(sceneId);
    }

    bool RamsesClient::dispatchEvents(IClientEventHandler& clientEventHandler)
    {
        auto status = m_impl.dispatchEvents(clientEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(clientEventHandler));
        return status;
    }

    const RamsesFramework& RamsesClient::getRamsesFramework() const
    {
        return m_impl.getFramework().getHLRamsesFramework();
    }

    RamsesFramework& RamsesClient::getRamsesFramework()
    {
        return m_impl.getFramework().getHLRamsesFramework();
    }

    internal::RamsesClientImpl& RamsesClient::impl()
    {
        return m_impl;
    }

    const internal::RamsesClientImpl& RamsesClient::impl() const
    {
        return m_impl;
    }
}
