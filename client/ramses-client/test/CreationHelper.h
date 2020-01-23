//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CREATIONHELPER_H
#define RAMSES_CREATIONHELPER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/Vector.h"
#include "Collections/Pair.h"

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
    class RemoteCamera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Effect;
    class Appearance;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class UInt16Array;
    class UInt32Array;
    class FloatArray;
    class Vector2fArray;
    class Vector3fArray;
    class Vector4fArray;
    class RenderGroup;
    class RenderPass;
    class BlitPass;
    class TextureSampler;
    class RenderBuffer;
    class RenderTarget;
    class RenderGroup;
    class IndexDataBuffer;
    class VertexDataBuffer;
    class Texture2DBuffer;
    class GeometryBinding;
    class DataFloat;
    class DataVector2f;
    class DataVector3f;
    class DataVector4f;
    class DataMatrix22f;
    class DataMatrix33f;
    class DataMatrix44f;
    class DataInt32;
    class DataVector2i;
    class DataVector3i;
    class DataVector4i;
    class StreamTexture;
    class PickableObject;

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
        size_t getAdditionalAllocatedNodeCount() const;

    private:
        Scene* m_scene;
        RamsesClient* m_ramsesClient;

        typedef std::pair<RamsesClient*, ramses::RamsesFramework*> ClientAndFramework;
        typedef std::vector<ClientAndFramework> RamsesClientAndFrameworkComponentVector;
        RamsesClientAndFrameworkComponentVector m_allocatedClientAndFrameworkComponents;
        std::vector<SceneObject*> m_additionalAllocatedSceneObjects;
    };

    template <> RamsesClient*              CreationHelper::createObjectOfType<RamsesClient             >(const char* name);
    template <> Scene*                     CreationHelper::createObjectOfType<Scene                    >(const char* name);
    template <> Node*                      CreationHelper::createObjectOfType<Node                     >(const char* name);
    template <> MeshNode*                  CreationHelper::createObjectOfType<MeshNode                 >(const char* name);
    template <> RemoteCamera*              CreationHelper::createObjectOfType<RemoteCamera             >(const char* name);
    template <> PerspectiveCamera*         CreationHelper::createObjectOfType<PerspectiveCamera        >(const char* name);
    template <> OrthographicCamera*        CreationHelper::createObjectOfType<OrthographicCamera       >(const char* name);
    template <> Effect*                    CreationHelper::createObjectOfType<Effect                   >(const char* name);
    template <> Appearance*                CreationHelper::createObjectOfType<Appearance               >(const char* name);
    template <> Texture2D*                 CreationHelper::createObjectOfType<Texture2D                >(const char* name);
    template <> Texture3D*                 CreationHelper::createObjectOfType<Texture3D                >(const char* name);
    template <> TextureCube*               CreationHelper::createObjectOfType<TextureCube              >(const char* name);
    template <> UInt16Array*               CreationHelper::createObjectOfType<UInt16Array              >(const char* name);
    template <> UInt32Array*               CreationHelper::createObjectOfType<UInt32Array              >(const char* name);
    template <> FloatArray*                CreationHelper::createObjectOfType<FloatArray               >(const char* name);
    template <> Vector2fArray*             CreationHelper::createObjectOfType<Vector2fArray            >(const char* name);
    template <> Vector3fArray*             CreationHelper::createObjectOfType<Vector3fArray            >(const char* name);
    template <> Vector4fArray*             CreationHelper::createObjectOfType<Vector4fArray            >(const char* name);
    template <> RenderGroup*               CreationHelper::createObjectOfType<RenderGroup              >(const char* name);
    template <> RenderPass*                CreationHelper::createObjectOfType<RenderPass               >(const char* name);
    template <> BlitPass*                  CreationHelper::createObjectOfType<BlitPass                 >(const char* name);
    template <> TextureSampler*            CreationHelper::createObjectOfType<TextureSampler           >(const char* name);
    template <> RenderBuffer*              CreationHelper::createObjectOfType<RenderBuffer             >(const char* name);
    template <> RenderTarget*              CreationHelper::createObjectOfType<RenderTarget             >(const char* name);
    template <> GeometryBinding*           CreationHelper::createObjectOfType<GeometryBinding          >(const char* name);
    template <> DataFloat*                 CreationHelper::createObjectOfType<DataFloat                >(const char* name);
    template <> DataVector2f*              CreationHelper::createObjectOfType<DataVector2f             >(const char* name);
    template <> DataVector3f*              CreationHelper::createObjectOfType<DataVector3f             >(const char* name);
    template <> DataVector4f*              CreationHelper::createObjectOfType<DataVector4f             >(const char* name);
    template <> DataMatrix22f*             CreationHelper::createObjectOfType<DataMatrix22f            >(const char* name);
    template <> DataMatrix33f*             CreationHelper::createObjectOfType<DataMatrix33f            >(const char* name);
    template <> DataMatrix44f*             CreationHelper::createObjectOfType<DataMatrix44f            >(const char* name);
    template <> DataInt32*                 CreationHelper::createObjectOfType<DataInt32                >(const char* name);
    template <> DataVector2i*              CreationHelper::createObjectOfType<DataVector2i             >(const char* name);
    template <> DataVector3i*              CreationHelper::createObjectOfType<DataVector3i             >(const char* name);
    template <> DataVector4i*              CreationHelper::createObjectOfType<DataVector4i             >(const char* name);
    template <> StreamTexture*             CreationHelper::createObjectOfType<StreamTexture            >(const char* name);
    template <> IndexDataBuffer*           CreationHelper::createObjectOfType<IndexDataBuffer          >(const char* name);
    template <> VertexDataBuffer*          CreationHelper::createObjectOfType<VertexDataBuffer         >(const char* name);
    template <> Texture2DBuffer*           CreationHelper::createObjectOfType<Texture2DBuffer          >(const char* name);
    template <> PickableObject*            CreationHelper::createObjectOfType<PickableObject           >(const char* name);
}

#endif
