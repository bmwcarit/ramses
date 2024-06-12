//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/Scene.h"
#include "ramses/client/Effect.h"
#include "ramses/client/PerspectiveCamera.h"
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
#include "ramses/client/MeshNode.h"
#include "TestScenes/RenderPassClearScene.h"

namespace ramses::internal
{
    RenderPassClearScene::RenderPassClearScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(*getTestEffect("ramses-test-client-basic"))
        , m_blueTriangle(scene, m_effect, TriangleAppearance::EColor::Blue)
        , m_redTriangle(scene, m_effect, TriangleAppearance::EColor::Red)
        , m_greenTriangle(scene, m_effect, TriangleAppearance::EColor::Green)
        , m_colorBuffer(*m_scene.createRenderBuffer(200u, 200u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite))
        , m_depthStencilBuffer(*m_scene.createRenderBuffer(200u, 200u, ramses::ERenderBufferFormat::Depth24_Stencil8, ramses::ERenderBufferAccessMode::ReadWrite))
    {
        ramses::RenderTarget& renderTarget = createRenderTarget();
        const ramses::PerspectiveCamera& camera = createCamera();

        // Draw blue triangle in front in a separate render pass.
        {
            ramses::RenderGroup& blueRenderGroup = *m_scene.createRenderGroup();
            ramses::RenderPass& blueRenderPass = *m_scene.createRenderPass();

            m_blueTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc::LessEqual);
            m_blueTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Always, 0, 0xff);
            m_blueTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment);

            addTriangleMesh(m_blueTriangle, blueRenderGroup, -0.5f, -0.1f, -2.3f);

            blueRenderPass.setRenderOrder(0);
            blueRenderPass.setClearColor({1.f, 0.f, 1.f, 1.f});
            blueRenderPass.setClearFlags(ramses::EClearFlag::All);
            blueRenderPass.addRenderGroup(blueRenderGroup);
            blueRenderPass.setCamera(camera);
            blueRenderPass.setRenderTarget(&renderTarget);
        }

        // Draw red triangle behind, using different combinations of clear flags in a new render pass.
        {
            ramses::RenderGroup& redRenderGroup = *m_scene.createRenderGroup();
            ramses::RenderPass& redRenderPass = *m_scene.createRenderPass();

            m_redTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc::LessEqual);
            m_redTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Always, 0, 0xff);
            m_redTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment, ramses::EStencilOperation::Increment);
            addTriangleMesh(m_redTriangle, redRenderGroup, 0.5f, 0.1f, -3.0f);

            redRenderPass.setRenderOrder(1);
            redRenderPass.setClearColor({0.f, 0.f, 0.f, 1.f});
            redRenderPass.setClearFlags(ClearFlags(state)); // state will contain all combinations of clear flags tested
            redRenderPass.addRenderGroup(redRenderGroup);
            redRenderPass.setCamera(camera);
            redRenderPass.setRenderTarget(&renderTarget);
        }

        // Draw a final green triangle on top, which will only be visible where the stencil buffer is still 0.
        {
            ramses::RenderGroup& greenRenderGroup = *m_scene.createRenderGroup();
            ramses::RenderPass& greenRenderPass = *m_scene.createRenderPass();

            m_greenTriangle.GetAppearance().setDepthFunction(ramses::EDepthFunc::LessEqual);
            m_greenTriangle.GetAppearance().setStencilFunction(ramses::EStencilFunc::Equal, 0, 0xff);
            m_greenTriangle.GetAppearance().setStencilOperation(ramses::EStencilOperation::Keep, ramses::EStencilOperation::Keep, ramses::EStencilOperation::Keep);
            addTriangleMesh(m_greenTriangle, greenRenderGroup, 0.0f, 0.0f, -2.2f);

            greenRenderPass.setRenderOrder(2);
            greenRenderPass.setClearFlags(ramses::EClearFlag::None);
            greenRenderPass.addRenderGroup(greenRenderGroup);
            greenRenderPass.setCamera(camera);
            greenRenderPass.setRenderTarget(&renderTarget);
        }

        // Visualize the color buffer
        renderBufferToScreen(m_colorBuffer);
    }

    ramses::RenderTarget& RenderPassClearScene::createRenderTarget()
    {
        ramses::RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(m_colorBuffer);
        rtDesc.addRenderBuffer(m_depthStencilBuffer);
        return *m_scene.createRenderTarget(rtDesc);
    }

    void RenderPassClearScene::renderBufferToScreen(ramses::RenderBuffer& renderBuffer)
    {
        const ramses::MeshNode& quad = createQuadWithTexture(renderBuffer);

        ramses::RenderPass& renderPass = *m_scene.createRenderPass();
        renderPass.setRenderOrder(100);

        renderPass.setCamera(createCamera());

        ramses::RenderGroup& renderGroup = *m_scene.createRenderGroup();
        renderPass.addRenderGroup(renderGroup);
        renderGroup.addMeshNode(quad);
    }

    void RenderPassClearScene::addTriangleMesh(Triangle& triangle, ramses::RenderGroup& targetGroup, float x, float y, float z)
    {
        ramses::MeshNode& meshNode = *m_scene.createMeshNode();
        meshNode.setAppearance(triangle.GetAppearance());
        meshNode.setGeometry(triangle.GetGeometry());

        ramses::Node* translateNode = m_scene.createNode();
        translateNode->addChild(meshNode);
        translateNode->translate({x, y, z});

        targetGroup.addMeshNode(meshNode);
    }

    const ramses::PerspectiveCamera& RenderPassClearScene::createCamera()
    {
        ramses::PerspectiveCamera& camera = *m_scene.createPerspectiveCamera();
        camera.setFrustum(20.f, 1.f, 0.1f, 50.0f);
        camera.setViewport(0u, 0u, 200u, 200u);

        ramses::Node& posNode = *m_scene.createNode();
        posNode.addChild(camera);
        posNode.translate({0.f, 0.f, 5.0f});

        camera.setParent(posNode);

        return camera;
    }

    const ramses::MeshNode& RenderPassClearScene::createQuadWithTexture(const ramses::RenderBuffer& renderBuffer)
    {
        const ramses::Effect* effect = getTestEffect("ramses-test-client-textured");

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
            renderBuffer);

        appearance->setInputTexture(*effect->findUniformInput("u_texture"), *sampler);

        ramses::MeshNode* meshNode = m_scene.createMeshNode("quad");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({0.f, 0.f, -2.5f});
        meshNode->setParent(*transNode);

        return *meshNode;
    }
}
