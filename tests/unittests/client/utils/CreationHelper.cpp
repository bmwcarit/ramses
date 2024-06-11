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
#include "ramses/client/Node.h"
#include "ramses/client/Scene.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Effect.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/EffectInputSemantic.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/Camera.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/logic/LogicEngine.h"

namespace ramses::internal
{
    CreationHelper::CreationHelper(ramses::Scene* scene, RamsesClient* ramsesClient)
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

    void CreationHelper::setScene(ramses::Scene* scene)
    {
        m_scene = scene;
    }

    void CreationHelper::destroyAdditionalAllocatedSceneObjects()
    {
        assert(m_scene != nullptr);
        for(const auto& obj : m_additionalAllocatedSceneObjects)
        {
            ASSERT_TRUE(m_scene->destroy(*obj));
        }
        m_additionalAllocatedSceneObjects.clear();
    }

    size_t CreationHelper::getAdditionalAllocatedNodeCount() const
    {
        size_t nodeCount = 0;
        for (auto obj : m_additionalAllocatedSceneObjects)
        {
            if (obj->isOfType(ERamsesObjectType::Node))
                ++nodeCount;
        }
        return nodeCount;
    }

    template <> RamsesClient* CreationHelper::createObjectOfType<RamsesClient>(std::string_view name)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        auto* framework = new RamsesFramework(config);
        RamsesClient* allocatedClient = framework->createClient(name);
        if (allocatedClient)
            m_allocatedClientAndFrameworkComponents.push_back(ClientAndFramework(allocatedClient, framework));
        return allocatedClient;
    }
    template <> ramses::Scene* CreationHelper::createObjectOfType<ramses::Scene>(std::string_view name)
    {
        return m_ramsesClient->createScene(sceneId_t(999u), name);
    }
    template <> LogicEngine* CreationHelper::createObjectOfType<LogicEngine>(std::string_view name)
    {
        return m_scene->createLogicEngine(name);
    }
    template <> Node* CreationHelper::createObjectOfType<Node>(std::string_view name)
    {
        return m_scene->createNode(name);
    }
    template <> MeshNode* CreationHelper::createObjectOfType<MeshNode>(std::string_view name)
    {
        return m_scene->createMeshNode(name);
    }
    template <> PerspectiveCamera* CreationHelper::createObjectOfType<PerspectiveCamera>(std::string_view name)
    {
        return m_scene->createPerspectiveCamera(name);
    }
    template <> OrthographicCamera* CreationHelper::createObjectOfType<OrthographicCamera>(std::string_view name)
    {
        return m_scene->createOrthographicCamera(name);
    }
    template <> Effect* CreationHelper::createObjectOfType<Effect>(std::string_view name)
    {
        EffectDescription effectDescription;
        effectDescription.setVertexShader("void main(void) {gl_Position=vec4(0);}");
        effectDescription.setFragmentShader("void main(void) {gl_FragColor=vec4(1);}");
        return m_scene->createEffect(effectDescription, name);
    }
    template <> Appearance* CreationHelper::createObjectOfType<Appearance>(std::string_view name)
    {
        return m_scene->createAppearance(*TestEffects::CreateTestEffect(*m_scene, "appearance effect"), name);
    }
    template <> Texture2D* CreationHelper::createObjectOfType<Texture2D>(std::string_view name)
    {
        std::vector<std::byte> data(4, std::byte{ 0u });
        const std::vector<MipLevelData> mipLevelData{ data };
        return m_scene->createTexture2D(ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {}, name);
    }
    template <> Texture3D* CreationHelper::createObjectOfType<Texture3D>(std::string_view name)
    {
        std::vector<std::byte> data(32, std::byte{ 0u });
        const std::vector<MipLevelData> mipLevelData{ data };
        return m_scene->createTexture3D(ETextureFormat::RGBA8, 1u, 2u, 4u, mipLevelData, false, name);
    }
    template <> TextureCube* CreationHelper::createObjectOfType<TextureCube>(std::string_view name)
    {
        std::vector<std::byte> data(4, std::byte{ 0u });
        std::vector<CubeMipLevelData> mipLevelData{ {data, data, data, data, data, data} };
        return m_scene->createTextureCube(ETextureFormat::RGBA8, 1u, mipLevelData, false, {}, name);
    }
    template <> ArrayResource* CreationHelper::createObjectOfType<ArrayResource>(std::string_view name)
    {
        const uint16_t data = 0u;
        return m_scene->createArrayResource(1u, &data, name);
    }

    template <> ramses::RenderGroup* CreationHelper::createObjectOfType<ramses::RenderGroup>(std::string_view name)
    {
        return m_scene->createRenderGroup(name);
    }
    template <> ramses::RenderPass* CreationHelper::createObjectOfType<ramses::RenderPass>(std::string_view name)
    {
        return m_scene->createRenderPass(name);
    }

    template <> ramses::BlitPass* CreationHelper::createObjectOfType<ramses::BlitPass>(std::string_view name)
    {
        const ramses::RenderBuffer* sourceRenderBuffer = createObjectOfType<ramses::RenderBuffer>("src render buffer");
        const ramses::RenderBuffer* destinationRenderBuffer = createObjectOfType<ramses::RenderBuffer>("dst render buffer");
        return m_scene->createBlitPass(*sourceRenderBuffer, *destinationRenderBuffer, name);
    }

    template <> ramses::TextureSampler* CreationHelper::createObjectOfType<ramses::TextureSampler>(std::string_view name)
    {
        std::vector<std::byte> data(4, std::byte{ 0u });
        const std::vector<MipLevelData> mipLevelData{ data };
        Texture2D* texture = m_scene->createTexture2D(ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {}, "texture");
        return m_scene->createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear, ETextureSamplingMethod::Nearest, *texture, 1u, name);
    }
    template <> TextureSamplerMS* CreationHelper::createObjectOfType<TextureSamplerMS>(std::string_view name)
    {
        ramses::RenderBuffer* renderBuffer = m_scene->createRenderBuffer(16, 16, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 4u, "renderBuffer");
        return m_scene->createTextureSamplerMS(*renderBuffer, name);
    }
    template <> TextureSamplerExternal* CreationHelper::createObjectOfType<TextureSamplerExternal>(std::string_view name)
    {
        return m_scene->createTextureSamplerExternal(ETextureSamplingMethod::Linear, ETextureSamplingMethod::Linear, name);
    }
    template <> ramses::RenderBuffer* CreationHelper::createObjectOfType<ramses::RenderBuffer>(std::string_view name)
    {
        return m_scene->createRenderBuffer(16, 16, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 0u, name);
    }
    template <> RenderTarget* CreationHelper::createObjectOfType<RenderTarget>(std::string_view name)
    {
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*createObjectOfType<ramses::RenderBuffer>("rb"));
        return m_scene->createRenderTarget(rtDesc, name);
    }

    template <> Geometry* CreationHelper::createObjectOfType<Geometry>(std::string_view name)
    {
        return m_scene->createGeometry(*createObjectOfType<Effect>("geometry_binding_effect"), name);
    }

    template <> DataObject* CreationHelper::createObjectOfType<DataObject>(std::string_view name)
    {
        return m_scene->createDataObject(ramses::EDataType::Float, name);
    }

    template <> ArrayBuffer* CreationHelper::createObjectOfType<ArrayBuffer>(std::string_view name)
    {
        return m_scene->createArrayBuffer(ramses::EDataType::UInt32, 13u, name);

    }
    template <> Texture2DBuffer* CreationHelper::createObjectOfType<Texture2DBuffer>(std::string_view name)
    {
        return m_scene->createTexture2DBuffer(ETextureFormat::RGBA8, 3, 4, 2, name);
    }

    template <> ramses::PickableObject* CreationHelper::createObjectOfType<ramses::PickableObject>(std::string_view name)
    {
        const auto vb = m_scene->createArrayBuffer(ramses::EDataType::Vector3F, 3u, "vb");
        m_additionalAllocatedSceneObjects.push_back(vb);
        PerspectiveCamera* camera =  m_scene->createPerspectiveCamera("pickableCamera");
        m_additionalAllocatedSceneObjects.push_back(camera);
        camera->setFrustum(-1.4f, 1.4f, -1.4f, 1.4f, 1.f, 100.f);
        camera->setViewport(0, 0, 200, 200);
        ramses::PickableObject* pickableObject =  m_scene->createPickableObject(*vb, pickableObjectId_t{ 123u }, name);
        pickableObject->setCamera(*camera);
        return pickableObject;
    }

    template <> ramses::SceneReference* CreationHelper::createObjectOfType<ramses::SceneReference>(std::string_view name)
    {
        // SceneReference cannot refer to same sceneID, create a different one every time
        ++m_lastReferencedSceneId.getReference();
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        return m_scene->createSceneReference(m_lastReferencedSceneId, name);
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
    }
}
