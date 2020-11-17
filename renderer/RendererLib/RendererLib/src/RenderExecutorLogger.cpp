//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderExecutorLogger.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererCachedScene.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/BlitPass.h"

namespace ramses_internal
{
    RenderExecutorLogger::RenderExecutorLogger(IDevice& device, const TargetBufferInfo& bufferInfo, RendererLogContext& context)
        : RenderExecutor(device, bufferInfo)
        , m_logContext(context)
    {
    }

    void RenderExecutorLogger::logScene(const RendererCachedScene& scene) const
    {
        setGlobalInternalStates(scene);

        m_logContext << RendererLogContext::NewLine << "Scene [id: " << scene.getSceneId() << "]" << RendererLogContext::NewLine;
        m_logContext.indent();

        const RenderingPassInfoVector& orderedPasses = scene.getSortedRenderingPasses();
        for (const auto& passInfo : orderedPasses)
        {
            switch (passInfo.getType())
            {
            case ERenderingPassType::RenderPass:
                logRenderPass(scene, passInfo.getRenderPassHandle());
                break;
            case ERenderingPassType::BlitPass:
                logBlitPass(scene, passInfo.getBlitPassHandle());
                break;
            default:
                m_logContext << "Unknown rendering pass type!" << RendererLogContext::NewLine;
            }
        }

        // End of scene
        m_logContext.unindent();
    }

    void RenderExecutorLogger::logRenderPass(const RendererCachedScene& scene, const RenderPassHandle pass) const
    {
        // Begin of render pass
        m_logContext << RendererLogContext::NewLine << "Start render pass " << pass << RendererLogContext::NewLine;
        const RenderPass& rp = scene.getRenderPass(pass);
        if (rp.isRenderOnce)
            m_logContext << " - 'render once' pass" << RendererLogContext::NewLine;
        m_logContext.indent();

        const RenderableVector& orderedRenderables = scene.getOrderedRenderablesForPass(pass);
        if (!orderedRenderables.empty())
        {
            const RenderTargetHandle renderTarget = rp.renderTarget;
            if (renderTarget.isValid())
            {
                const DeviceResourceHandle rtHandle = scene.getCachedHandlesForRenderTargets()[renderTarget.asMemoryHandle()];
                const RenderBufferHandle firstBufferHandle = scene.getRenderTargetRenderBuffer(renderTarget, 0u);
                const RenderBuffer& renderBuffer = scene.getRenderBuffer(firstBufferHandle);

                m_logContext << "--> Render target " << renderTarget << " [Device handle: " << rtHandle << "]" << RendererLogContext::NewLine;
                m_logContext << "    Size: " << renderBuffer.width << " x " << renderBuffer.height << RendererLogContext::NewLine;
                m_logContext << "    Access mode: " << RenderBufferAccessModeNames[renderBuffer.accessMode] << RendererLogContext::NewLine;
                m_logContext << "    Sample count: " << renderBuffer.sampleCount << RendererLogContext::NewLine;

                const UInt32 clearFlags = rp.clearFlags;
                m_logContext << "    Clear flags: "
                            << ((clearFlags & EClearFlags_Color) ? "Color|" : "")
                            << ((clearFlags & EClearFlags_Depth) ? "Depth|" : "")
                            << ((clearFlags & EClearFlags_Stencil) ? "Stencil" : "")
                            << ((clearFlags == EClearFlags_None) ? "None" : "")
                            << RendererLogContext::NewLine;

                if (clearFlags & EClearFlags_Color)
                {
                    const Vector4& clearColor = rp.clearColor;
                    m_logContext << "    Clear color: ["
                        << clearColor.r << " "
                        << clearColor.g << " "
                        << clearColor.b << " "
                        << clearColor.a << "]" << RendererLogContext::NewLine;
                }

                m_logContext << "    Render buffers: " << scene.getRenderTargetRenderBufferCount(renderTarget) << RendererLogContext::NewLine;

                for (UInt32 i = 0; i < scene.getRenderTargetRenderBufferCount(renderTarget); i++)
                {
                    const RenderBufferHandle rbHandle = scene.getRenderTargetRenderBuffer(renderTarget, i);
                    const DeviceResourceHandle deviceHandle = GetDeviceHandleForRenderBuffer(rbHandle, scene);
                    const RenderBuffer& rbData = scene.getRenderBuffer(rbHandle);

                    m_logContext << "      Render buffer (index " << i << ")" << RendererLogContext::NewLine;
                    m_logContext << "         Type: " << RenderBufferTypeNames[rbData.type] << RendererLogContext::NewLine;
                    m_logContext << "         Format: " << EnumToString(rbData.format) << RendererLogContext::NewLine;
                    m_logContext << "         [RenderBuffer handle: " << rbHandle << "]" << RendererLogContext::NewLine;
                    m_logContext << "         [Device handle: " << deviceHandle << "]" << RendererLogContext::NewLine;
                }
            }
            else
            {
                m_logContext << "--> Render to framebuffer" << RendererLogContext::NewLine;
            }

            m_logContext << RendererLogContext::NewLine;
            executeRenderTarget(rp.renderTarget);
            executeCamera(rp.camera);
            m_logContext << RendererLogContext::NewLine;

            for (const auto renderable : orderedRenderables)
            {
                const Bool filterMatches = m_logContext.isMatchingNodeHandeFilter(scene.getRenderable(renderable).node);
                const Bool detailedLogging = m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details);

                // skip renderable name, unless it was both filtered out and logging is not detailed
                if (filterMatches || detailedLogging)
                {
                    m_logContext << "Renderable " << renderable << " [\"" << scene.getRenderable(renderable).node.asMemoryHandle() << "\"]" << RendererLogContext::NewLine;
                }

                if (!filterMatches)
                {
                    // If detailled logging is enabled, dont log renderable content but mention it's being rendered
                    if (detailedLogging)
                    {
                        m_logContext << "  - not logging due to filter, note that render states might be changed" << RendererLogContext::NewLine;
                    }

                    continue;
                }

                m_logContext.indent();

                if (!scene.renderableResourcesDirty(renderable))
                {
                    setRenderableInternalStates(renderable);
                    setSemanticDataFields();
                    executeRenderable();
                }
                else
                {
                    m_logContext << "Missing resource(s)";
                }

                m_logContext.unindent();
                m_logContext << RendererLogContext::NewLine;
            }
        }
        else
        {
            m_logContext << "No renderables in render pass!" << RendererLogContext::NewLine;
        }

        // End of render pass
        m_logContext.unindent();
    }


    void RenderExecutorLogger::logBlitPass(const RendererCachedScene& scene, const BlitPassHandle pass) const
    {
        // Begin of blit pass
        m_logContext << RendererLogContext::NewLine << "Start blit pass " << pass << RendererLogContext::NewLine;
        m_logContext.indent();

        const BlitPass& blitPass = scene.getBlitPass(pass);
        m_logContext << "    Source render buffer       : " << blitPass.sourceRenderBuffer << RendererLogContext::NewLine;
        m_logContext << "    Destination render buffer  : " << blitPass.destinationRenderBuffer << RendererLogContext::NewLine;
        m_logContext << "    Source region              : [x=" << blitPass.sourceRegion.x
                                                    << ", y=" << blitPass.sourceRegion.y
                                                    << ", width=" << blitPass.sourceRegion.width
                                                    << ", height=" << blitPass.sourceRegion.height
                                                    << RendererLogContext::NewLine;

        m_logContext << "    Destination region         : [x=" << blitPass.destinationRegion.x
                                                    << ", y=" << blitPass.destinationRegion.y
                                                    << ", width=" << blitPass.destinationRegion.width
                                                    << ", height=" << blitPass.destinationRegion.height
                                                    << RendererLogContext::NewLine;

        // End of blit pass
        m_logContext.unindent();
    }

    DeviceResourceHandle RenderExecutorLogger::GetDeviceHandleForRenderBuffer(RenderBufferHandle rbHandle, const RendererCachedScene& scene)
    {
        const UInt32 samplerCount = scene.getTextureSamplerCount();
        for (TextureSamplerHandle samplerHandle(0u); samplerHandle < samplerCount; ++samplerHandle)
        {
            if (scene.isTextureSamplerAllocated(samplerHandle) && scene.getTextureSampler(samplerHandle).contentType == TextureSampler::ContentType::RenderBuffer)
            {
                const RenderBufferHandle samplerRbHandle(scene.getTextureSampler(samplerHandle).contentHandle);
                if (samplerRbHandle == rbHandle)
                {
                    return scene.getCachedHandlesForTextureSamplers()[samplerRbHandle.asMemoryHandle()];
                }
            }
        }

        return DeviceResourceHandle::Invalid();
    }
}
