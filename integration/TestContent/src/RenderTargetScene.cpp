//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/RenderTargetScene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
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
    RenderTargetScene::RenderTargetScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_renderBuffer(createRenderBuffer(state))
    {
        initInputRenderPass(state);
        initFinalRenderPass();
    }

    const ramses::RenderBuffer& RenderTargetScene::createRenderBuffer(UInt32 state)
    {
        ramses::ERenderBufferFormat bufferFormat = ramses::ERenderBufferFormat_RGBA8;

        switch (state)
        {
        case PERSPECTIVE_PROJECTION:
        case ORTHOGRAPHIC_PROJECTION:
            bufferFormat = ramses::ERenderBufferFormat_RGBA8;
            break;
        case RENDERBUFFER_FORMAT_R8:
            bufferFormat = ramses::ERenderBufferFormat_R8;
            break;
        case RENDERBUFFER_FORMAT_RG8:
            bufferFormat = ramses::ERenderBufferFormat_RG8;
            break;
        case RENDERBUFFER_FORMAT_RGB8:
            bufferFormat = ramses::ERenderBufferFormat_RGB8;
            break;
        case RENDERBUFFER_FORMAT_R16F:
            bufferFormat = ramses::ERenderBufferFormat_R16F;
            break;
        case RENDERBUFFER_FORMAT_R32F:
            bufferFormat = ramses::ERenderBufferFormat_R32F;
            break;
        case RENDERBUFFER_FORMAT_RG16F:
            bufferFormat = ramses::ERenderBufferFormat_RG16F;
            break;
        case RENDERBUFFER_FORMAT_RG32F:
            bufferFormat = ramses::ERenderBufferFormat_RG32F;
            break;
        case RENDERBUFFER_FORMAT_RGB16F:
            bufferFormat = ramses::ERenderBufferFormat_RGB16F;
            break;
        case RENDERBUFFER_FORMAT_RGB32F:
            bufferFormat = ramses::ERenderBufferFormat_RGB32F;
            break;
        case RENDERBUFFER_FORMAT_RGBA16F:
            bufferFormat = ramses::ERenderBufferFormat_RGBA16F;
            break;
        case RENDERBUFFER_FORMAT_RGBA32F:
            bufferFormat = ramses::ERenderBufferFormat_RGBA32F;
            break;

        default:
            assert(false);
            break;
        }

        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Color, bufferFormat, ramses::ERenderBufferAccessMode_ReadWrite);
        assert(renderBuffer != NULL);
        return *renderBuffer;
    }

    ramses::Camera* RenderTargetScene::createCamera(UInt32 state)
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
            return 0;
        }
    }

    void RenderTargetScene::initInputRenderPass(UInt32 state)
    {
        ramses::MeshNode* meshNode = m_scene.createMeshNode();
        if (state == PERSPECTIVE_PROJECTION || state == ORTHOGRAPHIC_PROJECTION)
        {
            ramses::Triangle blueTriangle(m_client, m_scene, *getTestEffect("ramses-test-client-basic"), ramses::TriangleAppearance::EColor_Blue);
            meshNode->setAppearance(blueTriangle.GetAppearance());
            meshNode->setGeometryBinding(blueTriangle.GetGeometry());
        }
        else
        {
            ramses::Triangle greyTriangle(m_client, m_scene, *getTestEffect("ramses-test-client-basic"), ramses::TriangleAppearance::EColor_Grey);
            meshNode->setAppearance(greyTriangle.GetAppearance());
            meshNode->setGeometryBinding(greyTriangle.GetGeometry());
        }

        ramses::Node* translateNode = m_scene.createNode();
        translateNode->addChild(*meshNode);
        translateNode->translate(0.0f, -0.5f, -5.0f);

        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        ramses::RenderGroup* renderGroup = m_scene.createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(*meshNode);

        renderPass->setCamera(*createCamera(state));

        ramses::RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(m_renderBuffer);
        ramses::RenderTarget* renderTarget = m_scene.createRenderTarget(rtDesc);
        renderPass->setRenderTarget(renderTarget);
        renderPass->setClearColor(1.f, 0.f, 1.f, 0.5f);
        renderPass->setClearFlags(ramses::EClearFlags::EClearFlags_All);
    }

    void RenderTargetScene::initFinalRenderPass()
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-textured");

        uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(6, indicesArray);

        float vertexPositionsArray[] = {
            -0.5f, -0.5f, 0.f,
            0.5f, -0.5f, 0.f,
            -0.5f, 0.5f, 0.f,
            0.5f, 0.5f, 0.f };
        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(4, vertexPositionsArray);

        float textureCoordsArray[] = { 0.f, 0.f, 2.f, 0.f, 0.f, 2.f, 2.f, 2.f };
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
        renderGroup->addMeshNode(*meshNode);
    }
}
