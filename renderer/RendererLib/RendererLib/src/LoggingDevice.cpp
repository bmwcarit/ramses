//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/LoggingDevice.h"
#include "RendererLib/ConstantLogger.h"
#include "SceneAPI/RenderBuffer.h"
#include "Resource/EffectResource.h"

namespace ramses_internal
{
    LoggingDevice::LoggingDevice(const IDevice& deviceDelegate, RendererLogContext& context)
        : m_deviceDelegate(deviceDelegate)
        , m_logContext(context)
    {
    }

    EDeviceTypeId LoggingDevice::getDeviceTypeId() const
    {
        return EDeviceTypeId_INVALID;
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Matrix22f* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Matrix33f* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Matrix44f* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Vector4i* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Vector3i* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Vector2i* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Int32* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Vector4* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Vector3* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Vector2* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::setConstant(DataFieldHandle field, UInt32 count, const Float* value)
    {
        ConstantLogger::LogValueArray(field, value, count, m_logContext);
    }

    void LoggingDevice::colorMask(Bool r, Bool g, Bool b, Bool a)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set color write mask: [" << r << "; " << g << "; " << b << "; " << a << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::clearColor(const Vector4& clearColor)
    {
        m_logContext << "clear color to [" << clearColor.x << "; " << clearColor.y << "; " << clearColor.z << "; " << clearColor.w << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::blendOperations(EBlendOperation colorOperation, EBlendOperation alphaOperation)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set blend ops: [color: " << EnumToString(colorOperation) << "; alpha: " << EnumToString(alphaOperation) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::blendFactors(EBlendFactor sourceColor, EBlendFactor destinationColor, EBlendFactor sourceAlpha, EBlendFactor destinationAlpha)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set blend factors to ["
                << EnumToString(sourceColor) << "; "
                << EnumToString(destinationColor) << "; "
                << EnumToString(sourceAlpha) << "; "
                << EnumToString(destinationAlpha) << " ]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::cullMode(ECullMode mode)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set cull mode: [mode: " << EnumToString(mode) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::depthFunc(EDepthFunc func)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set depth func: [func: " << EnumToString(func) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::depthWrite(EDepthWrite flag)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set depth write: [flag: " << EnumToString(flag) << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::scissorTest(EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set scissor test: [flag: " << EnumToString(flag) << "; x: " << region.x << "; y: " << region.y << "; width: " << region.width << "; height: " << region.height << "]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::stencilFunc(EStencilFunc func, UInt8 ref, UInt8 mask)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set stencil func: [func: " << EnumToString(func) << "; ref: " << ref << "; mask: " << mask << " ]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::stencilOp(EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "set stencil operation: [sfail: " << EnumToString(sfail) << "; dpfail: " << EnumToString(dpfail) << "; dppass: " << EnumToString(dppass) << " ]" << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::setViewport(UInt32 x, UInt32 y, UInt32 width, UInt32 height)
    {
        m_logContext << "set viewport: [x: " << x << "; y: " << y << "; w: " << width << "; h: " << height << " ]" <<RendererLogContext::NewLine;
    }

    void LoggingDevice::drawMode(EDrawMode mode)
    {
        m_logContext << "set draw mode: " << EnumToString(mode) << RendererLogContext::NewLine;
    }

    void LoggingDevice::setTextureSampling(DataFieldHandle field, EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel)
    {
        m_logContext << "set texture sampling for texture " << field << " : [wrapU: " << EnumToString(wrapU) << "; wrapV: " << EnumToString(wrapV) << "; wrapR: " << EnumToString(wrapR) << "; min sampling: " << EnumToString(minSampling) << "; mag sampling: " << EnumToString(magSampling) << "; anisotropyLevel: " << anisotropyLevel << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::allocateVertexBuffer(EDataType dataType, UInt32 sizeInBytes)
    {
        m_logContext << "allocate vertex buffer [type: " << EnumToString(dataType) << " size: " << sizeInBytes << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::uploadVertexBufferData(DeviceResourceHandle handle, const Byte*, UInt32 dataSize)
    {
        m_logContext << "upload vertex buffer data [device handle: " << handle << " size: " << dataSize << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::deleteVertexBuffer(DeviceResourceHandle handle)
    {
        m_logContext << "delete vertex buffer [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateVertexBuffer(DeviceResourceHandle handle, DataFieldHandle field, UInt32 instancingDivisor)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "activate vertex buffer [handle: " << handle << "]";

            if (field.isValid())
            {
                m_logContext << " for input [" << field << "]";
            }

            m_logContext << " [divisor: " << instancingDivisor << "]";

            m_logContext << RendererLogContext::NewLine;
        }
    }

    DeviceResourceHandle LoggingDevice::allocateIndexBuffer(EDataType dataType, UInt32 sizeInBytes)
    {
        m_logContext << "allocate index buffer [type: " << EnumToString(dataType) << " size: " << sizeInBytes << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::uploadIndexBufferData(DeviceResourceHandle handle, const Byte*, UInt32 dataSize)
    {
        m_logContext << "upload index buffer data [device handle: " << handle << " size: " << dataSize << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::deleteIndexBuffer(DeviceResourceHandle handle)
    {
        m_logContext << "delete index buffer [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateIndexBuffer(DeviceResourceHandle handle)
    {
        logResourceActivation("index buffer", handle, DataFieldHandle::Invalid());
    }

    DeviceResourceHandle LoggingDevice::uploadShader(const EffectResource& effect)
    {
        m_logContext << "upload shader " << effect.getName() << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle LoggingDevice::uploadBinaryShader(const EffectResource& effect, const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, UInt32 binaryShaderFormat)
    {
        UNUSED(binaryShaderData);

        m_logContext << "upload binary shader " << effect.getName() << " size: " << binaryShaderDataSize << " format: "  << binaryShaderFormat << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    Bool LoggingDevice::getBinaryShader(DeviceResourceHandle handle, UInt8Vector& binaryShader, UInt32& binaryShaderFormat)
    {
        UNUSED(binaryShader);
        UNUSED(binaryShaderFormat);

        m_logContext << "get shader binary for shader [handle: " << handle << "]" << RendererLogContext::NewLine;
        return false;
    }

    void LoggingDevice::deleteShader(DeviceResourceHandle handle)
    {
        m_logContext << "delete shader [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateShader(DeviceResourceHandle handle)
    {
        m_logContext << "activate shader [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::allocateTexture2D(UInt32 width, UInt32 height, ETextureFormat format, UInt32 mipLevelCount, UInt32 totalSizeInBytes)
    {
        m_logContext << "allocate texture2d [ (w,h):(" << width << "," << height << ") mipLevelCount:" << mipLevelCount << " format:" << EnumToString(format) << " totalSizeInBytes:" << totalSizeInBytes << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle LoggingDevice::allocateTexture3D(UInt32 width, UInt32 height, UInt32 depth, ETextureFormat format, UInt32 mipLevelCount, UInt32)
    {
        m_logContext << "allocate texture3d [ (w,h,d):(" << width << "," << height << "," << depth << ") mipLevelCount:" << mipLevelCount << " format:" << EnumToString(format) << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle LoggingDevice::allocateTextureCube(UInt32 faceSize, ETextureFormat format, UInt32 mipLevelCount, UInt32)
    {
        m_logContext << "allocate textureCube [ faceSize:" << faceSize << " mipLevelCount:" << mipLevelCount << " format:" << EnumToString(format) << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::bindTexture(DeviceResourceHandle handle)
    {
        m_logContext << "bind texture [handle:" << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::generateMipmaps(DeviceResourceHandle handle)
    {
        m_logContext << "generate mipmaps for texture [handle:" << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::uploadTextureData(DeviceResourceHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 z, UInt32 width, UInt32 height, UInt32 depth, const Byte*, UInt32 dataSize)
    {
        m_logContext << "update texture data [handle:" << handle << " mipLevel:" << mipLevel << " (x,y,z):(" << x << "," << y << "," << z << ") (w,h,d):(" << width << "," << height << "," << depth << ") dataSize:" << dataSize << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::uploadStreamTexture2D(DeviceResourceHandle handle, UInt32 width, UInt32 height, ETextureFormat, const UInt8*)
    {
        m_logContext << "upload stream texture2d [textureHandle: " << handle << " (w,h):(" << width << "," << height << ")]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::deleteTexture(DeviceResourceHandle handle)
    {
        m_logContext << "delete texture [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateTexture(DeviceResourceHandle handle, DataFieldHandle field)
    {
        logResourceActivation("texture", handle, field);
    }

    DeviceResourceHandle LoggingDevice::uploadRenderBuffer(const RenderBuffer& renderBuffer)
    {
        m_logContext << "upload render buffer [type: " << EnumToString(renderBuffer.type) << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::deleteRenderBuffer(DeviceResourceHandle handle)
    {
        m_logContext << "delete render buffer [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    DeviceResourceHandle LoggingDevice::uploadTextureSampler(EWrapMethod wrapU, EWrapMethod wrapV, EWrapMethod wrapR, ESamplingMethod minSampling, ESamplingMethod magSampling, UInt32 anisotropyLevel)
    {
        m_logContext << "upload texture sampler: [wrapU: " << EnumToString(wrapU) << "; wrapV: " << EnumToString(wrapV) << "; wrapR: " << EnumToString(wrapR) << "; min sampling: " << EnumToString(minSampling) << "; mag sampling: " << EnumToString(magSampling) << "; anisotropyLevel: " << anisotropyLevel << "]" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::deleteTextureSampler(DeviceResourceHandle handle)
    {
        m_logContext << "delete texture sampler [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::activateTextureSampler(DeviceResourceHandle handle, DataFieldHandle field)
    {
        logResourceActivation("textureSampler", handle, field);
    }

    DeviceResourceHandle LoggingDevice::getFramebufferRenderTarget() const
    {
        return DeviceResourceHandle::Invalid();
    }

    DeviceResourceHandle    LoggingDevice::uploadRenderTarget(const DeviceHandleVector& renderBuffers)
    {
        m_logContext << "creating render target with " << renderBuffers.size() << " renderbuffers'" << RendererLogContext::NewLine;
        return DeviceResourceHandle::Invalid();
    }

    void LoggingDevice::activateRenderTarget(DeviceResourceHandle handle)
    {
        m_logContext << "activate rendertarget [Handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::deleteRenderTarget(DeviceResourceHandle handle)
    {
        m_logContext << "delete render target [handle: " << handle << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::blitRenderTargets(DeviceResourceHandle rtSrc, DeviceResourceHandle rtDst, const PixelRectangle& /*srcRect*/, const PixelRectangle& /*dstRect*/, Bool /*colorOnly*/)
    {
        m_logContext << "blit render pass [RT src device handle:  " << rtSrc << ", RT dst device handle: " << rtDst << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount)
    {
        m_logContext << "draw " << instanceCount << " instances with " << elementCount << " indexed vertices starting from " << startOffset << RendererLogContext::NewLine;
    }

    void LoggingDevice::drawTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount)
    {
        m_logContext << "draw " << instanceCount << " instances with " << elementCount << " vertices starting from " << startOffset << RendererLogContext::NewLine;
    }

    void LoggingDevice::clear(UInt32 clearFlags)
    {
        m_logContext << "clear buffer [flags: " << clearFlags << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::pairRenderTargetsForDoubleBuffering(DeviceResourceHandle renderTargets[2], DeviceResourceHandle colorBuffers[2])
    {
        m_logContext << "pair render targets [render targets: (" << renderTargets[0] << ", " << renderTargets[1] << "), render buffers: (" << colorBuffers[0] << ", " << colorBuffers[1] << ")]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::unpairRenderTargets(DeviceResourceHandle renderTarget)
    {
        m_logContext << "unpair render targets [render target: " << renderTarget << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::swapDoubleBufferedRenderTarget(DeviceResourceHandle renderTarget)
    {
        m_logContext << "swap render targets [render target: " << renderTarget << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::readPixels(UInt8* /*buffer*/, UInt32 /*x*/, UInt32 /*y*/, UInt32 /*width*/, UInt32 /*height*/)
    {
    }

    UInt32 LoggingDevice::getTotalGpuMemoryUsageInKB() const
    {
        return m_deviceDelegate.getTotalGpuMemoryUsageInKB();
    }

    void LoggingDevice::logResourceActivation(const String& resourceTypeName, DeviceResourceHandle handle, DataFieldHandle field)
    {
        if (m_logContext.isLogLevelFlagEnabled(ERendererLogLevelFlag_Details))
        {
            m_logContext << "activate " << resourceTypeName << " [handle: " << handle << "]";

            if (field.isValid())
            {
                m_logContext << " for input [" << field << "]";
            }

            m_logContext << RendererLogContext::NewLine;
        }
    }

    void LoggingDevice::clearDepth(Float d)
    {
        m_logContext << "clear depth to [ " << d << "]" << RendererLogContext::NewLine;
    }

    void LoggingDevice::clearStencil(Int32 s)
    {
        m_logContext << "clear stencil to [ " << s << "]" << RendererLogContext::NewLine;
    }

    Int32 LoggingDevice::getTextureAddress(DeviceResourceHandle) const
    {
        return 0;
    }

    void LoggingDevice::validateDeviceStatusHealthy() const
    {
    }

    Bool LoggingDevice::isDeviceStatusHealthy() const
    {
        return true;
    }

    ramses_internal::UInt32 LoggingDevice::getDrawCallCount() const
    {
        return 0;
    }

    void LoggingDevice::resetDrawCallCount()
    {
    }

    void LoggingDevice::finish()
    {
    }

}
