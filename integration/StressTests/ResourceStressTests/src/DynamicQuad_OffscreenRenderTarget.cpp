//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_OffscreenRenderTarget.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderTargetDescription.h"

#include "TestRandom.h"

namespace ramses_internal
{
    DynamicQuad_OffscreenRenderTarget::DynamicQuad_OffscreenRenderTarget(ramses::RamsesClient& client, ramses::Scene& scene, ramses::RenderPass& offscreenRenderPass, const ScreenspaceQuad& screenspaceQuad)
        : DynamicQuad_Base(client, scene, screenspaceQuad)
        , m_offscreenRenderPass(offscreenRenderPass)
    {
        m_renderGroup.addMeshNode(m_meshNode);

        m_quadResources = createRandomizedQuadResources();
        m_renderTargetSceneObjects = createRenderTarget();
        setQuadResources(m_quadResources, *m_renderTargetSceneObjects.textureSampler);

        m_meshNode.setAppearance(m_appearance);
        m_meshNode.setIndexCount(4);
        m_appearance.setDrawMode(ramses::EDrawMode_TriangleStrip);
        m_meshNode.setGeometryBinding(m_geometryBinding);
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
    }

    RenderTargetResources DynamicQuad_OffscreenRenderTarget::createRenderTarget()
    {
        RenderTargetResources resources;

        const uint32_t randomDivisor = static_cast<uint32_t>(TestRandom::Get(5, 10));

        resources.renderBuffer = m_scene.createRenderBuffer(m_screenspaceQuad.getWidthOfScreen() / randomDivisor, m_screenspaceQuad.getHeightOfScreen()/ randomDivisor, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite);

        ramses::RenderTargetDescription renderTargetDescription;
        renderTargetDescription.addRenderBuffer(*resources.renderBuffer);

        resources.renderTarget = m_scene.createRenderTarget(renderTargetDescription);
        m_offscreenRenderPass.setRenderTarget(resources.renderTarget);

        resources.textureSampler = m_scene.createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
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
