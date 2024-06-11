//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/RenderBufferScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderTargetDescription.h"
#include <cassert>

namespace ramses::internal
{
    RenderBufferScene::RenderBufferScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : CommonRenderBufferTestScene(scene, cameraPosition)
        , m_readWriteColorRenderBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite))
        , m_writeOnlyDepthBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::Depth32, ramses::ERenderBufferAccessMode::WriteOnly))
        , m_writeOnlyDepthStencilBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::Depth24_Stencil8, ramses::ERenderBufferAccessMode::WriteOnly))
        , m_writeOnlyDepthBuffer16(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::Depth16, ramses::ERenderBufferAccessMode::WriteOnly))
        , m_writeOnlyDepthBuffer24(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::Depth24, ramses::ERenderBufferAccessMode::WriteOnly))
        , m_readWriteDepthBuffer16(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::Depth16, ramses::ERenderBufferAccessMode::ReadWrite))
        , m_readWriteDepthBuffer24(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::Depth24, ramses::ERenderBufferAccessMode::ReadWrite))
        , m_readWriteDepthBuffer32(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferFormat::Depth32, ramses::ERenderBufferAccessMode::ReadWrite))
    {
        initClearPass(state);
        initRenderingPass(state);

        switch (state)
        {
        case ONE_COLOR_BUFFER_NO_DEPTH_OR_STENCIL:
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_BUFFER:
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_STENCIL_BUFFER:
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH16:
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH24:
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_readWriteColorRenderBuffer));
            break;
        case READ_WRITE_DEPTH16:
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_readWriteDepthBuffer16));
            break;
        case READ_WRITE_DEPTH24:
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_readWriteDepthBuffer24));
            break;
        case READ_WRITE_DEPTH32:
            addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_readWriteDepthBuffer32));
            break;
        default:
            assert(false);
            break;
        }
    }

    ramses::RenderTarget& RenderBufferScene::createRenderTarget(uint32_t state)
    {
        ramses::RenderTargetDescription rtDesc;

        switch (state)
        {
        case ONE_COLOR_BUFFER_NO_DEPTH_OR_STENCIL:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            break;
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_BUFFER:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            rtDesc.addRenderBuffer(m_writeOnlyDepthBuffer);
            break;
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_STENCIL_BUFFER:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            rtDesc.addRenderBuffer(m_writeOnlyDepthStencilBuffer);
            break;
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH16:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            rtDesc.addRenderBuffer(m_writeOnlyDepthBuffer16);
            break;
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH24:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            rtDesc.addRenderBuffer(m_writeOnlyDepthBuffer24);
            break;
        case READ_WRITE_DEPTH16:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            rtDesc.addRenderBuffer(m_readWriteDepthBuffer16);
            break;
        case READ_WRITE_DEPTH24:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            rtDesc.addRenderBuffer(m_readWriteDepthBuffer24);
            break;
        case READ_WRITE_DEPTH32:
            rtDesc.addRenderBuffer(m_readWriteColorRenderBuffer);
            rtDesc.addRenderBuffer(m_readWriteDepthBuffer32);
            break;
        default:
            assert(false);
            break;
        }

        return *m_scene.createRenderTarget(rtDesc);
    }

    void RenderBufferScene::initClearPass(uint32_t state)
    {
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setRenderOrder(-100);
        renderPass->setCamera(createCamera());

        ramses::RenderTarget& renderTarget = createRenderTarget(state);

        renderPass->setRenderTarget(&renderTarget);
        renderPass->setClearColor({1.f, 0.f, 1.f, 0.5f});
        renderPass->setClearFlags(ramses::EClearFlag::All);
    }

    void RenderBufferScene::initRenderingPass(uint32_t state)
    {
        ramses::MeshNode& meshNode = createMesh(getEffectRenderOneBuffer());

        //fill stencil buffer with value of 1 for every fragment that gets rendered into
        meshNode.getAppearance()->setStencilFunction(ramses::EStencilFunc::Always, 1, 0xff);
        meshNode.getAppearance()->setStencilOperation(ramses::EStencilOperation::Replace, ramses::EStencilOperation::Replace, ramses::EStencilOperation::Replace);

        ramses::Node& transNode = *m_scene.createNode();
        transNode.addChild(meshNode);
        transNode.translate({0.0f, -0.5f, -5.0f});

        ramses::RenderGroup& renderGroup = *m_scene.createRenderGroup();
        renderGroup.addMeshNode(meshNode);

        ramses::RenderPass& renderPass = *m_scene.createRenderPass();
        renderPass.setRenderOrder(0);
        renderPass.addRenderGroup(renderGroup);

        ramses::PerspectiveCamera& camera = createCamera();
        renderPass.setCamera(camera);

        ramses::RenderTarget& renderTarget = createRenderTarget(state);
        renderPass.setRenderTarget(&renderTarget);
        renderPass.setClearFlags(ramses::EClearFlag::None);

        float zTranslate = -0.1f;
        switch (state)
        {
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_STENCIL_BUFFER:
            zTranslate = 0.1f;
            break;
        case READ_WRITE_DEPTH16:
        case READ_WRITE_DEPTH24:
        case READ_WRITE_DEPTH32:
            zTranslate = 3.f; // move closer to camera for better contrast with far plane (displayed as full red color)
            break;
        case ONE_COLOR_BUFFER_NO_DEPTH_OR_STENCIL:
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_BUFFER:
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH16:
        case ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH24:
            break;
        default:
            assert(false);
            break;
        }

        ramses::Node& farTriangleTransNode = *m_scene.createNode();
        farTriangleTransNode.translate({ 0.5f, 0.0f, zTranslate });

        ramses::MeshNode& meshNode2 = createMesh(getEffectRenderOneBuffer(), TriangleAppearance::EColor::Blue);
        farTriangleTransNode.addChild(meshNode2);
        transNode.addChild(farTriangleTransNode);

        meshNode2.getAppearance()->setDepthFunction(ramses::EDepthFunc::LessEqual);
        meshNode2.getAppearance()->setStencilFunction(ramses::EStencilFunc::NotEqual, 0u, 0xff);
        renderGroup.addMeshNode(meshNode2);

        renderGroup.addMeshNode(meshNode2);
    }
}
