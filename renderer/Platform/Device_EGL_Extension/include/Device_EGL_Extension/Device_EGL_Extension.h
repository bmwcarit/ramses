//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEVICE_EGL_EXTENSION_H
#define RAMSES_DEVICE_EGL_EXTENSION_H

#include "RendererAPI/IDeviceExtension.h"
#include "Context_EGL/Context_EGL.h"
#include "Platform_Base/RenderBufferGPUResource.h"
#include "WaylandEGLExtensionProcs/WaylandEGLExtensionProcs.h"

struct gbm_device;
struct gbm_bo;

namespace ramses_internal
{
    class DmaRenderBufferGpuResource: public RenderBufferGPUResource
    {
    public:
        DmaRenderBufferGpuResource(UInt32 gpuAddress, UInt32 width, UInt32 height, EGLImage eglImage, gbm_bo* gbmBufferObject, int fd, UInt32 stride)
            : RenderBufferGPUResource(gpuAddress, width, height, ERenderBufferType_ColorBuffer, ETextureFormat::RGBA8, 0u, ERenderBufferAccessMode_ReadWrite)
            , m_eglImage(eglImage)
            , m_gbmBufferObject(gbmBufferObject)
            , m_fd(fd)
            , m_stride(stride)
        {
        }

        [[nodiscard]] EGLImage getEGLImage() const
        {
            return m_eglImage;
        }

        [[nodiscard]] gbm_bo* getGBMBufferObject() const
        {
            return m_gbmBufferObject;
        }

        [[nodiscard]] int getFD() const
        {
            return m_fd;
        }

        [[nodiscard]] UInt32 getStride() const
        {
            return m_stride;
        }

    private:
        EGLImage m_eglImage = EGL_NO_IMAGE;
        gbm_bo* m_gbmBufferObject = nullptr;
        int m_fd = -1;
        UInt32 m_stride = 0u;
    };

    class Device_EGL_Extension: public IDeviceExtension
    {
    public:
        explicit Device_EGL_Extension(Context_EGL& context, std::string_view renderNode);
        ~Device_EGL_Extension() override;

        bool init();

        DeviceResourceHandle    createDmaRenderBuffer       (uint32_t width, uint32_t height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers) override;
        int                     getDmaRenderBufferFD        (DeviceResourceHandle handle) override;
        uint32_t                getDmaRenderBufferStride    (DeviceResourceHandle handle) override;
        void                    destroyDmaRenderBuffer      (DeviceResourceHandle handle) override;

    private:
        DeviceResourceMapper& m_resourceMapper;
        WaylandEGLExtensionProcs m_eglExtensionProcs;
        const String m_renderNode;

        // GBM (Generic Buffer Manager) lib provides an API for platform abstracted creation of buffers
        // that could be used by EGL and GL for rendering.
        //
        // To use GBM API a "render node" must be provided, which specifies where to load the DRM driver's
        // render node that supports GBM.
        //
        // A "GBM device" object is created using the provided "render node". A GBM device provides the means for creating GBM buffer
        // objects, which are used as follows:
        //  * The GBM buffer objects are imported to EGL images, which are then
        //    used as textures. Those textures are used as color buffer attachment for offscreen buffers
        //    (OpenGL FBOs) where RAMSES scenes get rendrered.
        //  * The GBM buffer objects provide a file descriptor that can be mapped to CPU memory.
        //    The platform must have the right support for this mapping to succeed. If the mapping
        //    succeeds it is possible to run any CPU-side algorithms on that memory.
        //
        // When both usage modes are combined together this allows uses cases that require
        // both CPU algorithms and GPU rendering pipeline operations.
        //
        // For more information: https://github.com/robclark/libgbm/blob/master/gbm.h
        int m_drmRenderNodeFD = -1;
        gbm_device* m_gbmDevice = nullptr;
    };
}

#endif
