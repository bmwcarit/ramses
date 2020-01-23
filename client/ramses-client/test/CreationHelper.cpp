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
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
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
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"
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
        return m_ramsesClient->createEffect(effectDescription, ResourceCacheFlag_DoNotCache, name);
    }
    template <> Appearance* CreationHelper::createObjectOfType<Appearance>(const char* name)
    {
        return m_scene->createAppearance(*TestEffects::CreateTestEffect(*m_ramsesClient), name);
    }
    template <> Texture2D* CreationHelper::createObjectOfType<Texture2D>(const char* name)
    {
        uint8_t data[4] = { 0u };
        MipLevelData mipLevelData(sizeof(data), data);
        return m_ramsesClient->createTexture2D(1u, 1u, ETextureFormat_RGBA8, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, name);
    }
    template <> Texture3D* CreationHelper::createObjectOfType<Texture3D>(const char* name)
    {
        uint8_t data[32] = { 0u };
        MipLevelData mipLevelData(sizeof(data), data);
        return m_ramsesClient->createTexture3D(1u, 2u, 4u, ETextureFormat_RGBA8, 1, &mipLevelData, false, ResourceCacheFlag_DoNotCache, name);
    }
    template <> TextureCube* CreationHelper::createObjectOfType<TextureCube>(const char* name)
    {
        uint8_t data[4] = { 0u };
        CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        return m_ramsesClient->createTextureCube(1u, ETextureFormat_RGBA8, 1, &mipLevelData, false, ResourceCacheFlag_DoNotCache, name);
    }
    template <> UInt16Array* CreationHelper::createObjectOfType<UInt16Array>(const char* name)
    {
        const uint16_t data = 0u;
        return const_cast<UInt16Array*>(m_ramsesClient->createConstUInt16Array(1u, &data, ResourceCacheFlag_DoNotCache, name));
    }
    template <> UInt32Array* CreationHelper::createObjectOfType<UInt32Array>(const char* name)
    {
        const uint32_t data[5] = {0u, 3u, 6u, 9u, 12u };
        return const_cast<UInt32Array*>(m_ramsesClient->createConstUInt32Array(5u, &data[0], ResourceCacheFlag_DoNotCache, name));
    }
    template <> FloatArray* CreationHelper::createObjectOfType<FloatArray>(const char* name)
    {
        const float data[1] = { 0.f };
        return const_cast<FloatArray*>(m_ramsesClient->createConstFloatArray(1u, data, ResourceCacheFlag_DoNotCache, name));
    }
    template <> Vector2fArray* CreationHelper::createObjectOfType<Vector2fArray>(const char* name)
    {
        const float data[2] = { 0.f };
        return const_cast<Vector2fArray*>(m_ramsesClient->createConstVector2fArray(1u, data, ResourceCacheFlag_DoNotCache, name));
    }
    template <> Vector3fArray* CreationHelper::createObjectOfType<Vector3fArray>(const char* name)
    {
        const float data[3] = { 0.f };
        return const_cast<Vector3fArray*>(m_ramsesClient->createConstVector3fArray(1u, data, ResourceCacheFlag_DoNotCache, name));
    }
    template <> Vector4fArray* CreationHelper::createObjectOfType<Vector4fArray>(const char* name)
    {
        const float data[4] = { 0.f };
        return const_cast<Vector4fArray*>(m_ramsesClient->createConstVector4fArray(1u, data, ResourceCacheFlag_DoNotCache, name));
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
        Texture2D* texture = m_ramsesClient->createTexture2D(1u, 1u, ETextureFormat_RGBA8, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, "texture");
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

    Effect* createFakeEffectWithPositions(RamsesClient& client)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(
            "attribute vec3 vec3input; \n"
            "void main()\n"
            "{\n"
            "  gl_Position = vec4(vec3input, 1.0); \n"
            "}\n");
        effectDesc.setFragmentShader(
            "void main(void)\n"
            "{\n"
            "  gl_FragColor = vec4(0.0); \n"
            "}\n");
        return client.impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, "effect");
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
        Texture2D* fallback = m_ramsesClient->createTexture2D(1u, 1u, ETextureFormat_RGBA8, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, "fallbackTex");
        StreamTexture* streamTexture = m_scene->createStreamTexture(*fallback, streamSource_t(0), name);
        return streamTexture;
    }

    template <> IndexDataBuffer* CreationHelper::createObjectOfType<IndexDataBuffer>(const char* name)
    {
        return m_scene->createIndexDataBuffer(13 * sizeof(uint32_t), EDataType_UInt32, name);

    }
    template <> VertexDataBuffer* CreationHelper::createObjectOfType<VertexDataBuffer>(const char* name)
    {
        return m_scene->createVertexDataBuffer(13 * sizeof(uint32_t), EDataType_Float, name);
    }
    template <> Texture2DBuffer* CreationHelper::createObjectOfType<Texture2DBuffer>(const char* name)
    {
        return m_scene->createTexture2DBuffer(2, 3, 4, ETextureFormat_RGBA8, name);
    }

    template <> PickableObject* CreationHelper::createObjectOfType<PickableObject>(const char* name)
    {
        const auto vb = m_scene->createVertexDataBuffer(3 * sizeof(DataVector3f), EDataType_Vector3F, "vb");
        m_additionalAllocatedSceneObjects.push_back(vb);
        PerspectiveCamera* camera =  m_scene->createPerspectiveCamera("pickableCamera");
        m_additionalAllocatedSceneObjects.push_back(camera);
        camera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
        camera->setViewport(0, 0, 200, 200);
        PickableObject* pickableObject =  m_scene->createPickableObject(*vb, pickableObjectId_t{ 123u }, name);
        pickableObject->setCamera(*camera);
        return pickableObject;
    }
}
