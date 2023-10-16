//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/logic/RamsesObjectResolver.h"

#include "impl/ErrorReporting.h"

#include "ramses/client/Node.h"
#include "ramses/client/Camera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/SceneObject.h"
#include "ramses/client/ramses-utils.h"

#include "impl/SceneImpl.h"

#include "fmt/format.h"

namespace ramses::internal
{
    RamsesObjectResolver::RamsesObjectResolver(ErrorReporting& errorReporting, SceneImpl& scene)
        : m_errors(errorReporting)
        , m_scene(scene)
    {
    }

    ramses::Node* RamsesObjectResolver::findRamsesNodeInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const
    {
        return findRamsesObjectInScene<ramses::Node>(logicNodeName, objectId);
    }

    ramses::Appearance* RamsesObjectResolver::findRamsesAppearanceInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const
    {
        return findRamsesObjectInScene<ramses::Appearance>(logicNodeName, objectId);
    }

    ramses::Camera* RamsesObjectResolver::findRamsesCameraInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const
    {
        return findRamsesObjectInScene<ramses::Camera>(logicNodeName, objectId);
    }

    ramses::RenderPass* RamsesObjectResolver::findRamsesRenderPassInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const
    {
        return findRamsesObjectInScene<ramses::RenderPass>(logicNodeName, objectId);
    }

    ramses::RenderGroup* RamsesObjectResolver::findRamsesRenderGroupInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const
    {
        return findRamsesObjectInScene<ramses::RenderGroup>(logicNodeName, objectId);
    }

    ramses::SceneObject* RamsesObjectResolver::findRamsesSceneObjectInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const
    {
        return findRamsesObjectInScene<ramses::SceneObject>(logicNodeName, objectId);
    }

    template <typename T>
    T* RamsesObjectResolver::findRamsesObjectInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const
    {
        ramses::SceneObject* sceneObject = m_scene.findObjectById(objectId);

        if (sceneObject == nullptr)
        {
            m_errors.set(
                fmt::format("Fatal error during loading from file! Serialized Ramses Logic object '{}' points to a Ramses object (id: {}) which couldn't be found in the provided scene!",
                    logicNodeName, objectId.getValue()), nullptr);
            return nullptr;
        }

        T* concreteObject = sceneObject->as<T>();
        if (concreteObject == nullptr)
        {
            m_errors.set(fmt::format("Fatal error during loading from file! Ramses binding '{}' points to a Ramses scene object (id: {}) which is not of the same type!",
                logicNodeName, objectId.getValue()), nullptr);
            return nullptr;
        }

        return concreteObject;
    }

    template ramses::Node* RamsesObjectResolver::findRamsesObjectInScene<ramses::Node>(std::string_view, ramses::sceneObjectId_t) const;
    template ramses::Appearance* RamsesObjectResolver::findRamsesObjectInScene<ramses::Appearance>(std::string_view, ramses::sceneObjectId_t) const;
    template ramses::Camera* RamsesObjectResolver::findRamsesObjectInScene<ramses::Camera>(std::string_view, ramses::sceneObjectId_t) const;
    template ramses::RenderPass* RamsesObjectResolver::findRamsesObjectInScene<ramses::RenderPass>(std::string_view, ramses::sceneObjectId_t) const;
    template ramses::RenderGroup* RamsesObjectResolver::findRamsesObjectInScene<ramses::RenderGroup>(std::string_view, ramses::sceneObjectId_t) const;
    template ramses::SceneObject* RamsesObjectResolver::findRamsesObjectInScene<ramses::SceneObject>(std::string_view, ramses::sceneObjectId_t) const;
}
