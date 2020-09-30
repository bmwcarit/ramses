//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "CreationHelper.h"
#include "TestEffects.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepInt32.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/SplineStepVector2f.h"
#include "ramses-client-api/SplineStepVector3f.h"
#include "ramses-client-api/SplineStepVector4f.h"
#include "ramses-client-api/SplineStepVector2i.h"
#include "ramses-client-api/SplineStepVector3i.h"
#include "ramses-client-api/SplineStepVector4i.h"
#include "ramses-client-api/SplineLinearInt32.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearVector2f.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineLinearVector2i.h"
#include "ramses-client-api/SplineLinearVector3i.h"
#include "ramses-client-api/SplineLinearVector4i.h"
#include "ramses-client-api/SplineBezierInt32.h"
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/SplineBezierVector2f.h"
#include "ramses-client-api/SplineBezierVector3f.h"
#include "ramses-client-api/SplineBezierVector4f.h"
#include "ramses-client-api/SplineBezierVector2i.h"
#include "ramses-client-api/SplineBezierVector3i.h"
#include "ramses-client-api/SplineBezierVector4i.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/EffectInputSemantic.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4i.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/LocalCamera.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/PerspectiveCamera.h"

namespace ramses
{
    CreationHelper::CreationHelper(Scene* scene, AnimationSystem* animSystem, RamsesClient* ramsesClient)
        : m_scene(scene)
        , m_animationSystem(animSystem)
        , m_ramsesClient(ramsesClient)
    {
    }

    CreationHelper::~CreationHelper()
    {
        for(const auto& component : m_allocatedClientAndFrameworkComponents)
        {
            component.second->destroyClient(*component.first);
            delete component.second;
        }

        if (m_animationSystem != nullptr)
        {
            destroyAdditionalAllocatedAnimationSystemObjects();
        }

        if (m_scene != nullptr)
        {
            destroyAdditionalAllocatedSceneObjects();
        }
    }

    void CreationHelper::setScene(Scene* scene)
    {
        m_scene = scene;
    }

    void CreationHelper::setAnimationSystem(AnimationSystem* animSystem)
    {
        m_animationSystem = animSystem;
    }

    void CreationHelper::destroyAdditionalAllocatedSceneObjects()
    {
        assert(m_scene != nullptr);
        for(const auto& obj : m_additionalAllocatedSceneObjects)
        {
            ASSERT_TRUE(m_scene->destroy(*obj) == StatusOK);
        }
        m_additionalAllocatedSceneObjects.clear();
    }

    size_t CreationHelper::getAdditionalAllocatedNodeCount() const
    {
        size_t nodeCount = 0;
        for (auto obj : m_additionalAllocatedSceneObjects)
        {
            if (obj->isOfType(ERamsesObjectType_Node))
                ++nodeCount;
        }
        return nodeCount;
    }

    void CreationHelper::destroyAdditionalAllocatedAnimationSystemObjects()
    {
        assert(m_animationSystem != nullptr);
        for (const auto& obj : m_additionalAllocatedAnimationSystemObjects)
        {
            ASSERT_TRUE(m_animationSystem->destroy(*obj) == StatusOK);
        }
        m_additionalAllocatedAnimationSystemObjects.clear();
    }

    template <> RamsesClient* CreationHelper::createObjectOfType<RamsesClient>(const char* name)
    {
        ramses::RamsesFramework* framework = new ramses::RamsesFramework;
        RamsesClient* allocatedClient = framework->createClient(name);
        if (allocatedClient)
            m_allocatedClientAndFrameworkComponents.push_back(ClientAndFramework(allocatedClient, framework));
        return allocatedClient;
    }
    template <> Scene* CreationHelper::createObjectOfType<Scene>(const char* name)
    {
        return m_ramsesClient->createScene(sceneId_t(999u), ramses::SceneConfig(), name);
    }
    template <> AnimationSystem* CreationHelper::createObjectOfType<AnimationSystem>(const char* name)
    {
        return m_scene->createAnimationSystem(ramses::EAnimationSystemFlags_Default, name);
    }
    template <> AnimationSystemRealTime* CreationHelper::createObjectOfType<AnimationSystemRealTime>(const char* name)
    {
        return m_scene->createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_Default, name);
    }
    template <> Node* CreationHelper::createObjectOfType<Node>(const char* name)
    {
        return m_scene->createNode(name);
    }
    template <> MeshNode* CreationHelper::createObjectOfType<MeshNode>(const char* name)
    {
        return m_scene->createMeshNode(name);
    }
    template <> RemoteCamera* CreationHelper::createObjectOfType<RemoteCamera>(const char* name)
    {
        return m_scene->createRemoteCamera(name);
    }
    template <> PerspectiveCamera* CreationHelper::createObjectOfType<PerspectiveCamera>(const char* name)
    {
        return m_scene->createPerspectiveCamera(name);
    }
    template <> OrthographicCamera* CreationHelper::createObjectOfType<OrthographicCamera>(const char* name)
    {
        return m_scene->createOrthographicCamera(name);
    }
    template <> Effect* CreationHelper::createObjectOfType<Effect>(const char* name)
    {
        EffectDescription effectDescription;
        effectDescription.setVertexShader("void main(void) {gl_Position=vec4(0);}");
        effectDescription.setFragmentShader("void main(void) {gl_FragColor=vec4(1);}");
        return m_scene->createEffect(effectDescription, ResourceCacheFlag_DoNotCache, name);
    }
    template <> AnimatedProperty* CreationHelper::createObjectOfType<AnimatedProperty>(const char* name)
    {
        Node& node = *m_scene->createNode("node");
        m_additionalAllocatedSceneObjects.push_back(&node);
        return m_animationSystem->createAnimatedProperty(node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All, name);
    }
    template <> Animation* CreationHelper::createObjectOfType<Animation>(const char* name)
    {
        Node& node = *m_scene->createNode("node");
        m_additionalAllocatedSceneObjects.push_back(&node);
        AnimatedProperty& prop = *m_animationSystem->createAnimatedProperty(node, EAnimatedProperty_Translation);
        SplineLinearVector3f& spline = *m_animationSystem->createSplineLinearVector3f("spline");
        m_additionalAllocatedAnimationSystemObjects.push_back(&prop);
        m_additionalAllocatedAnimationSystemObjects.push_back(&spline);
        return m_animationSystem->createAnimation(prop, spline, name);
    }
    template <> AnimationSequence* CreationHelper::createObjectOfType<AnimationSequence>(const char* name)
    {
        return m_animationSystem->createAnimationSequence(name);
    }
    template <> Appearance* CreationHelper::createObjectOfType<Appearance>(const char* name)
    {
        return m_scene->createAppearance(*TestEffects::CreateTestEffect(*m_scene), name);
    }
    template <> SplineStepBool* CreationHelper::createObjectOfType<SplineStepBool      >(const char* name)
    {
        return m_animationSystem->createSplineStepBool(name);
    }
    template <> SplineStepFloat* CreationHelper::createObjectOfType<SplineStepFloat     >(const char* name)
    {
        return m_animationSystem->createSplineStepFloat(name);
    }
    template <> SplineStepInt32* CreationHelper::createObjectOfType<SplineStepInt32     >(const char* name)
    {
        return m_animationSystem->createSplineStepInt32(name);
    }
    template <> SplineStepVector2f* CreationHelper::createObjectOfType<SplineStepVector2f  >(const char* name)
    {
        return m_animationSystem->createSplineStepVector2f(name);
    }
    template <> SplineStepVector3f* CreationHelper::createObjectOfType<SplineStepVector3f  >(const char* name)
    {
        return m_animationSystem->createSplineStepVector3f(name);
    }
    template <> SplineStepVector4f* CreationHelper::createObjectOfType<SplineStepVector4f  >(const char* name)
    {
        return m_animationSystem->createSplineStepVector4f(name);
    }
    template <> SplineStepVector2i* CreationHelper::createObjectOfType<SplineStepVector2i  >(const char* name)
    {
        return m_animationSystem->createSplineStepVector2i(name);
    }
    template <> SplineStepVector3i* CreationHelper::createObjectOfType<SplineStepVector3i  >(const char* name)
    {
        return m_animationSystem->createSplineStepVector3i(name);
    }
    template <> SplineStepVector4i* CreationHelper::createObjectOfType<SplineStepVector4i  >(const char* name)
    {
        return m_animationSystem->createSplineStepVector4i(name);
    }
    template <> SplineLinearFloat* CreationHelper::createObjectOfType<SplineLinearFloat   >(const char* name)
    {
        return m_animationSystem->createSplineLinearFloat(name);
    }
    template <> SplineLinearInt32* CreationHelper::createObjectOfType<SplineLinearInt32   >(const char* name)
    {
        return m_animationSystem->createSplineLinearInt32(name);
    }
    template <> SplineLinearVector2f* CreationHelper::createObjectOfType<SplineLinearVector2f>(const char* name)
    {
        return m_animationSystem->createSplineLinearVector2f(name);
    }
    template <> SplineLinearVector3f* CreationHelper::createObjectOfType<SplineLinearVector3f>(const char* name)
    {
        return m_animationSystem->createSplineLinearVector3f(name);
    }
    template <> SplineLinearVector4f* CreationHelper::createObjectOfType<SplineLinearVector4f>(const char* name)
    {
        return m_animationSystem->createSplineLinearVector4f(name);
    }
    template <> SplineLinearVector2i* CreationHelper::createObjectOfType<SplineLinearVector2i>(const char* name)
    {
        return m_animationSystem->createSplineLinearVector2i(name);
    }
    template <> SplineLinearVector3i* CreationHelper::createObjectOfType<SplineLinearVector3i>(const char* name)
    {
        return m_animationSystem->createSplineLinearVector3i(name);
    }
    template <> SplineLinearVector4i* CreationHelper::createObjectOfType<SplineLinearVector4i>(const char* name)
    {
        return m_animationSystem->createSplineLinearVector4i(name);
    }
    template <> SplineBezierFloat* CreationHelper::createObjectOfType<SplineBezierFloat   >(const char* name)
    {
        return m_animationSystem->createSplineBezierFloat(name);
    }
    template <> SplineBezierInt32* CreationHelper::createObjectOfType<SplineBezierInt32   >(const char* name)
    {
        return m_animationSystem->createSplineBezierInt32(name);
    }
    template <> SplineBezierVector2f* CreationHelper::createObjectOfType<SplineBezierVector2f>(const char* name)
    {
        return m_animationSystem->createSplineBezierVector2f(name);
    }
    template <> SplineBezierVector3f* CreationHelper::createObjectOfType<SplineBezierVector3f>(const char* name)
    {
        return m_animationSystem->createSplineBezierVector3f(name);
    }
    template <> SplineBezierVector4f* CreationHelper::createObjectOfType<SplineBezierVector4f>(const char* name)
    {
        return m_animationSystem->createSplineBezierVector4f(name);
    }
    template <> SplineBezierVector2i* CreationHelper::createObjectOfType<SplineBezierVector2i>(const char* name)
    {
        return m_animationSystem->createSplineBezierVector2i(name);
    }
    template <> SplineBezierVector3i* CreationHelper::createObjectOfType<SplineBezierVector3i>(const char* name)
    {
        return m_animationSystem->createSplineBezierVector3i(name);
    }
    template <> SplineBezierVector4i* CreationHelper::createObjectOfType<SplineBezierVector4i>(const char* name)
    {
        return m_animationSystem->createSplineBezierVector4i(name);
    }
    template <> Texture2D* CreationHelper::createObjectOfType<Texture2D>(const char* name)
    {
        uint8_t data[4] = { 0u };
        MipLevelData mipLevelData(sizeof(data), data);
        return m_scene->createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, name);
    }
    template <> Texture3D* CreationHelper::createObjectOfType<Texture3D>(const char* name)
    {
        uint8_t data[32] = { 0u };
        MipLevelData mipLevelData(sizeof(data), data);
        return m_scene->createTexture3D(ETextureFormat::RGBA8, 1u, 2u, 4u, 1, &mipLevelData, false, ResourceCacheFlag_DoNotCache, name);
    }
    template <> TextureCube* CreationHelper::createObjectOfType<TextureCube>(const char* name)
    {
        uint8_t data[4] = { 0u };
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        return m_scene->createTextureCube(ETextureFormat::RGBA8, 1u, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, name);
    }
    template <> ArrayResource* CreationHelper::createObjectOfType<ArrayResource>(const char* name)
    {
        const uint16_t data = 0u;
        return m_scene->createArrayResource(EDataType::UInt16, 1u, &data, ResourceCacheFlag_DoNotCache, name);
    }

    template <> RenderGroup* CreationHelper::createObjectOfType<RenderGroup>(const char* name)
    {
        return m_scene->createRenderGroup(name);
    }
    template <> RenderPass* CreationHelper::createObjectOfType<RenderPass>(const char* name)
    {
        return m_scene->createRenderPass(name);
    }

    template <> BlitPass* CreationHelper::createObjectOfType<BlitPass>(const char* name)
    {
        const RenderBuffer* sourceRenderBuffer = createObjectOfType<RenderBuffer>("src render buffer");
        const RenderBuffer* destinationRenderBuffer = createObjectOfType<RenderBuffer>("dst render buffer");
        return m_scene->createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, name);
    }

    template <> TextureSampler* CreationHelper::createObjectOfType<TextureSampler>(const char* name)
    {
        uint8_t data[4] = { 0u };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* texture = m_scene->createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, "texture");
        return m_scene->createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Mirror, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Nearest, *texture, 1u, name);
    }
    template <> RenderBuffer* CreationHelper::createObjectOfType<RenderBuffer>(const char* name)
    {
        return m_scene->createRenderBuffer(600, 400, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u, name);
    }
    template <> RenderTarget* CreationHelper::createObjectOfType<RenderTarget>(const char* name)
    {
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*createObjectOfType<RenderBuffer>("rb"));
        return m_scene->createRenderTarget(rtDesc, name);
    }

    template <> GeometryBinding* CreationHelper::createObjectOfType<GeometryBinding>(const char* name)
    {
        return m_scene->createGeometryBinding(*createObjectOfType<Effect>("geometry_binding_effect"), name);
    }

    template <> DataFloat* CreationHelper::createObjectOfType<DataFloat>(const char* name)
    {
        return m_scene->createDataFloat(name);
    }

    template <> DataVector2f* CreationHelper::createObjectOfType<DataVector2f>(const char* name)
    {
        return m_scene->createDataVector2f(name);
    }

    template <> DataVector3f* CreationHelper::createObjectOfType<DataVector3f>(const char* name)
    {
        return m_scene->createDataVector3f(name);
    }

    template <> DataVector4f* CreationHelper::createObjectOfType<DataVector4f>(const char* name)
    {
        return m_scene->createDataVector4f(name);
    }

    template <> DataMatrix22f* CreationHelper::createObjectOfType<DataMatrix22f>(const char* name)
    {
        return m_scene->createDataMatrix22f(name);
    }

    template <> DataMatrix33f* CreationHelper::createObjectOfType<DataMatrix33f>(const char* name)
    {
        return m_scene->createDataMatrix33f(name);
    }

    template <> DataMatrix44f* CreationHelper::createObjectOfType<DataMatrix44f>(const char* name)
    {
        return m_scene->createDataMatrix44f(name);
    }

    template <> DataInt32* CreationHelper::createObjectOfType<DataInt32>(const char* name)
    {
        return m_scene->createDataInt32(name);
    }

    template <> DataVector2i* CreationHelper::createObjectOfType<DataVector2i>(const char* name)
    {
        return m_scene->createDataVector2i(name);
    }

    template <> DataVector3i* CreationHelper::createObjectOfType<DataVector3i>(const char* name)
    {
        return m_scene->createDataVector3i(name);
    }
    template <> DataVector4i* CreationHelper::createObjectOfType<DataVector4i>(const char* name)
    {
        return m_scene->createDataVector4i(name);
    }
    template <> StreamTexture* CreationHelper::createObjectOfType<StreamTexture>(const char* name)
    {
        uint8_t data[4] = { 0u };
        MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* fallback = m_scene->createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, "fallbackTex");
        StreamTexture* streamTexture = m_scene->createStreamTexture(*fallback, waylandIviSurfaceId_t(0), name);
        return streamTexture;
    }

    template <> ArrayBuffer* CreationHelper::createObjectOfType<ArrayBuffer>(const char* name)
    {
        return m_scene->createArrayBuffer(EDataType::UInt32, 13u, name);

    }
    template <> Texture2DBuffer* CreationHelper::createObjectOfType<Texture2DBuffer>(const char* name)
    {
        return m_scene->createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2, name);
    }

    template <> PickableObject* CreationHelper::createObjectOfType<PickableObject>(const char* name)
    {
        const auto vb = m_scene->createArrayBuffer(EDataType::Vector3F, 3u, "vb");
        m_additionalAllocatedSceneObjects.push_back(vb);
        PerspectiveCamera* camera =  m_scene->createPerspectiveCamera("pickableCamera");
        m_additionalAllocatedSceneObjects.push_back(camera);
        camera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
        camera->setViewport(0, 0, 200, 200);
        PickableObject* pickableObject =  m_scene->createPickableObject(*vb, pickableObjectId_t{ 123u }, name);
        pickableObject->setCamera(*camera);
        return pickableObject;
    }

    template <> SceneReference* CreationHelper::createObjectOfType<SceneReference>(const char* name)
    {
        // SceneReference cannot refer to same sceneID, create a different one every time
        ++m_lastReferencedSceneId.getReference();
        return m_scene->createSceneReference(m_lastReferencedSceneId, name);
    }
}
