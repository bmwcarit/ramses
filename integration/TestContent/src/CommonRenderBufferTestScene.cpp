//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/CommonRenderBufferTestScene.h"
#include "TestScenes/Triangle.h"
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
#include "Math3d/Vector3.h"

namespace ramses_internal
{
    CommonRenderBufferTestScene::CommonRenderBufferTestScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
    }

    const ramses::Effect& CommonRenderBufferTestScene::getEffectRenderOneBuffer()
    {
        return *getTestEffect("ramses-test-client-render-one-buffer");
    }

    const ramses::Effect& CommonRenderBufferTestScene::getEffectRenderTwoBuffers()
    {
        return *getTestEffect("ramses-test-client-render-two-buffers");
    }

    ramses::PerspectiveCamera& CommonRenderBufferTestScene::createCamera(Float nearPlane, Float farPlane)
    {
        ramses::PerspectiveCamera& camera = *m_scene.createPerspectiveCamera();
        camera.setFrustum(25.f, 1.0f, nearPlane, farPlane);
        camera.setViewport(0u, 0u, 16u, 16u);
        camera.setParent(getDefaultCameraTranslationNode());

        return camera;
    }

    const ramses::MeshNode& CommonRenderBufferTestScene::createQuadWithTexture(const ramses::RenderBuffer& renderBuffer, const Vector3& translation)
    {
        const ramses::Effect* effect = getTestEffect("ramses-test-client-textured");

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

        const float vertexPositionsArray[] =
        {
            -1.f, -1.f, 0.f,
            1.f, -1.f, 0.f,
            -1.f, 1.f, 0.f,
            1.f, 1.f, 0.f
        };
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

    ramses::MeshNode& CommonRenderBufferTestScene::createMesh(const ramses::Effect& effect, ramses::TriangleAppearance::EColor color)
    {
        ramses::Triangle triangle(m_client, m_scene, effect, color);

        ramses::MeshNode& meshNode = *m_scene.createMeshNode();
        meshNode.setAppearance(triangle.GetAppearance());
        meshNode.setGeometryBinding(triangle.GetGeometry());

        return meshNode;
    }

    ramses::RenderPass* CommonRenderBufferTestScene::addRenderPassUsingRenderBufferAsQuadTexture(ramses::RenderBuffer& sourceRenderBuffer)
    {
        const ramses::MeshNode& quad = createQuadWithTexture(sourceRenderBuffer, Vector3(0.0f, 0.f, -8.f));

        ramses::Camera *camera = m_scene.createRemoteCamera();
        camera->setParent(getDefaultCameraTranslationNode());
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setRenderOrder(100);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(quad);

        return renderPass;
    }
}
