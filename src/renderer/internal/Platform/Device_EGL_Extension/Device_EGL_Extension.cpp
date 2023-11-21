//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Device_EGL_Extension/Device_EGL_Extension.h"
#include "internal/Core/Utils/LogMacros.h"
#include <drm_fourcc.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>

namespace ramses::internal
{
    Device_EGL_Extension::Device_EGL_Extension(Context_EGL& context, std::string_view renderNode)
        : m_resourceMapper(context.getResources())
        , m_eglExtensionProcs(context.getEglDisplay())
        , m_renderNode(renderNode)
    {
    }

    Device_EGL_Extension::~Device_EGL_Extension()
    {
        if(m_gbmDevice != nullptr)
        {
            LOG_INFO(CONTEXT_RENDERER, "Device_EGL_Extension::~Device_EGL_Extension(): Destroy GBM device");
            gbm_device_destroy(m_gbmDevice);
        }

        if(m_drmRenderNodeFD != -1)
        {
            LOG_INFO(CONTEXT_RENDERER, "Device_EGL_Extension::~Device_EGL_Extension(): Close render node FD");
            close(m_drmRenderNodeFD);
        }
    }

    bool Device_EGL_Extension::init()
    {
        if(!m_eglExtensionProcs.areDmabufExtensionsSupported())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_EGL_Extension::init(): DMA buffer EGL extensions not supported!");
            return false;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): false positive
        m_drmRenderNodeFD = open(m_renderNode.c_str(), O_RDWR);
        if (m_drmRenderNodeFD < 0)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_EGL_Extension::init(): failed to open render node \"{}\"!", m_renderNode);
            return false;
        }

        m_gbmDevice = gbm_create_device(m_drmRenderNodeFD);
        if (m_gbmDevice == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_EGL_Extension::init(): failed to create GBM device!");
            return false;
        }

        LOG_INFO(CONTEXT_RENDERER, "Device_EGL_Extension::init(): init successful");
        return true;
    }

    DeviceResourceHandle Device_EGL_Extension::createDmaRenderBuffer(uint32_t width, uint32_t height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers)
    {
        assert(m_gbmDevice != nullptr);
        assert(m_eglExtensionProcs.areDmabufExtensionsSupported());

        const uint32_t bufferFormat = fourccFormat.getValue();
        const uint32_t bufferUsage = usageFlags.getValue();

        const bool isBufferFormatSupported = (gbm_device_is_format_supported(m_gbmDevice, bufferFormat, bufferUsage) != 0);
        if(!isBufferFormatSupported)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_EGL_Extension::createDmaRenderBuffer(): buffer format \"{}\" is not supported! Buffer creation will probably fail!", bufferFormat);
        }
        else
            LOG_INFO(CONTEXT_RENDERER, "Device_EGL_Extension::createDmaRenderBuffer(): buffer format \"{}\" is supported", bufferFormat);

        const auto gbmBufferObject = gbm_bo_create(m_gbmDevice, width, height, bufferFormat, bufferUsage);
        if(gbmBufferObject == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_EGL_Extension::createDmaRenderBuffer(): failed to create GBM buffer object!");
            return {};
        }

        const auto bufferStride = gbm_bo_get_stride(gbmBufferObject);
        const auto bufferFD = gbm_bo_get_fd(gbmBufferObject);
        if (bufferFD < 0)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_EGL_Extension::createDmaRenderBuffer(): failed to get FD from GBM buffer object!");
            return {};
        }

        std::vector<EGLint> eglImageCreationAttribs;
        eglImageCreationAttribs.push_back(EGL_WIDTH);
        eglImageCreationAttribs.push_back(static_cast<EGLint>(width));
        eglImageCreationAttribs.push_back(EGL_HEIGHT);
        eglImageCreationAttribs.push_back(static_cast<EGLint>(height));
        eglImageCreationAttribs.push_back(EGL_LINUX_DRM_FOURCC_EXT);
        eglImageCreationAttribs.push_back(static_cast<EGLint>(bufferFormat));

        eglImageCreationAttribs.push_back(EGL_DMA_BUF_PLANE0_FD_EXT);
        eglImageCreationAttribs.push_back(bufferFD);
        eglImageCreationAttribs.push_back(EGL_DMA_BUF_PLANE0_OFFSET_EXT);
        eglImageCreationAttribs.push_back(0);
        eglImageCreationAttribs.push_back(EGL_DMA_BUF_PLANE0_PITCH_EXT);
        eglImageCreationAttribs.push_back(static_cast<EGLint>(bufferStride));

        if(modifiers.isValid() && modifiers.getValue() != DRM_FORMAT_MOD_INVALID)
        {
            const uint64_t bufferModifiers = modifiers.getValue();
            const auto bufferModifiersLowBytes   = static_cast<EGLint>(bufferModifiers & 0xffffffff);
            const auto bufferModifiersHighBytes  = static_cast<EGLint>(bufferModifiers >> 32);

            eglImageCreationAttribs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT);
            eglImageCreationAttribs.push_back(bufferModifiersLowBytes);
            eglImageCreationAttribs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT);
            eglImageCreationAttribs.push_back(bufferModifiersHighBytes);
        }

        eglImageCreationAttribs.push_back(EGL_NONE);

        const auto eglImage = m_eglExtensionProcs.eglCreateImageKHR(EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, eglImageCreationAttribs.data());
        if (eglImage == EGL_NO_IMAGE)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_EGL_Extension::createDmaRenderBuffer(): failed to create EGL Image, EGL error: {}! [width: {}, height: {}, format: {}, usage: {}, modifiers: {}, FD :{}, stride: {}]", eglGetError(), width, height, bufferFormat, bufferUsage, modifiers.getValue(), bufferFD, bufferStride);
            close(bufferFD);
            gbm_bo_destroy(gbmBufferObject);
            return {};
        }

        LOG_INFO(CONTEXT_RENDERER, "Device_EGL_Extension::createDmaRenderBuffer(): DMA render buffer created succesfully [width: {}, height: {}, stride: {}, format: {}, FD :{}, EGL image :{}]", width, height, bufferStride, bufferFormat, bufferFD, eglImage);

        GLuint glTexAddress = 0u;
        glGenTextures(1, &glTexAddress);
        glBindTexture(GL_TEXTURE_2D, glTexAddress);
        m_eglExtensionProcs.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);

        return m_resourceMapper.registerResource(std::make_unique<DmaRenderBufferGpuResource>(glTexAddress, width, height, eglImage, gbmBufferObject, bufferFD, bufferStride));
    }

    int Device_EGL_Extension::getDmaRenderBufferFD(DeviceResourceHandle handle)
    {
        const auto& resource = m_resourceMapper.getResourceAs<DmaRenderBufferGpuResource>(handle);
        return resource.getFD();
    }

    uint32_t Device_EGL_Extension::getDmaRenderBufferStride(DeviceResourceHandle handle)
    {
        const auto& resource = m_resourceMapper.getResourceAs<DmaRenderBufferGpuResource>(handle);
        return resource.getStride();
    }

    void Device_EGL_Extension::destroyDmaRenderBuffer(DeviceResourceHandle handle)
    {
        const auto& resource = m_resourceMapper.getResourceAs<DmaRenderBufferGpuResource>(handle);

        LOG_INFO(CONTEXT_RENDERER, "Device_EGL_Extension::destroyDmaRenderBuffer(): destroy DMA render buffer [FD :{}, EGL image :{}]", resource.getFD(), resource.getEGLImage());
        m_eglExtensionProcs.eglDestroyImageKHR(resource.getEGLImage());
        close(resource.getFD());
        gbm_bo_destroy(resource.getGBMBufferObject());

        m_resourceMapper.deleteResource(handle);
    }
}
