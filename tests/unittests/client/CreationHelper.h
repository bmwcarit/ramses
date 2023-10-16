//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"

#include <cstdint>
#include <cassert>
#include <string_view>

namespace ramses
{
    class RamsesObject;
    class SceneObject;
    class RamsesClient;
    class RamsesFramework;
    class TextureCubeInput;
    class Scene;
    class Node;
    class MeshNode;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Effect;
    class Appearance;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class ArrayResource;
    class RenderGroup;
    class RenderPass;
    class BlitPass;
    class TextureSampler;
    class TextureSamplerMS;
    class TextureSamplerExternal;
    class RenderBuffer;
    class RenderTarget;
    class RenderGroup;
    class ArrayBuffer;
    class Texture2DBuffer;
    class Geometry;
    class DataObject;
    class PickableObject;
    class SceneReference;
    class LogicEngine;
}

namespace ramses::internal
{
    class CreationHelper
    {
    public:
        CreationHelper(ramses::Scene* scene, RamsesClient* ramsesClient);
        ~CreationHelper();

        void setScene(ramses::Scene* scene);

        template <typename ObjectType>
        ObjectType* createObjectOfType([[maybe_unused]] std::string_view name)
        {
            assert(false);
            return NULL;
        }

        void destroyAdditionalAllocatedSceneObjects();
        [[nodiscard]] size_t getAdditionalAllocatedNodeCount() const;

    private:
        ramses::Scene* m_scene;
        RamsesClient* m_ramsesClient;

        using ClientAndFramework = std::pair<RamsesClient *, RamsesFramework *>;
        using RamsesClientAndFrameworkComponentVector = std::vector<ClientAndFramework>;
        RamsesClientAndFrameworkComponentVector m_allocatedClientAndFrameworkComponents;
        std::vector<SceneObject*> m_additionalAllocatedSceneObjects;
        sceneId_t m_lastReferencedSceneId{ 123u };
    };

    template <> ramses::RamsesClient*       CreationHelper::createObjectOfType<ramses::RamsesClient>(std::string_view name);
    template <> ramses::Scene*              CreationHelper::createObjectOfType<ramses::Scene>(std::string_view name);
    template <> ramses::LogicEngine*        CreationHelper::createObjectOfType<ramses::LogicEngine>(std::string_view name);
    template <> ramses::Node*               CreationHelper::createObjectOfType<ramses::Node>(std::string_view name);
    template <> ramses::MeshNode*           CreationHelper::createObjectOfType<ramses::MeshNode>(std::string_view name);
    template <> ramses::PerspectiveCamera*  CreationHelper::createObjectOfType<ramses::PerspectiveCamera>(std::string_view name);
    template <> ramses::OrthographicCamera* CreationHelper::createObjectOfType<ramses::OrthographicCamera>(std::string_view name);
    template <> ramses::Effect*             CreationHelper::createObjectOfType<ramses::Effect>(std::string_view name);
    template <> ramses::Appearance*         CreationHelper::createObjectOfType<ramses::Appearance>(std::string_view name);
    template <> ramses::Texture2D*          CreationHelper::createObjectOfType<ramses::Texture2D>(std::string_view name);
    template <> ramses::Texture3D*          CreationHelper::createObjectOfType<ramses::Texture3D>(std::string_view name);
    template <> ramses::TextureCube*        CreationHelper::createObjectOfType<ramses::TextureCube>(std::string_view name);
    template <> ramses::ArrayResource*      CreationHelper::createObjectOfType<ramses::ArrayResource>(std::string_view name);
    template <> ramses::RenderGroup*        CreationHelper::createObjectOfType<ramses::RenderGroup>(std::string_view name);
    template <> ramses::RenderPass*         CreationHelper::createObjectOfType<ramses::RenderPass>(std::string_view name);
    template <> ramses::BlitPass*           CreationHelper::createObjectOfType<ramses::BlitPass>(std::string_view name);
    template <> ramses::TextureSampler*     CreationHelper::createObjectOfType<ramses::TextureSampler>(std::string_view name);
    template <> ramses::TextureSamplerMS*   CreationHelper::createObjectOfType<ramses::TextureSamplerMS>(std::string_view name);
    template <> ramses::TextureSamplerExternal* CreationHelper::createObjectOfType<ramses::TextureSamplerExternal>(std::string_view name);
    template <> ramses::RenderBuffer*       CreationHelper::createObjectOfType<ramses::RenderBuffer>(std::string_view name);
    template <> ramses::RenderTarget*       CreationHelper::createObjectOfType<ramses::RenderTarget>(std::string_view name);
    template <> ramses::Geometry*           CreationHelper::createObjectOfType<ramses::Geometry>(std::string_view name);
    template <> ramses::DataObject*         CreationHelper::createObjectOfType<ramses::DataObject>(std::string_view name);
    template <> ramses::ArrayBuffer*        CreationHelper::createObjectOfType<ramses::ArrayBuffer>(std::string_view name);
    template <> ramses::Texture2DBuffer*    CreationHelper::createObjectOfType<ramses::Texture2DBuffer>(std::string_view name);
    template <> ramses::PickableObject*     CreationHelper::createObjectOfType<ramses::PickableObject>(std::string_view name);
    template <> ramses::SceneReference*     CreationHelper::createObjectOfType<ramses::SceneReference>(std::string_view name);
}
