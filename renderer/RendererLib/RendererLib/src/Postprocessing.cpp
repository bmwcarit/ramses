//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/Postprocessing.h"

#include "RendererAPI/IDevice.h"
#include "RendererLib/WarpingPass.h"
#include "RendererLib/WarpingMeshData.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{
    Postprocessing::Postprocessing(UInt32 effectIds, UInt32 width, UInt32 height, IDevice& device)
        : m_postEffectsMask(effectIds)
        , m_device(device)
        , m_displayWidth(width)
        , m_displayHeight(height)
        , m_framebuffer(device.getFramebufferRenderTarget())
        , m_warpingPass(0)
    {
        if (effectIds != EPostProcessingEffect_None)
        {
            m_scenesColorBuffer = m_device.uploadRenderBuffer      ({ m_displayWidth, m_displayHeight, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8,   ERenderBufferAccessMode_ReadWrite, 0u });
            m_sceneDepthStencilBuffer = m_device.uploadRenderBuffer({ m_displayWidth, m_displayHeight, ERenderBufferType_DepthBuffer, ETextureFormat_Depth24, ERenderBufferAccessMode_WriteOnly, 0u });
            assert(m_scenesColorBuffer.isValid());
            assert(m_sceneDepthStencilBuffer.isValid());

            DeviceHandleVector sceneRenderTargetBuffers;
            sceneRenderTargetBuffers.push_back(m_scenesColorBuffer);
            sceneRenderTargetBuffers.push_back(m_sceneDepthStencilBuffer);
            m_scenesRenderTarget = m_device.uploadRenderTarget(sceneRenderTargetBuffers);

            if (effectIds & EPostProcessingEffect_Warping)
            {
                WarpingMeshData warpingMeshQuad;
                m_warpingPass = new WarpingPass(m_device, warpingMeshQuad);
            }
        }
        else
        {
            m_scenesRenderTarget = m_framebuffer;
        }

        assert(m_framebuffer.isValid());
    }

    Postprocessing::~Postprocessing()
    {
        delete m_warpingPass;

        if (m_scenesRenderTarget != m_framebuffer)
        {
            m_device.deleteRenderTarget(m_scenesRenderTarget);

            m_device.deleteRenderBuffer(m_scenesColorBuffer);
            m_device.deleteRenderBuffer(m_sceneDepthStencilBuffer);
        }
    }

    void Postprocessing::execute()
    {
        if (0 != m_warpingPass)
        {
            m_device.activateRenderTarget(m_framebuffer);
            m_device.cullMode(ECullMode::Disabled);
            m_device.scissorTest(EScissorTest::Disabled, {});
            m_device.depthFunc(EDepthFunc::Disabled);
            m_device.depthWrite(EDepthWrite::Disabled);
            m_device.colorMask(true, true, true, true);
            // Use default clear color here, the content should be anyway overwritten by warped 'scenes buffer'.
            // If warping geometry does not cover whole frame, this clear color will be in the final image.
            // TODO vaclav use framebuffer clear color set in renderer also here.
            m_device.clearColor({ 0.f, 0.f, 0.f, 1.f });
            m_device.clear(EClearFlags_Color);
            m_device.setViewport(0u, 0u, m_displayWidth, m_displayHeight);

            m_warpingPass->execute(m_scenesColorBuffer);
        }
    }

    UInt32 Postprocessing::getPostEffectsMask() const
    {
        return m_postEffectsMask;
    }

    DeviceResourceHandle Postprocessing::getScenesRenderTarget() const
    {
        return m_scenesRenderTarget;
    }

    DeviceResourceHandle Postprocessing::getFramebuffer() const
    {
        return m_framebuffer;
    }

    void Postprocessing::setWarpingMeshData(const WarpingMeshData& warpingMeshData)
    {
        assert(0 != m_warpingPass);
        delete m_warpingPass;

        m_warpingPass = new WarpingPass(m_device, warpingMeshData);
    }
}
