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
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/EffectInputSemantic.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/PerspectiveCamera.h"

namespace ramses
{
    CreationHelper::CreationHelper(Scene* scene, RamsesClient* ramsesClient)
        : m_scene(scene)
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

        if (m_scene != nullptr)
        {
            destroyAdditionalAllocatedSceneObjects();
        }
    }

    void CreationHelper::setScene(Scene* scene)
    {
        m_scene = scene;
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
    template <> Node* CreationHelper::createObjectOfType<Node>(const char* name)
    {
        return m_scene->createNode(name);
    }
    template <> MeshNode* CreationHelper::createObjectOfType<MeshNode>(const char* name)
    {
        return m_scene->createMeshNode(name);
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
    template <> Appearance* CreationHelper::createObjectOfType<Appearance>(const char* name)
    {
        return m_scene->createAppearance(*TestEffects::CreateTestEffect(*m_scene, "appearance effect"), name);
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
        return m_scene->createArrayResource(1u, &data, ResourceCacheFlag_DoNotCache, name);
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
    template <> TextureSamplerMS* CreationHelper::createObjectOfType<TextureSamplerMS>(const char* name)
    {
        RenderBuffer* renderBuffer = m_scene->createRenderBuffer(16, 16, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 4u, "renderBuffer");
        return m_scene->createTextureSamplerMS(*renderBuffer, name);
    }
    template <> RenderBuffer* CreationHelper::createObjectOfType<RenderBuffer>(const char* name)
    {
        return m_scene->createRenderBuffer(16, 16, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u, name);
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

    template <> DataObject* CreationHelper::createObjectOfType<DataObject>(const char* name)
    {
        return m_scene->createDataObject(EDataType::Float, name);
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
