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
    class AnimationObject;
    class RamsesClient;
    class RamsesFramework;
    class TextureCubeInput;
    class Scene;
    class AnimationSystem;
    class AnimationSystemRealTime;
    class Node;
    class MeshNode;
    class RemoteCamera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Effect;
    class AnimatedProperty;
    class Animation;
    class AnimationSequence;
    class AnimatedSetter;
    class Appearance;
    class SplineStepBool;
    class SplineStepFloat;
    class SplineStepInt32;
    class SplineStepVector2f;
    class SplineStepVector3f;
    class SplineStepVector4f;
    class SplineStepVector2i;
    class SplineStepVector3i;
    class SplineStepVector4i;
    class SplineLinearFloat;
    class SplineLinearInt32;
    class SplineLinearVector2f;
    class SplineLinearVector3f;
    class SplineLinearVector4f;
    class SplineLinearVector2i;
    class SplineLinearVector3i;
    class SplineLinearVector4i;
    class SplineBezierFloat;
    class SplineBezierInt32;
    class SplineBezierVector2f;
    class SplineBezierVector3f;
    class SplineBezierVector4f;
    class SplineBezierVector2i;
    class SplineBezierVector3i;
    class SplineBezierVector4i;
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

    class CreationHelper
    {
    public:
        CreationHelper(Scene* scene, AnimationSystem* animSystem, RamsesClient* ramsesClient);
        ~CreationHelper();

        void setScene(Scene* scene);
        void setAnimationSystem(AnimationSystem* animSystem);

        template <typename ObjectType>
        ObjectType* createObjectOfType(const char* name)
        {
            UNUSED(name);
            assert(false);
            return NULL;
        }

        void destroyAdditionalAllocatedSceneObjects();
        void destroyAdditionalAllocatedAnimationSystemObjects();

    private:
        Scene* m_scene;
        AnimationSystem* m_animationSystem;
        RamsesClient* m_ramsesClient;

        typedef std::pair<RamsesClient*, ramses::RamsesFramework*> ClientAndFramework;
        typedef std::vector<ClientAndFramework> RamsesClientAndFrameworkComponentVector;
        RamsesClientAndFrameworkComponentVector m_allocatedClientAndFrameworkComponents;
        std::vector<SceneObject*> m_additionalAllocatedSceneObjects;
        std::vector<AnimationObject*> m_additionalAllocatedAnimationSystemObjects;
    };

    template <> RamsesClient*              CreationHelper::createObjectOfType<RamsesClient             >(const char* name);
    template <> Scene*                     CreationHelper::createObjectOfType<Scene                    >(const char* name);
    template <> AnimationSystem*           CreationHelper::createObjectOfType<AnimationSystem          >(const char* name);
    template <> AnimationSystemRealTime*   CreationHelper::createObjectOfType<AnimationSystemRealTime  >(const char* name);
    template <> Node*                      CreationHelper::createObjectOfType<Node                     >(const char* name);
    template <> MeshNode*                  CreationHelper::createObjectOfType<MeshNode                 >(const char* name);
    template <> RemoteCamera*              CreationHelper::createObjectOfType<RemoteCamera             >(const char* name);
    template <> PerspectiveCamera*         CreationHelper::createObjectOfType<PerspectiveCamera        >(const char* name);
    template <> OrthographicCamera*        CreationHelper::createObjectOfType<OrthographicCamera       >(const char* name);
    template <> Effect*                    CreationHelper::createObjectOfType<Effect                   >(const char* name);
    template <> AnimatedProperty*          CreationHelper::createObjectOfType<AnimatedProperty         >(const char* name);
    template <> Animation*                 CreationHelper::createObjectOfType<Animation                >(const char* name);
    template <> AnimationSequence*         CreationHelper::createObjectOfType<AnimationSequence        >(const char* name);
    template <> AnimatedSetter*            CreationHelper::createObjectOfType<AnimatedSetter           >(const char* name);
    template <> Appearance*                CreationHelper::createObjectOfType<Appearance               >(const char* name);
    template <> SplineStepBool*            CreationHelper::createObjectOfType<SplineStepBool           >(const char* name);
    template <> SplineStepFloat*           CreationHelper::createObjectOfType<SplineStepFloat          >(const char* name);
    template <> SplineStepInt32*           CreationHelper::createObjectOfType<SplineStepInt32          >(const char* name);
    template <> SplineStepVector2f*        CreationHelper::createObjectOfType<SplineStepVector2f       >(const char* name);
    template <> SplineStepVector3f*        CreationHelper::createObjectOfType<SplineStepVector3f       >(const char* name);
    template <> SplineStepVector4f*        CreationHelper::createObjectOfType<SplineStepVector4f       >(const char* name);
    template <> SplineStepVector2i*        CreationHelper::createObjectOfType<SplineStepVector2i       >(const char* name);
    template <> SplineStepVector3i*        CreationHelper::createObjectOfType<SplineStepVector3i       >(const char* name);
    template <> SplineStepVector4i*        CreationHelper::createObjectOfType<SplineStepVector4i       >(const char* name);
    template <> SplineLinearFloat*         CreationHelper::createObjectOfType<SplineLinearFloat        >(const char* name);
    template <> SplineLinearInt32*         CreationHelper::createObjectOfType<SplineLinearInt32        >(const char* name);
    template <> SplineLinearVector2f*      CreationHelper::createObjectOfType<SplineLinearVector2f     >(const char* name);
    template <> SplineLinearVector3f*      CreationHelper::createObjectOfType<SplineLinearVector3f     >(const char* name);
    template <> SplineLinearVector4f*      CreationHelper::createObjectOfType<SplineLinearVector4f     >(const char* name);
    template <> SplineLinearVector2i*      CreationHelper::createObjectOfType<SplineLinearVector2i     >(const char* name);
    template <> SplineLinearVector3i*      CreationHelper::createObjectOfType<SplineLinearVector3i     >(const char* name);
    template <> SplineLinearVector4i*      CreationHelper::createObjectOfType<SplineLinearVector4i     >(const char* name);
    template <> SplineBezierFloat*         CreationHelper::createObjectOfType<SplineBezierFloat        >(const char* name);
    template <> SplineBezierInt32*         CreationHelper::createObjectOfType<SplineBezierInt32        >(const char* name);
    template <> SplineBezierVector2f*      CreationHelper::createObjectOfType<SplineBezierVector2f     >(const char* name);
    template <> SplineBezierVector3f*      CreationHelper::createObjectOfType<SplineBezierVector3f     >(const char* name);
    template <> SplineBezierVector4f*      CreationHelper::createObjectOfType<SplineBezierVector4f     >(const char* name);
    template <> SplineBezierVector2i*      CreationHelper::createObjectOfType<SplineBezierVector2i     >(const char* name);
    template <> SplineBezierVector3i*      CreationHelper::createObjectOfType<SplineBezierVector3i     >(const char* name);
    template <> SplineBezierVector4i*      CreationHelper::createObjectOfType<SplineBezierVector4i     >(const char* name);
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
}

#endif
