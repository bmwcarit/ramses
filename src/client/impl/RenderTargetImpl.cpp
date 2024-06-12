//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RenderTargetImpl.h"
#include "impl/RenderTargetDescriptionImpl.h"
#include "impl/SerializationContext.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    RenderTargetImpl::RenderTargetImpl(SceneImpl& scene, std::string_view name)
        : SceneObjectImpl(scene, ERamsesObjectType::RenderTarget, name)
        , m_renderTargetHandle( ramses::internal::RenderTargetHandle::Invalid() )
    {
    }

    RenderTargetImpl::~RenderTargetImpl() = default;

    void RenderTargetImpl::initializeFrameworkData(const RenderTargetDescriptionImpl& rtDesc)
    {
        assert(!m_renderTargetHandle.isValid());
        assert(!rtDesc.getRenderBuffers().empty());

        m_renderTargetHandle = getIScene().allocateRenderTarget({});
        assert(m_renderTargetHandle.isValid());

        const ramses::internal::RenderBufferHandleVector& rtBuffers = rtDesc.getRenderBuffers();
        for(const auto& rb : rtBuffers)
        {
            getIScene().addRenderTargetRenderBuffer(m_renderTargetHandle, rb);
        }
    }

    void RenderTargetImpl::deinitializeFrameworkData()
    {
        assert(m_renderTargetHandle.isValid());
        getIScene().releaseRenderTarget(m_renderTargetHandle);
        m_renderTargetHandle = ramses::internal::RenderTargetHandle::Invalid();
    }

    uint32_t RenderTargetImpl::getWidth() const
    {
        // In order to support legacy API, RenderTarget can be queried for resolution
        // RenderTarget has to always have at least 1 RenderBuffer and all of RenderBuffers have same resolution
        // therefore we can query the first RenderBuffer for resolution
        assert(m_renderTargetHandle.isValid());
        assert(getIScene().getRenderTargetRenderBufferCount(m_renderTargetHandle) != 0u);

        const ramses::internal::RenderBufferHandle firstRenderBufferHandle = getIScene().getRenderTargetRenderBuffer(m_renderTargetHandle, 0u);
        return getIScene().getRenderBuffer(firstRenderBufferHandle).width;
    }

    uint32_t RenderTargetImpl::getHeight() const
    {
        // In order to support legacy API, RenderTarget can be queried for resolution
        // RenderTarget has to always have at least 1 RenderBuffer and all of RenderBuffers have same resolution
        // therefore we can query the first RenderBuffer for resolution
        assert(m_renderTargetHandle.isValid());
        assert(getIScene().getRenderTargetRenderBufferCount(m_renderTargetHandle) != 0u);

        const ramses::internal::RenderBufferHandle firstRenderBufferHandle = getIScene().getRenderTargetRenderBuffer(m_renderTargetHandle, 0u);
        return getIScene().getRenderBuffer(firstRenderBufferHandle).height;
    }

    ramses::internal::RenderTargetHandle RenderTargetImpl::getRenderTargetHandle() const
    {
        return m_renderTargetHandle;
    }

    bool RenderTargetImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_renderTargetHandle;

        return true;
    }

    bool RenderTargetImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        serializationContext.deserializeAndMap(inStream, m_renderTargetHandle);

        return true;
    }

    void RenderTargetImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        const auto& iscene = getIScene();
        const uint32_t rbCount = iscene.getRenderTargetRenderBufferCount(m_renderTargetHandle);
        assert(rbCount > 0u);
        const auto& firstRB = iscene.getRenderBuffer(iscene.getRenderTargetRenderBuffer(m_renderTargetHandle, 0u));

        for (uint32_t i = 1u; i < rbCount; ++i)
        {
            const auto& rb = iscene.getRenderBuffer(iscene.getRenderTargetRenderBuffer(m_renderTargetHandle, i));
            if (firstRB.width != rb.width ||
                firstRB.height != rb.height ||
                firstRB.sampleCount != rb.sampleCount)
            {
                report.add(EIssueType::Error,
                    "RenderTarget uses render buffers with mismatching resolution and/or sample count. This RenderTarget cannot be used and attempt to flush scene will fail.", &getRamsesObject());
            }
        }
    }
}
