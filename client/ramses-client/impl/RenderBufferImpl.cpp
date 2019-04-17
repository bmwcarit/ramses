//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderBufferImpl.h"
#include "Scene/ClientScene.h"
#include "TextureUtils.h"

namespace ramses
{
    RenderBufferImpl::RenderBufferImpl(SceneImpl& scene, const char* name)
        : SceneObjectImpl(scene, ERamsesObjectType_RenderBuffer, name)
    {
    }

    RenderBufferImpl::~RenderBufferImpl()
    {
    }

    void RenderBufferImpl::initializeFrameworkData(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount)
    {
        assert(!m_renderBufferHandle.isValid());
        m_renderBufferHandle = getIScene().allocateRenderBuffer({ width, height, TextureUtils::GetRenderBufferTypeInternal(bufferType), TextureUtils::GetRenderBufferFormatInternal(bufferFormat), TextureUtils::GetRenderBufferAccessModeInternal(accessMode), sampleCount });
        assert(m_renderBufferHandle.isValid());
    }

    void RenderBufferImpl::deinitializeFrameworkData()
    {
        assert(m_renderBufferHandle.isValid());
        getIScene().releaseRenderBuffer(m_renderBufferHandle);
        m_renderBufferHandle = ramses_internal::RenderBufferHandle::Invalid();
    }

    uint32_t RenderBufferImpl::getWidth() const
    {
        assert(m_renderBufferHandle.isValid());
        return getIScene().getRenderBuffer(m_renderBufferHandle).width;
    }

    uint32_t RenderBufferImpl::getHeight() const
    {
        assert(m_renderBufferHandle.isValid());
        return getIScene().getRenderBuffer(m_renderBufferHandle).height;
    }

    ERenderBufferType RenderBufferImpl::getBufferType() const
    {
        return TextureUtils::GetRenderBufferTypeFromInternal(getIScene().getRenderBuffer(m_renderBufferHandle).type);
    }

    ERenderBufferFormat RenderBufferImpl::getBufferFormat() const
    {
        return TextureUtils::GetRenderBufferFormatFromInternal(getIScene().getRenderBuffer(m_renderBufferHandle).format);
    }

    ERenderBufferAccessMode RenderBufferImpl::getAccessMode() const
    {
        return TextureUtils::GetRenderBufferAccessModeFromInternal(getIScene().getRenderBuffer(m_renderBufferHandle).accessMode);
    }

    uint32_t RenderBufferImpl::getSampleCount() const
    {
        assert(m_renderBufferHandle.isValid());
        return getIScene().getRenderBuffer(m_renderBufferHandle).sampleCount;
    }

    ramses_internal::RenderBufferHandle RenderBufferImpl::getRenderBufferHandle() const
    {
        return m_renderBufferHandle;
    }

    status_t RenderBufferImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_renderBufferHandle;

        return StatusOK;
    }

    status_t RenderBufferImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_renderBufferHandle;

        return StatusOK;
    }

    status_t RenderBufferImpl::validate(uint32_t indent) const
    {
        status_t status = SceneObjectImpl::validate(indent);
        indent += IndentationStep;

        const auto& iscene = getIScene();

        const bool isColorBuffer = (iscene.getRenderBuffer(getRenderBufferHandle()).type == ramses_internal::ERenderBufferType_ColorBuffer);

        bool usedAsTexture = false;
        for (ramses_internal::TextureSamplerHandle sampler(0u); sampler < iscene.getTextureSamplerCount() && !usedAsTexture; ++sampler)
        {
            usedAsTexture =
                iscene.isTextureSamplerAllocated(sampler) &&
                iscene.getTextureSampler(sampler).contentType == ramses_internal::TextureSampler::ContentType::RenderBuffer &&
                iscene.getTextureSampler(sampler).contentHandle == getRenderBufferHandle().asMemoryHandle();
        }

        bool usedInRenderPass = false;
        ramses_internal::RenderTargetHandle rtHandle;
        for (ramses_internal::RenderTargetHandle rt(0u); rt < iscene.getRenderTargetCount() && !rtHandle.isValid(); ++rt)
        {
            if (!iscene.isRenderTargetAllocated(rt))
                continue;

            for (uint32_t i = 0u; i < iscene.getRenderTargetRenderBufferCount(rt); ++i)
            {
                if (iscene.getRenderTargetRenderBuffer(rt, i) == getRenderBufferHandle())
                {
                    rtHandle = rt;
                    break;
                }
            }
        }
        if (rtHandle.isValid())
        {
            for (ramses_internal::RenderPassHandle rp(0u); rp < iscene.getRenderPassCount(); ++rp)
            {
                if (iscene.isRenderPassAllocated(rp) && iscene.getRenderPass(rp).renderTarget == rtHandle && iscene.getRenderPass(rp).isEnabled)
                {
                    usedInRenderPass = true;
                    break;
                }
            }
        }

        bool usedAsBlitDestination = false;
        bool usedAsBlitSource = false;
        ramses_internal::BlitPassHandle blitHandle;
        for (ramses_internal::BlitPassHandle bp(0u); bp < iscene.getBlitPassCount() && !blitHandle.isValid(); ++bp)
        {
            if (!iscene.isBlitPassAllocated(bp))
                continue;

            if (iscene.getBlitPass(bp).destinationRenderBuffer == getRenderBufferHandle())
            {
                usedAsBlitDestination = true;
                blitHandle = bp;
                break;
            }

            if (iscene.getBlitPass(bp).sourceRenderBuffer == getRenderBufferHandle())
            {
                usedAsBlitSource = true;
                blitHandle = bp;
                break;
            }
        }

        bool hasIssue = false;
        // explicitly warn about usage of potentially uninitialized buffer
        if (usedAsTexture && !(usedInRenderPass || usedAsBlitDestination))
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "RenderBuffer is used in a TextureSampler for reading but is not set as destination in any RenderPass or BlitPass, this can lead to usage of uninitialized data.");
            hasIssue = true;
        }

        if (!usedInRenderPass && !usedAsBlitDestination)
        {
            hasIssue = true;
            addValidationMessage(EValidationSeverity_Warning, indent, "RenderBuffer is not set as destination in any RenderPass or BlitPass, destroy it if not needed.");
        }

        if (!usedAsTexture && !usedAsBlitSource && isColorBuffer) // depth/stencil buffer does not need to be validated for usage as texture
        {
            hasIssue = true;
            addValidationMessage(EValidationSeverity_Warning, indent, "RenderBuffer is neither used in a TextureSampler for reading nor set as source in a BlitPass, destroy it if not needed.");
        }

        if (hasIssue)
        {
            ramses_internal::StringOutputStream rbDesc;
            const ramses_internal::RenderBuffer& rb = getIScene().getRenderBuffer(m_renderBufferHandle);
            rbDesc << " [" << rb.width << "x" << rb.height << "; " << ramses_internal::EnumToString(rb.type) << "; " << ramses_internal::EnumToString(rb.format) << "; " << ramses_internal::EnumToString(rb.accessMode) << "; " << rb.sampleCount << " samples]";
            addValidationMessage(EValidationSeverity_Warning, indent, rbDesc.c_str());
            return getValidationErrorStatus();
        }

        return status;
    }
}
