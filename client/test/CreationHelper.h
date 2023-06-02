//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CREATIONHELPER_H
#define RAMSES_CREATIONHELPER_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/Vector.h"
#include "Collections/Pair.h"

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
    class RenderBuffer;
    class RenderTarget;
    class RenderGroup;
    class ArrayBuffer;
    class Texture2DBuffer;
    class GeometryBinding;
    class DataObject;
    class PickableObject;
    class SceneReference;

    class CreationHelper
    {
    public:
        CreationHelper(Scene* scene, RamsesClient* ramsesClient);
        ~CreationHelper();

        void setScene(Scene* scene);

        template <typename ObjectType>
        ObjectType* createObjectOfType(std::string_view name)
        {
            UNUSED(name);
            assert(false);
            return NULL;
        }

        void destroyAdditionalAllocatedSceneObjects();
        [[nodiscard]] size_t getAdditionalAllocatedNodeCount() const;

    private:
        Scene* m_scene;
        RamsesClient* m_ramsesClient;

        using ClientAndFramework = std::pair<RamsesClient *, ramses::RamsesFramework *>;
        using RamsesClientAndFrameworkComponentVector = std::vector<ClientAndFramework>;
        RamsesClientAndFrameworkComponentVector m_allocatedClientAndFrameworkComponents;
        std::vector<SceneObject*> m_additionalAllocatedSceneObjects;
        sceneId_t m_lastReferencedSceneId{ 123u };
    };

    template <> RamsesClient*              CreationHelper::createObjectOfType<RamsesClient             >(std::string_view name);
    template <> Scene*                     CreationHelper::createObjectOfType<Scene                    >(std::string_view name);
    template <> Node*                      CreationHelper::createObjectOfType<Node                     >(std::string_view name);
    template <> MeshNode*                  CreationHelper::createObjectOfType<MeshNode                 >(std::string_view name);
    template <> PerspectiveCamera*         CreationHelper::createObjectOfType<PerspectiveCamera        >(std::string_view name);
    template <> OrthographicCamera*        CreationHelper::createObjectOfType<OrthographicCamera       >(std::string_view name);
    template <> Effect*                    CreationHelper::createObjectOfType<Effect                   >(std::string_view name);
    template <> Appearance*                CreationHelper::createObjectOfType<Appearance               >(std::string_view name);
    template <> Texture2D*                 CreationHelper::createObjectOfType<Texture2D                >(std::string_view name);
    template <> Texture3D*                 CreationHelper::createObjectOfType<Texture3D                >(std::string_view name);
    template <> TextureCube*               CreationHelper::createObjectOfType<TextureCube              >(std::string_view name);
    template <> ArrayResource*             CreationHelper::createObjectOfType<ArrayResource            >(std::string_view name);
    template <> RenderGroup*               CreationHelper::createObjectOfType<RenderGroup              >(std::string_view name);
    template <> RenderPass*                CreationHelper::createObjectOfType<RenderPass               >(std::string_view name);
    template <> BlitPass*                  CreationHelper::createObjectOfType<BlitPass                 >(std::string_view name);
    template <> TextureSampler*            CreationHelper::createObjectOfType<TextureSampler           >(std::string_view name);
    template <> TextureSamplerMS*          CreationHelper::createObjectOfType<TextureSamplerMS         >(std::string_view name);
    template <> RenderBuffer*              CreationHelper::createObjectOfType<RenderBuffer             >(std::string_view name);
    template <> RenderTarget*              CreationHelper::createObjectOfType<RenderTarget             >(std::string_view name);
    template <> GeometryBinding*           CreationHelper::createObjectOfType<GeometryBinding          >(std::string_view name);
    template <> DataObject*                CreationHelper::createObjectOfType<DataObject               >(std::string_view name);
    template <> ArrayBuffer*               CreationHelper::createObjectOfType<ArrayBuffer              >(std::string_view name);
    template <> Texture2DBuffer*           CreationHelper::createObjectOfType<Texture2DBuffer          >(std::string_view name);
    template <> PickableObject*            CreationHelper::createObjectOfType<PickableObject           >(std::string_view name);
    template <> SceneReference*            CreationHelper::createObjectOfType<SceneReference           >(std::string_view name);
}

#endif
