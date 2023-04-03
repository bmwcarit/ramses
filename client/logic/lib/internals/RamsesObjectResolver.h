//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include <string>

namespace ramses
{
    class SceneObject;
    class Scene;
    class Node;
    class Appearance;
    class Camera;
    class RenderPass;
    class RenderGroup;
}

namespace rlogic::internal
{
    class ErrorReporting;

    class IRamsesObjectResolver
    {
    public:
        virtual ~IRamsesObjectResolver() = default;

        [[nodiscard]] virtual ramses::Node* findRamsesNodeInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const = 0;
        [[nodiscard]] virtual ramses::Appearance* findRamsesAppearanceInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const = 0;
        [[nodiscard]] virtual ramses::Camera* findRamsesCameraInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const = 0;
        [[nodiscard]] virtual ramses::RenderPass* findRamsesRenderPassInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const = 0;
        [[nodiscard]] virtual ramses::RenderGroup* findRamsesRenderGroupInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const = 0;
        [[nodiscard]] virtual ramses::SceneObject* findRamsesSceneObjectInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const = 0;
    };

    class RamsesObjectResolver final : public IRamsesObjectResolver
    {
    public:
        explicit RamsesObjectResolver(ErrorReporting& errorReporting, ramses::Scene& scene);

        [[nodiscard]] ramses::Node* findRamsesNodeInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const override;
        [[nodiscard]] ramses::Appearance* findRamsesAppearanceInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const override;
        [[nodiscard]] ramses::Camera* findRamsesCameraInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const override;
        [[nodiscard]] ramses::RenderPass* findRamsesRenderPassInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const override;
        [[nodiscard]] ramses::RenderGroup* findRamsesRenderGroupInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const override;
        [[nodiscard]] ramses::SceneObject* findRamsesSceneObjectInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const override;

    private:
        template <typename T>
        [[nodiscard]] T* findRamsesObjectInScene(std::string_view logicNodeName, ramses::sceneObjectId_t objectId) const;

        ErrorReporting& m_errors;
        ramses::Scene& m_scene;
    };
}
