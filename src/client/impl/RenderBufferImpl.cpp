//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RenderBufferImpl.h"
#include "impl/TextureUtils.h"
#include "impl/TextureEnumsImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    RenderBufferImpl::RenderBufferImpl(SceneImpl& scene, std::string_view name)
        : SceneObjectImpl(scene, ERamsesObjectType::RenderBuffer, name)
    {
    }

    RenderBufferImpl::~RenderBufferImpl() = default;

    void RenderBufferImpl::initializeFrameworkData(uint32_t width, uint32_t height, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount)
    {
        assert(!m_renderBufferHandle.isValid());
        m_renderBufferHandle = getIScene().allocateRenderBuffer({ width, height, TextureUtils::GetRenderBufferFormatInternal(bufferFormat), accessMode, sampleCount }, {});
        assert(m_renderBufferHandle.isValid());
    }

    void RenderBufferImpl::deinitializeFrameworkData()
    {
        assert(m_renderBufferHandle.isValid());
        getIScene().releaseRenderBuffer(m_renderBufferHandle);
        m_renderBufferHandle = ramses::internal::RenderBufferHandle::Invalid();
    }

    uint32_t RenderBufferImpl::getWidth() const
    {
        return getIScene().getRenderBuffer(m_renderBufferHandle).width;
    }

    uint32_t RenderBufferImpl::getHeight() const
    {
        return getIScene().getRenderBuffer(m_renderBufferHandle).height;
    }

    ERenderBufferFormat RenderBufferImpl::getBufferFormat() const
    {
        return TextureUtils::GetRenderBufferFormatFromInternal(getIScene().getRenderBuffer(m_renderBufferHandle).format);
    }

    ERenderBufferAccessMode RenderBufferImpl::getAccessMode() const
    {
        return getIScene().getRenderBuffer(m_renderBufferHandle).accessMode;
    }

    uint32_t RenderBufferImpl::getSampleCount() const
    {
        return getIScene().getRenderBuffer(m_renderBufferHandle).sampleCount;
    }

    ramses::internal::RenderBufferHandle RenderBufferImpl::getRenderBufferHandle() const
    {
        return m_renderBufferHandle;
    }

    bool RenderBufferImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_renderBufferHandle;

        return true;
    }

    bool RenderBufferImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_renderBufferHandle;

        return true;
    }

    void RenderBufferImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        const auto& iscene = getIScene();

        const bool isColorBuffer = !IsDepthOrStencilFormat(iscene.getRenderBuffer(getRenderBufferHandle()).format);

        bool usedAsTexture = false;
        for (ramses::internal::TextureSamplerHandle sampler(0u); sampler < iscene.getTextureSamplerCount() && !usedAsTexture; ++sampler)
        {
            usedAsTexture =
                iscene.isTextureSamplerAllocated(sampler) &&
                iscene.getTextureSampler(sampler).isRenderBuffer() &&
                iscene.getTextureSampler(sampler).contentHandle == getRenderBufferHandle().asMemoryHandle();
        }

        bool usedInRenderPass = false;
        ramses::internal::RenderTargetHandle rtHandle;
        for (ramses::internal::RenderTargetHandle rt(0u); rt < iscene.getRenderTargetCount() && !rtHandle.isValid(); ++rt)
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
            for (ramses::internal::RenderPassHandle rp(0u); rp < iscene.getRenderPassCount(); ++rp)
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
        ramses::internal::BlitPassHandle blitHandle;
        for (ramses::internal::BlitPassHandle bp(0u); bp < iscene.getBlitPassCount() && !blitHandle.isValid(); ++bp)
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
            report.add(EIssueType::Warning,
                                "RenderBuffer is used in a TextureSampler for reading but is not set as destination in any RenderPass or BlitPass, this can lead to usage of "
                                "uninitialized data.", &getRamsesObject());
            hasIssue = true;
        }

        if (!usedInRenderPass && !usedAsBlitDestination)
        {
            hasIssue = true;
            report.add(EIssueType::Warning, "RenderBuffer is not set as destination in any RenderPass or BlitPass, destroy it if not needed.", &getRamsesObject());
        }

        if (!usedAsTexture && !usedAsBlitSource && isColorBuffer) // depth/stencil buffer does not need to be validated for usage as texture
        {
            hasIssue = true;
            report.add(EIssueType::Warning, "RenderBuffer is neither used in a TextureSampler for reading nor set as source in a BlitPass, destroy it if not needed.", &getRamsesObject());
        }

        if (hasIssue)
        {
            ramses::internal::StringOutputStream rbDesc;
            const ramses::internal::RenderBuffer& rb = getIScene().getRenderBuffer(m_renderBufferHandle);
            rbDesc << " [" << rb.width << "x" << rb.height << "; " << ramses::internal::EnumToString(rb.format) << "; " << EnumToString(rb.accessMode) << "; " << rb.sampleCount << " samples]";
            return report.add(EIssueType::Warning, rbDesc.release(), &getRamsesObject());
        }
    }

    bool RenderBufferImpl::setProperties(uint32_t width, uint32_t height, uint32_t sampleCount)
    {
        if (width == 0 || height == 0)
        {
            getErrorReporting().set("RenderBuffer::setProperties: width and height cannot be zero", *this);
            return false;
        }

        getIScene().setRenderBufferProperties(m_renderBufferHandle, width, height, sampleCount);
        return true;
    }
}
