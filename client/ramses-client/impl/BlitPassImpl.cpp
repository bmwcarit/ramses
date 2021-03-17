//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "BlitPassImpl.h"
#include "RenderBufferImpl.h"
#include "SerializationContext.h"
#include "SceneAPI/BlitPass.h"
#include "Scene/ClientScene.h"
#include "ramses-client-api/RenderBuffer.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses
{
    BlitPassImpl::BlitPassImpl(SceneImpl& scene, const char* blitpassName)
        : SceneObjectImpl(scene, ERamsesObjectType_BlitPass, blitpassName)
    {
    }

    BlitPassImpl::~BlitPassImpl()
    {
    }

    void BlitPassImpl::initializeFrameworkData(const RenderBufferImpl& sourceRenderBuffer, const RenderBufferImpl& destinationRenderBuffer)
    {
        assert(!m_blitPassHandle.isValid());
        m_sourceRenderBufferImpl = &sourceRenderBuffer;
        m_destinationRenderBufferImpl = &destinationRenderBuffer;

        const ramses_internal::RenderBufferHandle sourceRenderBufferHandle = sourceRenderBuffer.getRenderBufferHandle();
        const ramses_internal::RenderBufferHandle destinationRenderBufferHandle = destinationRenderBuffer.getRenderBufferHandle();
        m_blitPassHandle = getIScene().allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle);

        setBlittingRegion(0u, 0u, 0u, 0u, sourceRenderBuffer.getWidth(), sourceRenderBuffer.getHeight());
    }

    void BlitPassImpl::deinitializeFrameworkData()
    {
        assert(m_blitPassHandle.isValid());
        getIScene().releaseBlitPass(m_blitPassHandle);
        m_blitPassHandle = ramses_internal::BlitPassHandle::Invalid();
    }

    ramses::status_t BlitPassImpl::setBlittingRegion(uint32_t sourceX, uint32_t sourceY, uint32_t destinationX, uint32_t destinationY, uint32_t width, uint32_t height)
    {
        const ramses_internal::BlitPass& blitPass = getIScene().getBlitPass(m_blitPassHandle);
        const ramses_internal::RenderBuffer& renderBufferSrc = getIScene().getRenderBuffer(blitPass.sourceRenderBuffer);
        const ramses_internal::RenderBuffer& renderBufferDst = getIScene().getRenderBuffer(blitPass.destinationRenderBuffer);
        if (sourceX + width > renderBufferSrc.width || sourceY + height > renderBufferSrc.height)
        {
            return addErrorEntry("BlitPass::setBlittingRegion failed - invalid source region");
        }

        if (destinationX + width > renderBufferDst.width || destinationY + height > renderBufferDst.height)
        {
            return addErrorEntry("BlitPass::setBlittingRegion failed - invalid destination region");
        }

        const ramses_internal::PixelRectangle sourceRegion = {
            sourceX, sourceY, static_cast<int32_t>(width), static_cast<int32_t>(height)
        };

        const ramses_internal::PixelRectangle destinationRegion = {
            destinationX, destinationY, static_cast<int32_t>(width), static_cast<int32_t>(height)
        };

        getIScene().setBlitPassRegions(m_blitPassHandle, sourceRegion, destinationRegion);

        return StatusOK;
    }

    ramses::status_t BlitPassImpl::setRenderOrder(int32_t renderOrder)
    {
        getIScene().setBlitPassRenderOrder(m_blitPassHandle, renderOrder);

        return StatusOK;
    }

    int32_t BlitPassImpl::getRenderOrder() const
    {
        return getIScene().getBlitPass(m_blitPassHandle).renderOrder;
    }

    ramses::status_t BlitPassImpl::setEnabled(bool isEnabeld)
    {
        getIScene().setBlitPassEnabled(m_blitPassHandle, isEnabeld);

        return StatusOK;
    }

    bool BlitPassImpl::isEnabled() const
    {
        return getIScene().getBlitPass(m_blitPassHandle).isEnabled;
    }

    ramses_internal::BlitPassHandle BlitPassImpl::getBlitPassHandle() const
    {
        return m_blitPassHandle;
    }

    status_t BlitPassImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_blitPassHandle;
        assert(m_sourceRenderBufferImpl != nullptr);
        assert(m_destinationRenderBufferImpl != nullptr);
        outStream << serializationContext.getIDForObject(m_sourceRenderBufferImpl);
        outStream << serializationContext.getIDForObject(m_destinationRenderBufferImpl);

        return StatusOK;
    }

    status_t BlitPassImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_blitPassHandle;

        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_sourceRenderBufferImpl);
        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_destinationRenderBufferImpl);
        serializationContext.addForDependencyResolve(this);

        return StatusOK;
    }

    status_t BlitPassImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::resolveDeserializationDependencies(serializationContext));

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_sourceRenderBufferImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_destinationRenderBufferImpl);

        return StatusOK;
    }

    status_t BlitPassImpl::validate() const
    {
        status_t status = SceneObjectImpl::validate();

        const ramses_internal::BlitPass& blitPass = getIScene().getBlitPass(m_blitPassHandle);

        if (!getIScene().isRenderBufferAllocated(blitPass.sourceRenderBuffer))
            status = addValidationMessage(EValidationSeverity_Error, "blitpass references a deleted source render buffer");

        if (!getIScene().isRenderBufferAllocated(blitPass.destinationRenderBuffer))
            status = addValidationMessage(EValidationSeverity_Error, "blitpass references a deleted destination render buffer");

        return status;
    }

    const RenderBuffer& BlitPassImpl::getSourceRenderBuffer() const
    {
        assert(nullptr != m_sourceRenderBufferImpl);
        return RamsesObjectTypeUtils::ConvertTo<RenderBuffer>(m_sourceRenderBufferImpl->getRamsesObject());
    }

    const RenderBuffer& BlitPassImpl::getDestinationRenderBuffer() const
    {
        assert(nullptr != m_destinationRenderBufferImpl);
        return RamsesObjectTypeUtils::ConvertTo<RenderBuffer>(m_destinationRenderBufferImpl->getRamsesObject());
    }

    void BlitPassImpl::getBlittingRegion(uint32_t& sourceX, uint32_t& sourceY, uint32_t& destinationX, uint32_t& destinationY, uint32_t& width, uint32_t& height) const
    {
        const auto& blitPassInternal = getIScene().getBlitPass(m_blitPassHandle);
        sourceX = blitPassInternal.sourceRegion.x;
        sourceY = blitPassInternal.sourceRegion.y;
        destinationX = blitPassInternal.destinationRegion.x;
        destinationY = blitPassInternal.destinationRegion.y;
        width = blitPassInternal.sourceRegion.width;
        height = blitPassInternal.sourceRegion.height;
    }
}
