//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/RenderPassOnceScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RenderTargetDescription.h"

namespace ramses_internal
{
    RenderPassOnceScene::RenderPassOnceScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_camera(*scene.createOrthographicCamera())
        , m_renderPass(*m_scene.createRenderPass())
        , m_renderBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite))
    {
        m_camera.setFrustum(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
        m_camera.setViewport(0u, 0u, 16u, 16u);
        m_camera.setParent(getDefaultCameraTranslationNode());
        m_renderPass.setCamera(m_camera);

        initInputRenderPass();
        initFinalRenderPass();

        setState(state);
    }

    void RenderPassOnceScene::setState(UInt32 state)
    {
        switch (state)
        {
        case INITIAL_RENDER_ONCE:
            m_renderPass.setClearColor(1.f, 0.f, 1.f, 1.f);
            m_renderPass.setRenderOnce(true);
            break;
        case CHANGE_CLEAR_COLOR:
            m_renderPass.setClearColor(1.f, 1.f, 0.f, 1.f);
            break;
        case RETRIGGER_PASS:
            m_renderPass.retriggerRenderOnce();
            break;
        default:
            assert(false);
        }
    }

    void RenderPassOnceScene::initInputRenderPass()
    {
        ramses::MeshNode* meshNode = m_scene.createMeshNode();
        ramses::Triangle blueTriangle(m_client, m_scene, *getTestEffect("ramses-test-client-basic"), ramses::TriangleAppearance::EColor_Blue);
        meshNode->setAppearance(blueTriangle.GetAppearance());
        meshNode->setGeometryBinding(blueTriangle.GetGeometry());

        ramses::Node* translateNode = m_scene.createNode();
        translateNode->addChild(*meshNode);
        translateNode->translate(0.0f, -0.5f, -5.0f);

        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        m_renderPass.addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(*meshNode);

        ramses::RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(m_renderBuffer);
        ramses::RenderTarget* renderTarget = m_scene.createRenderTarget(rtDesc);
        m_renderPass.setRenderTarget(renderTarget);
        m_renderPass.setClearFlags(ramses::EClearFlags::EClearFlags_All);
        m_renderPass.setRenderOrder(-1);
    }

    void RenderPassOnceScene::initFinalRenderPass()
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

        const float vertexPositionsArray[] = {
            -0.5f, -0.5f, 0.f,
            0.5f, -0.5f, 0.f,
            -0.5f, 0.5f, 0.f,
            0.5f, 0.5f, 0.f };
        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        const float textureCoordsArray[] = { 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f };
        const ramses::Vector2fArray* textureCoords = m_client.createConstVector2fArray(4, textureCoordsArray);

        ramses::Appearance* appearance = m_scene.createAppearance(*effect, "appearance");

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
            m_renderBuffer);

        ramses::UniformInput textureInput;
        effect->findUniformInput("u_texture", textureInput);
        appearance->setInputTexture(textureInput, *sampler);

        ramses::MeshNode* meshNode = m_scene.createMeshNode("quad");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation(0.f, 0.f, -4.f);
        meshNode->setParent(*transNode);

        ramses::Camera *camera = m_scene.createRemoteCamera();
        camera->setParent(getDefaultCameraTranslationNode());
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderPass->setRenderOrder(1);
        renderGroup->addMeshNode(*meshNode);
    }
}
