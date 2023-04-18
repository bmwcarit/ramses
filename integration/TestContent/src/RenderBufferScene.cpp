//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/RenderBufferScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include <cassert>

namespace ramses_internal
{
    RenderBufferScene::RenderBufferScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : CommonRenderBufferTestScene(scene, cameraPosition)
        , m_readWriteColorRenderBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite))
        , m_writeOnlyDepthBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_Depth, ramses::ERenderBufferFormat_Depth24, ramses::ERenderBufferAccessMode_WriteOnly))
        , m_writeOnlyDepthStencilBuffer(*scene.createRenderBuffer(16u, 16u, ramses::ERenderBufferType_DepthStencil, ramses::ERenderBufferFormat_Depth24_Stencil8, ramses::ERenderBufferAccessMode_WriteOnly))
    {
        initClearPass(state);
        initRenderingPass(state);
        addRenderPassUsingRenderBufferAsQuadTexture(createQuadWithTexture(m_readWriteColorRenderBuffer));
    }

    ramses::RenderTarget& RenderBufferScene::createRenderTarget(UInt32 state)
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
        default:
            assert(false);
            break;
        }

        return *m_scene.createRenderTarget(rtDesc);
    }

    void RenderBufferScene::initClearPass(UInt32 state)
    {
        ramses::RenderPass* renderPass = m_scene.createRenderPass();
        renderPass->setRenderOrder(-100);
        renderPass->setCamera(createCamera());

        ramses::RenderTarget& renderTarget = createRenderTarget(state);

        renderPass->setRenderTarget(&renderTarget);
        renderPass->setClearColor({1.f, 0.f, 1.f, 0.5f});
        renderPass->setClearFlags(ramses::EClearFlags_All);
    }

    void RenderBufferScene::initRenderingPass(UInt32 state)
    {
        ramses::MeshNode& meshNode = createMesh(getEffectRenderOneBuffer());

        //fill stencil buffer with value of 1 for every fragment that gets rendered into
        meshNode.getAppearance()->setStencilFunction(ramses::EStencilFunc_Always, 1, 0xff);
        meshNode.getAppearance()->setStencilOperation(ramses::EStencilOperation_Replace, ramses::EStencilOperation_Replace, ramses::EStencilOperation_Replace);

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
        renderPass.setClearFlags(ramses::EClearFlags_None);


        ramses::Node& farTriangleTransNode = *m_scene.createNode();

        if (state == ONE_COLOR_BUFFER_WITH_WRITE_ONLY_DEPTH_STENCIL_BUFFER)
        {
            farTriangleTransNode.translate({0.5f, 0.0f, 0.1f});
        }
        else
        {
            farTriangleTransNode.translate({0.5f, 0.0f, -0.1f});
        }

        ramses::MeshNode& meshNode2 = createMesh(getEffectRenderOneBuffer(), ramses::TriangleAppearance::EColor_Blue);
        farTriangleTransNode.addChild(meshNode2);
        transNode.addChild(farTriangleTransNode);

        meshNode2.getAppearance()->setDepthFunction(ramses::EDepthFunc_LessEqual);
        meshNode2.getAppearance()->setStencilFunction(ramses::EStencilFunc_NotEqual, 0u, 0xff);
        renderGroup.addMeshNode(meshNode2);

        renderGroup.addMeshNode(meshNode2);
    }
}
