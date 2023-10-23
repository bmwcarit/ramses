//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/BlitPassImpl.h"
#include "ramses/client/RenderBuffer.h"
#include "impl/RenderBufferImpl.h"
#include "impl/SerializationContext.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/SceneAPI/BlitPass.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    BlitPassImpl::BlitPassImpl(SceneImpl& scene, std::string_view blitpassName)
        : SceneObjectImpl(scene, ERamsesObjectType::BlitPass, blitpassName)
    {
    }

    BlitPassImpl::~BlitPassImpl() = default;

    void BlitPassImpl::initializeFrameworkData(const RenderBufferImpl& sourceRenderBuffer, const RenderBufferImpl& destinationRenderBuffer)
    {
        assert(!m_blitPassHandle.isValid());
        m_sourceRenderBufferImpl = &sourceRenderBuffer;
        m_destinationRenderBufferImpl = &destinationRenderBuffer;

        const RenderBufferHandle sourceRenderBufferHandle = sourceRenderBuffer.getRenderBufferHandle();
        const RenderBufferHandle destinationRenderBufferHandle = destinationRenderBuffer.getRenderBufferHandle();
        m_blitPassHandle = getIScene().allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, {});

        setBlittingRegion(0u, 0u, 0u, 0u, sourceRenderBuffer.getWidth(), sourceRenderBuffer.getHeight());
    }

    void BlitPassImpl::deinitializeFrameworkData()
    {
        assert(m_blitPassHandle.isValid());
        getIScene().releaseBlitPass(m_blitPassHandle);
        m_blitPassHandle = BlitPassHandle::Invalid();
    }

    bool BlitPassImpl::setBlittingRegion(uint32_t sourceX, uint32_t sourceY, uint32_t destinationX, uint32_t destinationY, uint32_t width, uint32_t height)
    {
        const ramses::internal::BlitPass& blitPass = getIScene().getBlitPass(m_blitPassHandle);
        const ramses::internal::RenderBuffer& renderBufferSrc = getIScene().getRenderBuffer(blitPass.sourceRenderBuffer);
        const ramses::internal::RenderBuffer& renderBufferDst = getIScene().getRenderBuffer(blitPass.destinationRenderBuffer);
        if (sourceX + width > renderBufferSrc.width || sourceY + height > renderBufferSrc.height)
        {
            getErrorReporting().set("BlitPass::setBlittingRegion failed - invalid source region", *this);
            return false;
        }

        if (destinationX + width > renderBufferDst.width || destinationY + height > renderBufferDst.height)
        {
            getErrorReporting().set("BlitPass::setBlittingRegion failed - invalid destination region", *this);
            return false;
        }

        const PixelRectangle sourceRegion = {
            sourceX, sourceY, static_cast<int32_t>(width), static_cast<int32_t>(height)
        };

        const PixelRectangle destinationRegion = {
            destinationX, destinationY, static_cast<int32_t>(width), static_cast<int32_t>(height)
        };

        getIScene().setBlitPassRegions(m_blitPassHandle, sourceRegion, destinationRegion);

        return true;
    }

    bool BlitPassImpl::setRenderOrder(int32_t renderOrder)
    {
        getIScene().setBlitPassRenderOrder(m_blitPassHandle, renderOrder);

        return true;
    }

    int32_t BlitPassImpl::getRenderOrder() const
    {
        return getIScene().getBlitPass(m_blitPassHandle).renderOrder;
    }

    bool BlitPassImpl::setEnabled(bool isEnabeld)
    {
        getIScene().setBlitPassEnabled(m_blitPassHandle, isEnabeld);

        return true;
    }

    bool BlitPassImpl::isEnabled() const
    {
        return getIScene().getBlitPass(m_blitPassHandle).isEnabled;
    }

    BlitPassHandle BlitPassImpl::getBlitPassHandle() const
    {
        return m_blitPassHandle;
    }

    bool BlitPassImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_blitPassHandle;
        assert(m_sourceRenderBufferImpl != nullptr);
        assert(m_destinationRenderBufferImpl != nullptr);
        outStream << serializationContext.getIDForObject(m_sourceRenderBufferImpl);
        outStream << serializationContext.getIDForObject(m_destinationRenderBufferImpl);

        return true;
    }

    bool BlitPassImpl::deserialize(IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_blitPassHandle;

        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_sourceRenderBufferImpl);
        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_destinationRenderBufferImpl);
        serializationContext.addForDependencyResolve(this);

        return true;
    }

    bool BlitPassImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_sourceRenderBufferImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_destinationRenderBufferImpl);

        return true;
    }

    void BlitPassImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        const ramses::internal::BlitPass& blitPass = getIScene().getBlitPass(m_blitPassHandle);

        if (!getIScene().isRenderBufferAllocated(blitPass.sourceRenderBuffer))
            report.add(EIssueType::Error, "blitpass references a deleted source render buffer", &getRamsesObject());

        if (!getIScene().isRenderBufferAllocated(blitPass.destinationRenderBuffer))
            report.add(EIssueType::Error, "blitpass references a deleted destination render buffer", &getRamsesObject());
    }

    const ramses::RenderBuffer& BlitPassImpl::getSourceRenderBuffer() const
    {
        assert(nullptr != m_sourceRenderBufferImpl);
        return RamsesObjectTypeUtils::ConvertTo<ramses::RenderBuffer>(m_sourceRenderBufferImpl->getRamsesObject());
    }

    const ramses::RenderBuffer& BlitPassImpl::getDestinationRenderBuffer() const
    {
        assert(nullptr != m_destinationRenderBufferImpl);
        return RamsesObjectTypeUtils::ConvertTo<ramses::RenderBuffer>(m_destinationRenderBufferImpl->getRamsesObject());
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
