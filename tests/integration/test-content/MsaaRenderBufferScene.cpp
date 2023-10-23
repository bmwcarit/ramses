//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MsaaRenderBufferScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/EffectDescription.h"
#include <cassert>

namespace ramses::internal
{
    MsaaRenderBufferScene::MsaaRenderBufferScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : CommonRenderBufferTestScene(scene, cameraPosition)
        , m_colorBufferMsaa1(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::WriteOnly, 1u))
        , m_colorBufferMsaa2(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::WriteOnly, 2u))
        , m_colorBufferMsaa4(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::WriteOnly, 4u))
        , m_colorBufferMsaa1ReadWrite(scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite, 1u))
        , m_colorBufferMsaa4ReadWrite(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite, 4u))
        , m_blittingColorBuffer(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite))
    {
        // only checks that can be created but not used in test
        // GL implementations vary in what actual sample count to use with 1 sample specified
        assert(m_colorBufferMsaa1ReadWrite);

        initRenderPass(state);
        if (state != SAMPLE_COUNT_4_TEXEL_FETCH)
        {
            initBlittingPass(state);
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_blittingColorBuffer));
        }
        else
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTextureMS(m_colorBufferMsaa4ReadWrite));
    }

    ramses::RenderTarget& MsaaRenderBufferScene::createRenderTarget(uint32_t state)
    {
        ramses::RenderTargetDescription rtDesc;

        switch (state)
        {
        case SAMPLE_COUNT_1_BLIT:
            rtDesc.addRenderBuffer(m_colorBufferMsaa1);
            break;
        case SAMPLE_COUNT_2_BLIT:
            rtDesc.addRenderBuffer(m_colorBufferMsaa2);
            break;
        case SAMPLE_COUNT_4_BLIT:
            rtDesc.addRenderBuffer(m_colorBufferMsaa4);
            break;
        case SAMPLE_COUNT_4_TEXEL_FETCH:
            rtDesc.addRenderBuffer(m_colorBufferMsaa4ReadWrite);
            break;
        default:
            assert(false);
            break;
        }

        return *m_scene.createRenderTarget(rtDesc);
    }

    void MsaaRenderBufferScene::initRenderPass(uint32_t state)
    {
        ramses::MeshNode& meshNode = createMesh();

        ramses::Node& transNode = *m_scene.createNode();
        transNode.addChild(meshNode);
        transNode.translate({0.0f, -0.5f, -5.0f});

        ramses::RenderGroup& renderGroup = *m_scene.createRenderGroup();
        renderGroup.addMeshNode(meshNode);

        ramses::RenderPass& renderPass = *m_scene.createRenderPass();
        renderPass.setRenderOrder(0);
        renderPass.addRenderGroup(renderGroup);

        ramses::PerspectiveCamera& camera = createCamera();
        camera.setViewport(0u, 0u, 2u, 2u);
        renderPass.setCamera(camera);

        ramses::RenderTarget& renderTarget = createRenderTarget(state);
        renderPass.setRenderTarget(&renderTarget);
        renderPass.setClearColor({0.f, 0.f, 0.f, 1.0f});
        renderPass.setClearFlags(ramses::EClearFlag::All);
    }

    void MsaaRenderBufferScene::initBlittingPass(uint32_t state)
    {
        ramses::BlitPass* blitPass = nullptr;
        switch (state)
        {
        case SAMPLE_COUNT_1_BLIT:
            blitPass = m_scene.createBlitPass(m_colorBufferMsaa1, m_blittingColorBuffer);
            break;
        case SAMPLE_COUNT_2_BLIT:
            blitPass = m_scene.createBlitPass(m_colorBufferMsaa2, m_blittingColorBuffer);
            break;
        case SAMPLE_COUNT_4_BLIT:
            blitPass = m_scene.createBlitPass(m_colorBufferMsaa4, m_blittingColorBuffer);
            break;
        default:
            assert(false);
            break;
        }

        assert(nullptr != blitPass);

        blitPass->setRenderOrder(1);
    }

    ramses::MeshNode& MsaaRenderBufferScene::createMesh()
    {
        const ramses::Effect& effect = getEffectRenderOneBuffer();
        ramses::MeshNode& meshNode = CommonRenderBufferTestScene::createMesh(effect, TriangleAppearance::EColor_White);

        const std::array<ramses::vec3f, 3u> vertexPositionsData{
            ramses::vec3f{ -1.f, -1.f, 0.f },
            ramses::vec3f{ 1.f, -1.f, 0.f },
            ramses::vec3f{ -1.f, 1.f, 0.f } };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(3u, vertexPositionsData.data());
        meshNode.getGeometry()->setInputBuffer(*effect.findAttributeInput("a_position"), *vertexPositions);

        return meshNode;
    }

    const ramses::MeshNode& MsaaRenderBufferScene::createQuadWithTextureMS(const ramses::RenderBuffer& renderBuffer)
    {
        const ramses::Effect& effect = *getTestEffect("ramses-test-client-render-one-buffer-ms");

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(6u, indicesArray);

        const std::array<ramses::vec3f, 4u> vertexPositionsArray
        {
            ramses::vec3f{ -1.f, -1.f, 0.f },
            ramses::vec3f{ 1.f, -1.f, 0.f },
            ramses::vec3f{ -1.f, 1.f, 0.f },
            ramses::vec3f{ 1.f, 1.f, 0.f }
        };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f}, ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f} };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        ramses::Appearance* appearance = m_scene.createAppearance(effect, "appearance");

        // set vertex positions directly in geometry
        ramses::Geometry* geometry = m_scene.createGeometry(effect, "quad geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(*effect.findAttributeInput("a_position"), *vertexPositions);
        geometry->setInputBuffer(*effect.findAttributeInput("a_texcoord"), *textureCoords);

        ramses::TextureSamplerMS* sampler = m_scene.createTextureSamplerMS(renderBuffer, "MSAA sampler");
        appearance->setInputTexture(*effect.findUniformInput("textureSampler"), *sampler);
        appearance->setInputValue(*effect.findUniformInput("sampleCount"), 4);

        ramses::MeshNode* meshNode = m_scene.createMeshNode("quad");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({0.0f, 0.f, -8.f});
        meshNode->setParent(*transNode);

        return *meshNode;
    }
}
