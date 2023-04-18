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
    RamsesClient::RamsesClient(RamsesClientImpl& impl_)
    : RamsesObject(impl_)
    , impl(impl_)
    {
        impl.setHLObject(this);
    }

    RamsesClient::~RamsesClient()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    Scene* RamsesClient::createScene(sceneId_t sceneId, const SceneConfig& sceneConfig /*= SceneConfig()*/, const char* name)
    {
        Scene* scene =  impl.createScene(sceneId, sceneConfig.impl, name);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), sceneId, name);
        return scene;
    }

    status_t RamsesClient::destroy(Scene& scene)
    {
        const status_t status = impl.destroy(scene);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(scene));
        return status;
    }

    ramses::Scene* RamsesClient::loadSceneFromFile(const char* fileName, bool localOnly)
    {
        auto scene = impl.loadSceneFromFile(fileName, localOnly);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), fileName, localOnly);
        return scene;
    }

    ramses::Scene* RamsesClient::loadSceneFromMemory(std::unique_ptr<unsigned char[], void(*)(const unsigned char*)> data, size_t size, bool localOnly)
    {
        auto scene = impl.loadSceneFromMemory(std::move(data), size, localOnly);
        LOG_HL_CLIENT_API2(LOG_API_RAMSESOBJECT_PTR_STRING(scene), size, localOnly);
        return scene;
    }

    ramses::Scene* RamsesClient::loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, bool localOnly)
    {
        auto scene = impl.loadSceneFromFileDescriptor(fd, offset, length, localOnly);
        LOG_HL_CLIENT_API4(LOG_API_RAMSESOBJECT_PTR_STRING(scene), fd, offset, length, localOnly);
        return scene;
    }

    ramses::Scene* RamsesClient::loadSceneFromFileDescriptor(sceneId_t sceneId, int fd, size_t offset, size_t length, bool localOnly)
    {
        auto scene = impl.loadSceneFromFileDescriptor(sceneId, fd, offset, length, localOnly);
        LOG_HL_CLIENT_API5(LOG_API_RAMSESOBJECT_PTR_STRING(scene), sceneId, fd, offset, length, localOnly);
        return scene;
    }

    status_t RamsesClient::loadSceneFromFileAsync(const char* fileName, bool localOnly)
    {
        auto status = impl.loadSceneFromFileAsync(fileName, localOnly);
        LOG_HL_CLIENT_API2(status, fileName, localOnly);
        return status;
    }

    bool RamsesClient::GetFeatureLevelFromFile(const char* fileName, EFeatureLevel& detectedFeatureLevel)
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

    const Scene* RamsesClient::findSceneByName(const char* name) const
    {
        return impl.findSceneByName(name);
    }

    Scene* RamsesClient::findSceneByName(const char* name)
    {
        return impl.findSceneByName(name);
    }

    const Scene* RamsesClient::getScene(sceneId_t sceneId) const
    {
        return impl.getScene(sceneId);
    }

    Scene* RamsesClient::getScene(sceneId_t sceneId)
    {
        return impl.getScene(sceneId);
    }

    status_t RamsesClient::dispatchEvents(IClientEventHandler& clientEventHandler)
    {
        auto status = impl.dispatchEvents(clientEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(clientEventHandler));
        return status;
    }
}
