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
        ObjectType* createObjectOfType(const char* name)
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

    template <> RamsesClient*              CreationHelper::createObjectOfType<RamsesClient             >(const char* name);
    template <> Scene*                     CreationHelper::createObjectOfType<Scene                    >(const char* name);
    template <> Node*                      CreationHelper::createObjectOfType<Node                     >(const char* name);
    template <> MeshNode*                  CreationHelper::createObjectOfType<MeshNode                 >(const char* name);
    template <> PerspectiveCamera*         CreationHelper::createObjectOfType<PerspectiveCamera        >(const char* name);
    template <> OrthographicCamera*        CreationHelper::createObjectOfType<OrthographicCamera       >(const char* name);
    template <> Effect*                    CreationHelper::createObjectOfType<Effect                   >(const char* name);
    template <> Appearance*                CreationHelper::createObjectOfType<Appearance               >(const char* name);
    template <> Texture2D*                 CreationHelper::createObjectOfType<Texture2D                >(const char* name);
    template <> Texture3D*                 CreationHelper::createObjectOfType<Texture3D                >(const char* name);
    template <> TextureCube*               CreationHelper::createObjectOfType<TextureCube              >(const char* name);
    template <> ArrayResource*             CreationHelper::createObjectOfType<ArrayResource            >(const char* name);
    template <> RenderGroup*               CreationHelper::createObjectOfType<RenderGroup              >(const char* name);
    template <> RenderPass*                CreationHelper::createObjectOfType<RenderPass               >(const char* name);
    template <> BlitPass*                  CreationHelper::createObjectOfType<BlitPass                 >(const char* name);
    template <> TextureSampler*            CreationHelper::createObjectOfType<TextureSampler           >(const char* name);
    template <> TextureSamplerMS*          CreationHelper::createObjectOfType<TextureSamplerMS         >(const char* name);
    template <> RenderBuffer*              CreationHelper::createObjectOfType<RenderBuffer             >(const char* name);
    template <> RenderTarget*              CreationHelper::createObjectOfType<RenderTarget             >(const char* name);
    template <> GeometryBinding*           CreationHelper::createObjectOfType<GeometryBinding          >(const char* name);
    template <> DataObject*                CreationHelper::createObjectOfType<DataObject               >(const char* name);
    template <> ArrayBuffer*               CreationHelper::createObjectOfType<ArrayBuffer              >(const char* name);
    template <> Texture2DBuffer*           CreationHelper::createObjectOfType<Texture2DBuffer          >(const char* name);
    template <> PickableObject*            CreationHelper::createObjectOfType<PickableObject           >(const char* name);
    template <> SceneReference*            CreationHelper::createObjectOfType<SceneReference           >(const char* name);
}

#endif
