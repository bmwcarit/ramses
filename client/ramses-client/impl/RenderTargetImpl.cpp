//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderTargetImpl.h"
#include "RenderTargetDescriptionImpl.h"
#include "Scene/ClientScene.h"
#include "Common/Cpp11Macros.h"

namespace ramses
{
    RenderTargetImpl::RenderTargetImpl(SceneImpl& scene, const char* name)
        : SceneObjectImpl(scene, ERamsesObjectType_RenderTarget, name)
        , m_renderTargetHandle( ramses_internal::RenderTargetHandle::Invalid() )
    {
    }

    RenderTargetImpl::~RenderTargetImpl()
    {
    }

    void RenderTargetImpl::initializeFrameworkData(const RenderTargetDescriptionImpl& rtDesc)
    {
        assert(!m_renderTargetHandle.isValid());
        assert(!rtDesc.getRenderBuffers().empty());

        m_renderTargetHandle = getIScene().allocateRenderTarget();
        assert(m_renderTargetHandle.isValid());

        const ramses_internal::RenderBufferHandleVector& rtBuffers = rtDesc.getRenderBuffers();
        ramses_foreach(rtBuffers, rb)
        {
            getIScene().addRenderTargetRenderBuffer(m_renderTargetHandle, *rb);
        }
    }

    void RenderTargetImpl::deinitializeFrameworkData()
    {
        assert(m_renderTargetHandle.isValid());
        getIScene().releaseRenderTarget(m_renderTargetHandle);
        m_renderTargetHandle = ramses_internal::RenderTargetHandle::Invalid();
    }

    uint32_t RenderTargetImpl::getWidth() const
    {
        // In order to support legacy API, RenderTarget can be queried for resolution
        // RenderTarget has to always have at least 1 RenderBuffer and all of RenderBuffers have same resolution
        // therefore we can query the first RenderBuffer for resolution
        assert(m_renderTargetHandle.isValid());
        assert(getIScene().getRenderTargetRenderBufferCount(m_renderTargetHandle) != 0u);

        const ramses_internal::RenderBufferHandle firstRenderBufferHandle = getIScene().getRenderTargetRenderBuffer(m_renderTargetHandle, 0u);
        return getIScene().getRenderBuffer(firstRenderBufferHandle).width;
    }

    uint32_t RenderTargetImpl::getHeight() const
    {
        // In order to support legacy API, RenderTarget can be queried for resolution
        // RenderTarget has to always have at least 1 RenderBuffer and all of RenderBuffers have same resolution
        // therefore we can query the first RenderBuffer for resolution
        assert(m_renderTargetHandle.isValid());
        assert(getIScene().getRenderTargetRenderBufferCount(m_renderTargetHandle) != 0u);

        const ramses_internal::RenderBufferHandle firstRenderBufferHandle = getIScene().getRenderTargetRenderBuffer(m_renderTargetHandle, 0u);
        return getIScene().getRenderBuffer(firstRenderBufferHandle).height;
    }

    ramses_internal::RenderTargetHandle RenderTargetImpl::getRenderTargetHandle() const
    {
        return m_renderTargetHandle;
    }

    status_t RenderTargetImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_renderTargetHandle;

        return StatusOK;
    }

    status_t RenderTargetImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_renderTargetHandle;

        return StatusOK;
    }
}
