//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_OffscreenRenderTarget.h"

#include "ramses/client/Scene.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/Camera.h"
#include "ramses/client/Effect.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/RenderTargetDescription.h"

#include "TestRandom.h"

namespace ramses::internal
{
    DynamicQuad_OffscreenRenderTarget::DynamicQuad_OffscreenRenderTarget(ramses::Scene& scene, ramses::RenderPass& offscreenRenderPass, const ScreenspaceQuad& screenspaceQuad)
        : DynamicQuad_Base(scene, screenspaceQuad)
        , m_offscreenRenderPass(offscreenRenderPass)
    {
        m_renderGroup.addMeshNode(m_meshNode);

        m_quadResources = createRandomizedQuadResources();
        m_renderTargetSceneObjects = createRenderTarget();
        setQuadResources(m_quadResources, *m_renderTargetSceneObjects.textureSampler);

        m_meshNode.setAppearance(m_appearance);
        m_meshNode.setIndexCount(4);
        m_appearance.setDrawMode(ramses::EDrawMode::TriangleStrip);
        m_meshNode.setGeometry(m_geometryBinding);
    }

    DynamicQuad_OffscreenRenderTarget::~DynamicQuad_OffscreenRenderTarget()
    {
        destroyQuadResources(m_quadResources);
        destroyRenderTarget(m_renderTargetSceneObjects);
    }

    void DynamicQuad_OffscreenRenderTarget::recreate()
    {
        const QuadResources oldQuad = m_quadResources;
        const RenderTargetResources oldRenderTarget = m_renderTargetSceneObjects;

        m_quadResources = createRandomizedQuadResources();
        m_renderTargetSceneObjects = createRenderTarget();
        setQuadResources(m_quadResources, *m_renderTargetSceneObjects.textureSampler);

        destroyQuadResources(oldQuad);
        destroyRenderTarget(oldRenderTarget);
    }

    void DynamicQuad_OffscreenRenderTarget::markSceneObjectsDestroyed()
    {
        m_renderTargetSceneObjects.renderBuffer = nullptr;
        m_renderTargetSceneObjects.renderTarget = nullptr;
        m_renderTargetSceneObjects.textureSampler = nullptr;
        m_quadResources.indices = nullptr;
        m_quadResources.texCoords = nullptr;
        m_quadResources.vertexPos = nullptr;
    }

    RenderTargetResources DynamicQuad_OffscreenRenderTarget::createRenderTarget()
    {
        RenderTargetResources resources;

        const auto randomDivisor = static_cast<uint32_t>(TestRandom::Get(5, 10));

        resources.renderBuffer = m_scene.createRenderBuffer(m_screenspaceQuad.getWidthOfScreen() / randomDivisor, m_screenspaceQuad.getHeightOfScreen()/ randomDivisor, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite);

        ramses::RenderTargetDescription renderTargetDescription;
        renderTargetDescription.addRenderBuffer(*resources.renderBuffer);

        resources.renderTarget = m_scene.createRenderTarget(renderTargetDescription);
        m_offscreenRenderPass.setRenderTarget(resources.renderTarget);

        resources.textureSampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Linear,
            ramses::ETextureSamplingMethod::Linear,
            *resources.renderBuffer);

        return resources;
    }

    void DynamicQuad_OffscreenRenderTarget::destroyRenderTarget(const RenderTargetResources& renderTargetResources)
    {
        if (nullptr != renderTargetResources.renderTarget)
        {
            m_scene.destroy(*renderTargetResources.renderTarget);
        }

        if (nullptr != renderTargetResources.renderBuffer)
        {
            m_scene.destroy(*renderTargetResources.renderBuffer);
        }

        if (nullptr != renderTargetResources.textureSampler)
        {
            m_scene.destroy(*renderTargetResources.textureSampler);
        }
    }
}
