//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/BlitPassScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/BlitPass.h"
#include <cassert>

namespace ramses_internal
{
    BlitPassScene::BlitPassScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition)
        : CommonRenderBufferTestScene(scene, cameraPosition)
        , m_colorBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite))
        , m_depthBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Depth, ramses::ERenderBufferFormat_Depth24, ramses::ERenderBufferAccessMode_WriteOnly))
        , m_depthStencilBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_DepthStencil, ramses::ERenderBufferFormat_Depth24_Stencil8, ramses::ERenderBufferAccessMode_WriteOnly))
        , m_blittingColorBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite))
        , m_blittingDepthBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Depth, ramses::ERenderBufferFormat_Depth24, ramses::ERenderBufferAccessMode_WriteOnly))
        , m_blittingDepthStencilBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_DepthStencil, ramses::ERenderBufferFormat_Depth24_Stencil8, ramses::ERenderBufferAccessMode_WriteOnly))
    {
        initClearPass(state);
        initClearPassForBlittingBuffers(state);
        initRenderingPass(state);
        initBlittingPass(state);
        initRenderPassFromBlittingResult(state);
        addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_blittingColorBuffer));
    }

    ramses::RenderTarget& BlitPassScene::createRenderTarget(UInt32 state)
    {
        ramses::RenderTargetDescription rtDesc;

        switch (state)
        {
        case BLITS_COLOR_BUFFER:
        case BLITS_SUBREGION:
            rtDesc.addRenderBuffer(m_colorBuffer);
            break;
        case BLITS_DEPTH_BUFFER:
            rtDesc.addRenderBuffer(m_colorBuffer);
            rtDesc.addRenderBuffer(m_depthBuffer);
            break;
        case BLITS_DEPTH_STENCIL_BUFFER:
            rtDesc.addRenderBuffer(m_colorBuffer);
            rtDesc.addRenderBuffer(m_depthStencilBuffer);
            break;
        default:
            assert(false);
            break;
        }

        return *m_scene.createRenderTarget(rtDesc);
    }

    ramses::RenderTarget& BlitPassScene::createBlittingRenderTarget(UInt32 state)
    {
        ramses::RenderTargetDescription rtDesc;
        switch (state)
        {
        case BLITS_COLOR_BUFFER:
        case BLITS_SUBREGION:
            rtDesc.addRenderBuffer(m_blittingColorBuffer);
            break;
        case BLITS_DEPTH_BUFFER:
            rtDesc.addRenderBuffer(m_blittingColorBuffer);
            rtDesc.addRenderBuffer(m_blittingDepthBuffer);
            break;
        case BLITS_DEPTH_STENCIL_BUFFER:
            rtDesc.addRenderBuffer(m_blittingColorBuffer);
            rtDesc.addRenderBuffer(m_blittingDepthStencilBuffer);
            break;
        default:
            assert(false);
            break;
        }
        return *m_scene.createRenderTarget(rtDesc);
    }

    void BlitPassScene::initClearPass(UInt32 state)
    {
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setRenderOrder(-100);
        renderPass->setCamera(createCamera());

        ramses::RenderTarget& renderTarget = createRenderTarget(state);

        renderPass->setRenderTarget(&renderTarget);
        renderPass->setClearColor({1.f, 0.f, 1.f, 0.5f});
        renderPass->setClearFlags(ramses::EClearFlags_All);
    }


    void BlitPassScene::initRenderingPass(UInt32 state)
    {
        ramses::MeshNode& meshNode = createMesh(getEffectRenderOneBuffer());

        ramses::Node& transNode = *m_scene.createNode();
        transNode.addChild(meshNode);
        transNode.translate({0.0f, -0.5f, -5.0f});

        ramses::RenderGroup& renderGroup = *m_scene.createRenderGroup();
        renderGroup.addMeshNode(meshNode);

        meshNode.getAppearance()->setStencilFunction(ramses::EStencilFunc_Always, 1, 0xff);
        meshNode.getAppearance()->setStencilOperation(ramses::EStencilOperation_Replace, ramses::EStencilOperation_Replace, ramses::EStencilOperation_Replace);

        ramses::RenderPass& renderPass = *m_scene.createRenderPass();
        renderPass.setRenderOrder(0);
        renderPass.addRenderGroup(renderGroup);

        ramses::PerspectiveCamera& camera = createCamera();
        renderPass.setCamera(camera);

        ramses::RenderTarget& renderTarget = createRenderTarget(state);
        renderPass.setRenderTarget(&renderTarget);
        renderPass.setClearFlags(ramses::EClearFlags_None);
    }

    void BlitPassScene::initBlittingPass(UInt32 state)
    {
        ramses::BlitPass* blitPass = nullptr;
        switch (state)
        {
        case BLITS_COLOR_BUFFER:
            blitPass = m_scene.createBlitPass(m_colorBuffer, m_blittingColorBuffer);
            break;
        case BLITS_SUBREGION:
            blitPass = m_scene.createBlitPass(m_colorBuffer, m_blittingColorBuffer);
            blitPass->setBlittingRegion(8u, 6u, 6u, 10u, 8u, 4u);
            break;
        case BLITS_DEPTH_BUFFER:
            blitPass = m_scene.createBlitPass(m_depthBuffer, m_blittingDepthBuffer);
            break;
        case BLITS_DEPTH_STENCIL_BUFFER:
            blitPass = m_scene.createBlitPass(m_depthStencilBuffer, m_blittingDepthStencilBuffer);
            break;
        default:
            assert(false);
            break;
        }

        assert(nullptr != blitPass);

        blitPass->setRenderOrder(1);
    }

    void BlitPassScene::initClearPassForBlittingBuffers(UInt32 state)
    {
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setRenderOrder(-100);
        renderPass->setCamera(createCamera());

        ramses::RenderTarget& renderTarget = createBlittingRenderTarget(state);
        renderPass->setRenderTarget(&renderTarget);
        renderPass->setClearColor({0.f, 0.f, 1.f, 0.5f});
        renderPass->setClearFlags(ramses::EClearFlags_All);
    }

    void BlitPassScene::initRenderPassFromBlittingResult(UInt32 state)
    {
        if (BLITS_COLOR_BUFFER == state || BLITS_SUBREGION == state)
        {
            return;
        }

        ramses::MeshNode& meshNode = createMesh(getEffectRenderOneBuffer());

        ramses::Node& transNode = *m_scene.createNode();
        transNode.addChild(meshNode);
        if (BLITS_DEPTH_BUFFER == state ||
            BLITS_DEPTH_STENCIL_BUFFER == state)
        {
            transNode.translate({0.0f, -0.0f, -5.1f});
        }
        else
        {
            assert(false);
        }

        ramses::RenderGroup& renderGroup = *m_scene.createRenderGroup();
        renderGroup.addMeshNode(meshNode);

        if (BLITS_DEPTH_STENCIL_BUFFER == state)
        {
            //add another mesh that is filtered by stencil
            ramses::MeshNode& meshNode2 = createMesh(getEffectRenderOneBuffer(), ramses::TriangleAppearance::EColor_Green);
            meshNode2.getAppearance()->setStencilFunction(ramses::EStencilFunc_NotEqual, 0u, 0xff);

            ramses::Node& transNode2 = *m_scene.createNode();
            transNode2.addChild(meshNode2);
            transNode2.translate({0.0f, -0.8f, -4.9f});

            renderGroup.addMeshNode(meshNode2);
        }

        ramses::RenderPass& renderPass = *m_scene.createRenderPass();
        renderPass.setRenderOrder(3);
        renderPass.addRenderGroup(renderGroup);

        ramses::PerspectiveCamera& camera = createCamera();
        renderPass.setCamera(camera);

        ramses::RenderTarget& renderTarget = createBlittingRenderTarget(state);
        renderPass.setRenderTarget(&renderTarget);
        renderPass.setClearFlags(ramses::EClearFlags_None);
    }
}
