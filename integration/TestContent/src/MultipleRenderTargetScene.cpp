//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultipleRenderTargetScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "Math3d/Vector3.h"

namespace ramses_internal
{
    MultipleRenderTargetScene::MultipleRenderTargetScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : CommonRenderBufferTestScene(ramsesClient, scene, cameraPosition)
        , m_renderBuffer1(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite))
        , m_renderBuffer2(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite))
        , m_depthBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Depth, ramses::ERenderBufferFormat_Depth24, ramses::ERenderBufferAccessMode_ReadWrite))
    {
        initClearPass();

        if (state != CLEAR_MRT)
        {
            initMRTPass(state);
        }

        initFinalRenderPass(state);
    }

    const ramses::Effect& MultipleRenderTargetScene::getMRTEffect(UInt32 state)
    {
        switch (state)
        {
        case TWO_COLOR_BUFFERS:
        case SHADER_WRITES_TWO_COLOR_BUFFERS_RT_HAS_ONE:
            return getEffectRenderTwoBuffers();
        case SHADER_WRITES_ONE_COLOR_BUFFER_RT_HAS_TWO:
        case COLOR_WRITTEN_BY_TWO_DIFFERENT_RTS:
        case DEPTH_WRITTEN_AND_USED_BY_DIFFERENT_RT:
        case DEPTH_WRITTEN_AND_READ:
            return getEffectRenderOneBuffer();
        default:
            assert(false);
            return *getTestEffect("dummy");
        }
    }

    ramses::RenderTarget& MultipleRenderTargetScene::createMRTRenderTarget(UInt32 state)
    {
        ramses::RenderTargetDescription rtDesc;

        switch (state)
        {
        case TWO_COLOR_BUFFERS:
        case SHADER_WRITES_ONE_COLOR_BUFFER_RT_HAS_TWO:
            rtDesc.addRenderBuffer(m_renderBuffer1);
            rtDesc.addRenderBuffer(m_renderBuffer2);
            break;
        case SHADER_WRITES_TWO_COLOR_BUFFERS_RT_HAS_ONE:
        case COLOR_WRITTEN_BY_TWO_DIFFERENT_RTS:
            rtDesc.addRenderBuffer(m_renderBuffer1);
            break;
        case DEPTH_WRITTEN_AND_USED_BY_DIFFERENT_RT:
            rtDesc.addRenderBuffer(m_renderBuffer1);
            rtDesc.addRenderBuffer(m_depthBuffer);
            break;
        case DEPTH_WRITTEN_AND_READ:
            rtDesc.addRenderBuffer(m_depthBuffer);
            break;
        default:
            assert(false);
            break;
        }

        return *m_scene.createRenderTarget(rtDesc);
    }

    const ramses::MeshNode& MultipleRenderTargetScene::createQuadWithTexture(const ramses::RenderBuffer& renderBuffer, const Vector3& translation, const Vector4& modulateColor)
    {
        const ramses::Effect* effect = getTestEffect("ramses-test-client-texturedWithColor");

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

        const float vertexPositionsArray[] =
        {
            -0.5f, -0.5f, 0.f,
            0.5f, -0.5f, 0.f,
            -0.5f, 0.5f, 0.f,
            0.5f, 0.5f, 0.f
        };
        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        const float textureCoordsArray[] = { 0.f, 0.f, 2.f, 0.f, 0.f, 2.f, 2.f, 2.f };
        const ramses::Vector2fArray* textureCoords = m_client.createConstVector2fArray(4, textureCoordsArray);

        ramses::Appearance* appearance = m_scene.createAppearance(*effect, "appearance");
        ramses::UniformInput colorInput;
        effect->findUniformInput("u_color", colorInput);
        appearance->setInputValueVector4f(colorInput, modulateColor.x, modulateColor.y, modulateColor.z, modulateColor.w);

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texCoordsInput;
        effect->findAttributeInput("a_position", positionsInput);
        effect->findAttributeInput("a_texcoord", texCoordsInput);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*effect, "triangle geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(texCoordsInput, *textureCoords);

        ramses::TextureSampler* sampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest,
            ramses::ETextureSamplingMethod_Nearest,
            renderBuffer);

        ramses::UniformInput textureInput;
        effect->findUniformInput("u_texture", textureInput);
        appearance->setInputTexture(textureInput, *sampler);

        ramses::MeshNode* meshNode = m_scene.createMeshNode("quad");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation(translation.x, translation.y, translation.z);
        meshNode->setParent(*transNode);

        return *meshNode;
    }

    void MultipleRenderTargetScene::initClearPass()
    {
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setRenderOrder(-100);
        renderPass->setCamera(createCamera());

        ramses::RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(m_renderBuffer1);
        rtDesc.addRenderBuffer(m_renderBuffer2);
        rtDesc.addRenderBuffer(m_depthBuffer);
        ramses::RenderTarget& renderTarget = *m_scene.createRenderTarget(rtDesc);

        renderPass->setRenderTarget(&renderTarget);
        renderPass->setClearColor(1.f, 0.f, 1.f, 0.5f);
        renderPass->setClearFlags(ramses::EClearFlags::EClearFlags_All);
    }

    void MultipleRenderTargetScene::initMRTPass(UInt32 state)
    {
        ramses::MeshNode& meshNode = createMesh(getMRTEffect(state));

        ramses::Node& transNode = *m_scene.createNode();
        transNode.addChild(meshNode);
        transNode.translate(0.0f, -0.5f, -5.0f);
        if (state == DEPTH_WRITTEN_AND_READ)
        {
            transNode.rotate(30.0f, 0.f, 0.f);
        }

        ramses::RenderGroup& renderGroup = *m_scene.createRenderGroup();
        renderGroup.addMeshNode(meshNode);

        ramses::RenderPass& renderPass = *m_scene.createRenderPass();
        renderPass.setRenderOrder(0);
        renderPass.addRenderGroup(renderGroup);

        ramses::PerspectiveCamera& camera = createCamera(5.8f, 6.5f);
        renderPass.setCamera(camera);

        ramses::RenderTarget& renderTarget = createMRTRenderTarget(state);
        renderPass.setRenderTarget(&renderTarget);
        renderPass.setClearFlags(ramses::EClearFlags::EClearFlags_None);

        if (state == COLOR_WRITTEN_BY_TWO_DIFFERENT_RTS ||
            state == DEPTH_WRITTEN_AND_USED_BY_DIFFERENT_RT)
        {
            camera.setViewport(0u, 0u, 8u, 16u);
            camera.setFrustum(camera.getLeftPlane() * 0.5f, camera.getRightPlane() * 0.5f, camera.getBottomPlane(), camera.getTopPlane(), camera.getNearPlane(), camera.getFarPlane());

            ramses::MeshNode& meshNode2 = createMesh(getMRTEffect(state), ramses::TriangleAppearance::EColor_Blue);
            transNode.addChild(meshNode2);

            meshNode2.getAppearance()->setDepthFunction(ramses::EDepthFunc_NotEqual);

            ramses::RenderGroup& renderGroup2 = *m_scene.createRenderGroup();
            renderGroup2.addMeshNode(meshNode2);

            ramses::RenderPass& renderPass2 = *m_scene.createRenderPass();
            renderPass2.setRenderOrder(1);
            renderPass2.addRenderGroup(renderGroup2);

            ramses::PerspectiveCamera& camera2 = createCamera(5.8f, 6.5f);
            camera2.setViewport(6u, 0u, 8u, 16u);
            camera2.setFrustum(camera2.getLeftPlane() * 0.5f, camera2.getRightPlane() * 0.5f, camera2.getBottomPlane(), camera2.getTopPlane(), camera2.getNearPlane(), camera2.getFarPlane());
            renderPass2.setCamera(camera2);

            ramses::RenderTarget& renderTarget2 = createMRTRenderTarget(state);
            renderPass2.setRenderTarget(&renderTarget2);
            renderPass2.setClearFlags(ramses::EClearFlags::EClearFlags_None);
        }
    }

    void MultipleRenderTargetScene::initFinalRenderPass(UInt32 state)
    {
        const ramses::MeshNode* quad1 = NULL;
        const ramses::MeshNode* quad2 = NULL;

        if (state != DEPTH_WRITTEN_AND_READ)
        {
            quad1 = &createQuadWithTexture(m_renderBuffer1, Vector3(-0.6f, 0.f, -8.f));
            quad2 = &createQuadWithTexture(m_renderBuffer2, Vector3(0.6f, 0.f, -8.f));
        }
        else
        {
            // Modulate depth sampled as color with red color only, so that it can be tested consistently on different platforms.
            // Some platforms give the depth value in all RGBA channels, some only in R channel.
            quad1 = &createQuadWithTexture(m_depthBuffer, Vector3(-0.6f, 0.f, -8.f), Vector4(1.f, 0.f, 0.f, 1.f));
            quad2 = &createQuadWithTexture(m_depthBuffer, Vector3(0.6f, 0.f, -8.f), Vector4(1.f, 0.f, 0.f, 1.f));
        }

        ramses::Camera *camera = m_scene.createRemoteCamera();
        camera->setParent(getDefaultCameraTranslationNode());
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setRenderOrder(100);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(*quad1);
        renderGroup->addMeshNode(*quad2);
    }
}
