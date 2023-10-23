//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/RenderTargetScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/Effect.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderTargetDescription.h"
#include <cassert>

namespace ramses::internal
{
    RenderTargetScene::RenderTargetScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_renderBuffer(createRenderBuffer(state))
    {
        initInputRenderPass(state);
        initFinalRenderPass();
    }

    const ramses::RenderBuffer& RenderTargetScene::createRenderBuffer(uint32_t state)
    {
        ramses::ERenderBufferFormat bufferFormat = ramses::ERenderBufferFormat::RGBA8;

        switch (state)
        {
        case PERSPECTIVE_PROJECTION:
        case ORTHOGRAPHIC_PROJECTION:
            bufferFormat = ramses::ERenderBufferFormat::RGBA8;
            break;
        case RENDERBUFFER_FORMAT_RGBA4:
            bufferFormat = ramses::ERenderBufferFormat::RGBA4;
            break;
        case RENDERBUFFER_FORMAT_R8:
            bufferFormat = ramses::ERenderBufferFormat::R8;
            break;
        case RENDERBUFFER_FORMAT_RG8:
            bufferFormat = ramses::ERenderBufferFormat::RG8;
            break;
        case RENDERBUFFER_FORMAT_RGB8:
            bufferFormat = ramses::ERenderBufferFormat::RGB8;
            break;
        case RENDERBUFFER_FORMAT_R16F:
            bufferFormat = ramses::ERenderBufferFormat::R16F;
            break;
        case RENDERBUFFER_FORMAT_R32F:
            bufferFormat = ramses::ERenderBufferFormat::R32F;
            break;
        case RENDERBUFFER_FORMAT_RG16F:
            bufferFormat = ramses::ERenderBufferFormat::RG16F;
            break;
        case RENDERBUFFER_FORMAT_RG32F:
            bufferFormat = ramses::ERenderBufferFormat::RG32F;
            break;
        case RENDERBUFFER_FORMAT_RGB16F:
            bufferFormat = ramses::ERenderBufferFormat::RGB16F;
            break;
        case RENDERBUFFER_FORMAT_RGB32F:
            bufferFormat = ramses::ERenderBufferFormat::RGB32F;
            break;
        case RENDERBUFFER_FORMAT_RGBA16F:
            bufferFormat = ramses::ERenderBufferFormat::RGBA16F;
            break;
        case RENDERBUFFER_FORMAT_RGBA32F:
            bufferFormat = ramses::ERenderBufferFormat::RGBA32F;
            break;

        default:
            assert(false);
            break;
        }

        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(16u, 16u, bufferFormat, ramses::ERenderBufferAccessMode::ReadWrite);
        assert(renderBuffer != nullptr);
        return *renderBuffer;
    }

    ramses::Camera* RenderTargetScene::createCamera(uint32_t state)
    {
        switch (state)
        {
        case PERSPECTIVE_PROJECTION:
        {
            ramses::PerspectiveCamera* camera = m_scene.createPerspectiveCamera();
            camera->setFrustum(25.f, 16.f / 16.f, 0.1f, 1500.f);
            camera->setViewport(0, 0, 16, 16);
            camera->setParent(getDefaultCameraTranslationNode());
            return camera;
        }
        case ORTHOGRAPHIC_PROJECTION:
        case RENDERBUFFER_FORMAT_RGBA4:
        case RENDERBUFFER_FORMAT_R8:
        case RENDERBUFFER_FORMAT_RG8:
        case RENDERBUFFER_FORMAT_RGB8:
        case RENDERBUFFER_FORMAT_R16F:
        case RENDERBUFFER_FORMAT_R32F:
        case RENDERBUFFER_FORMAT_RG16F:
        case RENDERBUFFER_FORMAT_RG32F:
        case RENDERBUFFER_FORMAT_RGB16F:
        case RENDERBUFFER_FORMAT_RGB32F:
        case RENDERBUFFER_FORMAT_RGBA16F:
        case RENDERBUFFER_FORMAT_RGBA32F:
        {
            ramses::OrthographicCamera* camera = m_scene.createOrthographicCamera();
            camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
            camera->setViewport(0, 0, 16, 16);
            camera->setParent(getDefaultCameraTranslationNode());

            return camera;
        }
        default:
            assert(false);
            return nullptr;
        }
    }

    void RenderTargetScene::initInputRenderPass(uint32_t state)
    {
        ramses::MeshNode* meshNode = m_scene.createMeshNode();
        if (state == PERSPECTIVE_PROJECTION || state == ORTHOGRAPHIC_PROJECTION)
        {
            Triangle blueTriangle(m_scene, *getTestEffect("ramses-test-client-basic"), TriangleAppearance::EColor_Blue);
            meshNode->setAppearance(blueTriangle.GetAppearance());
            meshNode->setGeometry(blueTriangle.GetGeometry());
        }
        else
        {
            Triangle greyTriangle(m_scene, *getTestEffect("ramses-test-client-basic"), TriangleAppearance::EColor_Grey);
            meshNode->setAppearance(greyTriangle.GetAppearance());
            meshNode->setGeometry(greyTriangle.GetGeometry());
        }

        ramses::Node* translateNode = m_scene.createNode();
        translateNode->addChild(*meshNode);
        translateNode->translate({0.0f, -0.5f, -5.0f});

        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(*meshNode);

        renderPass->setCamera(*createCamera(state));

        ramses::RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(m_renderBuffer);
        ramses::RenderTarget* renderTarget = m_scene.createRenderTarget(rtDesc);
        renderPass->setRenderTarget(renderTarget);
        renderPass->setClearColor({1.f, 0.f, 1.f, 0.5f});
        renderPass->setClearFlags(ramses::EClearFlag::All);
    }

    void RenderTargetScene::initFinalRenderPass()
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");

        uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(6u, indicesArray);

        const std::array<ramses::vec3f, 4u> vertexPositionsArray{
            ramses::vec3f{ -0.5f, -0.5f, 0.f },
            ramses::vec3f{ 0.5f, -0.5f, 0.f },
            ramses::vec3f{ -0.5f, 0.5f, 0.f },
            ramses::vec3f{ 0.5f, 0.5f, 0.f } };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 0.f}, ramses::vec2f{2.f, 0.f}, ramses::vec2f{0.f, 2.f}, ramses::vec2f{2.f, 2.f} };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        ramses::Appearance* appearance = m_scene.createAppearance(*effect, "appearance");

        // set vertex positions directly in geometry
        ramses::Geometry* geometry = m_scene.createGeometry(*effect, "triangle geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *vertexPositions);
        geometry->setInputBuffer(*effect->findAttributeInput("a_texcoord"), *textureCoords);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Nearest,
            ramses::ETextureSamplingMethod::Nearest,
            m_renderBuffer);

        appearance->setInputTexture(*effect->findUniformInput("u_texture"), *sampler);

        ramses::MeshNode* meshNode = m_scene.createMeshNode("quad");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({0.f, 0.f, -4.f});
        meshNode->setParent(*transNode);

        ramses::Camera& camera = createCameraWithDefaultParameters();
        camera.setParent(getDefaultCameraTranslationNode());
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setCamera(camera);
        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(*meshNode);
    }
}
