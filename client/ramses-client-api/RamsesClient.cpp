//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// api
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/SceneConfig.h"

// private
#include "RamsesClientImpl.h"
#include "RamsesClientTypesImpl.h"
#include "SceneConfigImpl.h"

namespace ramses
{
    RamsesClient::RamsesClient(std::unique_ptr<RamsesClientImpl> impl)
        : RamsesObject{ std::move(impl) }
        , m_impl{ static_cast<RamsesClientImpl&>(RamsesObject::m_impl) }
    {
        m_impl.setHLObject(this);
    }

    Scene* RamsesClient::createScene(sceneId_t sceneId, const SceneConfig& sceneConfig /*= SceneConfig()*/, std::string_view name)
    {
        Scene* scene =  m_impl.createScene(sceneId, sceneConfig.m_impl, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), sceneId, name);
        return scene;
    }

    status_t RamsesClient::destroy(Scene& scene)
    {
        const status_t status = m_impl.destroy(scene);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(scene));
        return status;
    }

    ramses::Scene* RamsesClient::loadSceneFromFile(std::string_view fileName, bool localOnly)
    {
        auto scene = m_impl.loadSceneFromFile(fileName, localOnly);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), fileName, localOnly);
        return scene;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ramses::Scene* RamsesClient::loadSceneFromMemory(std::unique_ptr<unsigned char[], void(*)(const unsigned char*)> data, size_t size, bool localOnly)
    {
        auto scene = m_impl.loadSceneFromMemory(std::move(data), size, localOnly);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), size, localOnly);
        return scene;
    }

    ramses::Scene* RamsesClient::loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, bool localOnly)
    {
        auto scene = m_impl.loadSceneFromFileDescriptor(fd, offset, length, localOnly);
        LOG_HL_CLIENT_API4(LOG_API_RAMSESOBJECT_PTR_STRING(scene), fd, offset, length, localOnly);
        return scene;
    }

    ramses::Scene* RamsesClient::loadSceneFromFileDescriptor(sceneId_t sceneId, int fd, size_t offset, size_t length, bool localOnly)
    {
        auto scene = m_impl.loadSceneFromFileDescriptor(sceneId, fd, offset, length, localOnly);
        LOG_HL_CLIENT_API5(LOG_API_RAMSESOBJECT_PTR_STRING(scene), sceneId, fd, offset, length, localOnly);
        return scene;
    }

    status_t RamsesClient::loadSceneFromFileAsync(std::string_view fileName, bool localOnly)
    {
        auto status = m_impl.loadSceneFromFileAsync(fileName, localOnly);
        LOG_HL_CLIENT_API2(status, fileName, localOnly);
        return status;
    }

    bool RamsesClient::GetFeatureLevelFromFile(std::string_view fileName, EFeatureLevel& detectedFeatureLevel)
    {
        const bool ret = RamsesClientImpl::GetFeatureLevelFromFile(fileName, detectedFeatureLevel);
        LOG_HL_CLIENT_STATIC_API1(ret, fileName);
        return ret;
    }

    bool RamsesClient::GetFeatureLevelFromFile(int fd, size_t offset, size_t length, EFeatureLevel& detectedFeatureLevel)
    {
        const bool ret = RamsesClientImpl::GetFeatureLevelFromFile(fd, offset, length, detectedFeatureLevel);
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

    status_t RamsesClient::dispatchEvents(IClientEventHandler& clientEventHandler)
    {
        auto status = m_impl.dispatchEvents(clientEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(clientEventHandler));
        return status;
    }
}
