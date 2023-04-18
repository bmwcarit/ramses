//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MsaaRenderBufferScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/EffectDescription.h"
#include "Math3d/Vector3.h"
#include <cassert>

namespace ramses_internal
{
    MsaaRenderBufferScene::MsaaRenderBufferScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : CommonRenderBufferTestScene(scene, cameraPosition)
        , m_colorBufferMsaa2(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_WriteOnly, 2u))
        , m_colorBufferMsaa4(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_WriteOnly, 4u))
        , m_colorBufferMsaa4ReadWrite(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite, 4u))
        , m_blittingColorBuffer(*scene.createRenderBuffer(2u, 2u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite))
    {
        initRenderPass(state);
        if (state != SAMPLE_COUNT_4_TEXEL_FETCH)
        {
            initBlittingPass(state);
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_blittingColorBuffer));
        }
        else
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTextureMS(m_colorBufferMsaa4ReadWrite));
    }

    ramses::RenderTarget& MsaaRenderBufferScene::createRenderTarget(UInt32 state)
    {
        ramses::RenderTargetDescription rtDesc;

        switch (state)
        {
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

    void MsaaRenderBufferScene::initRenderPass(UInt32 state)
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
        renderPass.setClearFlags(ramses::EClearFlags_All);
    }

    void MsaaRenderBufferScene::initBlittingPass(UInt32 state)
    {
        ramses::BlitPass* blitPass = nullptr;
        switch (state)
        {
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
        ramses::MeshNode& meshNode = CommonRenderBufferTestScene::createMesh(effect, ramses::TriangleAppearance::EColor_White);

        ramses::AttributeInput positionsInput;
        effect.findAttributeInput("a_position", positionsInput);

        const std::array<ramses::vec3f, 3u> vertexPositionsData{
            ramses::vec3f{ -1.f, -1.f, 0.f },
            ramses::vec3f{ 1.f, -1.f, 0.f },
            ramses::vec3f{ -1.f, 1.f, 0.f } };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(3u, vertexPositionsData.data());
        meshNode.getGeometryBinding()->setInputBuffer(positionsInput, *vertexPositions);

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

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texCoordsInput;
        effect.findAttributeInput("a_position", positionsInput);
        effect.findAttributeInput("a_texcoord", texCoordsInput);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(effect, "quad geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *textureCoords);

        ramses::TextureSamplerMS* sampler = m_scene.createTextureSamplerMS(renderBuffer, "MSAA sampler");

        ramses::UniformInput textureInput;
        effect.findUniformInput("textureSampler", textureInput);
        appearance->setInputTexture(textureInput, *sampler);

        ramses::UniformInput sampleCountInput;
        effect.findUniformInput("sampleCount", sampleCountInput);
        appearance->setInputValue(sampleCountInput, 4);

        ramses::MeshNode* meshNode = m_scene.createMeshNode("quad");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({0.0f, 0.f, -8.f});
        meshNode->setParent(*transNode);

        return *meshNode;
    }
}
