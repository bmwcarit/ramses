//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <assert.h>
#include <functional>

#include <drm_fourcc.h>

#include "linux-dmabuf-unstable-v1-server-protocol.h"
#include "TextureUploadingAdapter_Wayland/TextureUploadingAdapter_Wayland.h"
#include "TextureUploadingAdapter_Wayland/LinuxDmabuf.h"
#include "RendererAPI/IDevice.h"
#include "Utils/LogMacros.h"
#include "Utils/Warnings.h"

namespace ramses_internal
{
    TextureUploadingAdapter_Wayland::TextureUploadingAdapter_Wayland(IDevice& device, wl_display* waylandWindowDisplay, wl_display* embeddedCompositingDisplay)
        : TextureUploadingAdapter_Base(device)
        , m_waylandEglExtensionProcs(waylandWindowDisplay)
        , m_embeddedCompositingDisplay(embeddedCompositingDisplay)
    {
        if (m_waylandEglExtensionProcs.areExtensionsSupported())
        {
            if(EGL_TRUE != m_waylandEglExtensionProcs.eglBindWaylandDisplayWL(m_embeddedCompositingDisplay))
            {
                LOG_ERROR(CONTEXT_RENDERER, "TextureUploadingAdapter_Wayland::TextureUploadingAdapter_Wayland failed to bind wayland display!");
            }
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "TextureUploadingAdapter_Wayland::TextureUploadingAdapter_Wayland: Wayland-EGL extensions not supported");
        }
    }

    TextureUploadingAdapter_Wayland::~TextureUploadingAdapter_Wayland()
    {
        if (m_waylandEglExtensionProcs.areExtensionsSupported())
        {
            if(EGL_TRUE != m_waylandEglExtensionProcs.eglUnbindWaylandDisplayWL(m_embeddedCompositingDisplay))
            {
                LOG_ERROR(CONTEXT_RENDERER, "TextureUploadingAdapter_Wayland::~TextureUploadingAdapter_Wayland failed to unbind wayland display!");
            }
        }

        for (auto entry: m_dmabufEglImagesMap)
        {
            entry.first->clearDestroyCallback();
            delete entry.second;
        }
    }

    void TextureUploadingAdapter_Wayland::uploadTextureFromWaylandResource(DeviceResourceHandle textureHandle, EGLClientBuffer bufferResource)
    {
        if (m_waylandEglExtensionProcs.areExtensionsSupported())
        {
            const GLuint texID = m_device.getTextureAddress(textureHandle);
            glBindTexture(GL_TEXTURE_2D, texID);

            const EGLImage eglImage = m_waylandEglExtensionProcs.eglCreateImageKHR(nullptr, EGL_WAYLAND_BUFFER_WL, bufferResource, nullptr);
            assert(EGL_NO_IMAGE != eglImage);
            m_waylandEglExtensionProcs.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);
            m_waylandEglExtensionProcs.eglDestroyImageKHR(eglImage);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "TextureUploadingAdapter_Wayland::uploadTextureFromWaylandResource: wayland-egl extension not supported!");
        }
    }

    void TextureUploadingAdapter_Wayland::handleDmabufDestroy(LinuxDmabufBufferData* dmabuf)
    {
        auto iter = m_dmabufEglImagesMap.find(dmabuf);
        assert(iter != m_dmabufEglImagesMap.end());

        assert(nullptr != iter->second);
        delete iter->second;

        m_dmabufEglImagesMap.erase(iter);
    }

    TextureUploadingAdapter_Wayland::DmabufEglImage* TextureUploadingAdapter_Wayland::importDmabufToEglImage(LinuxDmabufBufferData* dmabuf)
    {
        assert(m_waylandEglExtensionProcs.areDmabufExtensionsSupported());

        // First, import the dmabuf if this is the first time that the given
        // wl_buffer has been attached to a surface.

        for (unsigned int i = 0; i < dmabuf->getNumPlanes(); ++i)
        {
            PUSH_DISABLE_C_STYLE_CAST_WARNING

            // If modifiers are used, check that EGL doesn't supports them. For now, we haven't
            // implemented that support.
            if (dmabuf->getModifier(i) != DRM_FORMAT_MOD_INVALID)
            {
                return nullptr;
            }

            POP_DISABLE_C_STYLE_CAST_WARNING

            // Modifiers have to be the same across all planes
            if (dmabuf->getModifier(i) != dmabuf->getModifier(0))
            {
                return nullptr;
            }
        }

        // So far, the only value of the 'flags' parameter to zwp_linux_buffer_params_v1::create() that
        // we support is for inverting along the Y axis.
        if (dmabuf->getFlags() & ~ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT)
        {
            return nullptr;
        }

        EGLint attribs[50];
        int atti = 0;

        attribs[atti++] = EGL_WIDTH;
        attribs[atti++] = dmabuf->getWidth();
        attribs[atti++] = EGL_HEIGHT;
        attribs[atti++] = dmabuf->getHeight();
        attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
        attribs[atti++] = dmabuf->getFormat();

        PUSH_DISABLE_C_STYLE_CAST_WARNING
        bool hasModifier = dmabuf->getModifier(0) != DRM_FORMAT_MOD_INVALID;
        POP_DISABLE_C_STYLE_CAST_WARNING

        // TODO: no support for dmabuf modifiers
        if (hasModifier)
        {
            return nullptr;
        }

        // First plane
        if (dmabuf->getNumPlanes() > 0)
        {
            attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
            attribs[atti++] = dmabuf->getFd(0);
            attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
            attribs[atti++] = dmabuf->getOffset(0);
            attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
            attribs[atti++] = dmabuf->getStride(0);

            if (hasModifier)
            {
                attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
                attribs[atti++] = dmabuf->getModifier(0) & 0xffffffff;
                attribs[atti++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
                attribs[atti++] = dmabuf->getModifier(0) >> 32;
            }
        }

        // Second plane
        if (dmabuf->getNumPlanes() > 1)
        {
            attribs[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
            attribs[atti++] = dmabuf->getFd(1);
            attribs[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
            attribs[atti++] = dmabuf->getOffset(1);
            attribs[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
            attribs[atti++] = dmabuf->getStride(1);

            if (hasModifier)
            {
                attribs[atti++] = EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT;
                attribs[atti++] = dmabuf->getModifier(1) & 0xffffffff;
                attribs[atti++] = EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT;
                attribs[atti++] = dmabuf->getModifier(1) >> 32;
            }
        }

        // Third plane
        if (dmabuf->getNumPlanes() > 2)
        {
            attribs[atti++] = EGL_DMA_BUF_PLANE2_FD_EXT;
            attribs[atti++] = dmabuf->getFd(2);
            attribs[atti++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
            attribs[atti++] = dmabuf->getOffset(2);
            attribs[atti++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
            attribs[atti++] = dmabuf->getStride(2);

            if (hasModifier)
            {
                attribs[atti++] = EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT;
                attribs[atti++] = dmabuf->getModifier(2) & 0xffffffff;
                attribs[atti++] = EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT;
                attribs[atti++] = dmabuf->getModifier(2) >> 32;
            }
        }

        // Fourth plane
        if (dmabuf->getNumPlanes() > 3)
        {
            attribs[atti++] = EGL_DMA_BUF_PLANE3_FD_EXT;
            attribs[atti++] = dmabuf->getFd(3);
            attribs[atti++] = EGL_DMA_BUF_PLANE3_OFFSET_EXT;
            attribs[atti++] = dmabuf->getOffset(3);
            attribs[atti++] = EGL_DMA_BUF_PLANE3_PITCH_EXT;
            attribs[atti++] = dmabuf->getStride(3);

            if (hasModifier)
            {
                attribs[atti++] = EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT;
                attribs[atti++] = dmabuf->getModifier(3) & 0xffffffff;
                attribs[atti++] = EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT;
                attribs[atti++] = dmabuf->getModifier(3) >> 32;
            }
        }

        attribs[atti++] = EGL_NONE;

        const EGLImage eglImage = m_waylandEglExtensionProcs.eglCreateImageKHR(NULL, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
        assert(EGL_NO_IMAGE != eglImage);
        if (EGL_NO_IMAGE == eglImage)
        {
            return nullptr;
        }

        return new DmabufEglImage(*this, eglImage, GL_TEXTURE_EXTERNAL_OES);
    }

    void TextureUploadingAdapter_Wayland::uploadTextureFromLinuxDmabuf(DeviceResourceHandle textureHandle, LinuxDmabufBufferData* dmabuf)
    {
        UNUSED(textureHandle)

        DmabufEglImage* image;
        auto iter = m_dmabufEglImagesMap.find(dmabuf);

        // Lazily import it
        if (iter == m_dmabufEglImagesMap.end())
        {
            image = importDmabufToEglImage(dmabuf);
            assert(nullptr != image);
            dmabuf->setDestroyCallback(std::bind(&TextureUploadingAdapter_Wayland::handleDmabufDestroy, this, std::placeholders::_1));
            m_dmabufEglImagesMap[dmabuf] = image;
        }
        else
        {
            image = iter->second;
        }

        // Now refresh the texture from it
        const GLuint texID = m_device.getTextureAddress(textureHandle);
        glBindTexture(GL_TEXTURE_2D, texID);
        m_waylandEglExtensionProcs.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image->getEglImage());
    }

    TextureUploadingAdapter_Wayland::DmabufEglImage::DmabufEglImage(TextureUploadingAdapter_Wayland& parent, EGLImage eglImage, GLenum textureTarget)
        : m_parent(parent)
        , m_eglImage(eglImage)
        , m_textureTarget(textureTarget)
    {
    }

    TextureUploadingAdapter_Wayland::DmabufEglImage::~DmabufEglImage()
    {
        if (EGL_NO_IMAGE != m_eglImage)
        {
            m_parent.m_waylandEglExtensionProcs.eglDestroyImageKHR(m_eglImage);
        }
    }

    EGLImage TextureUploadingAdapter_Wayland::DmabufEglImage::getEglImage() const
    {
        return m_eglImage;
    }

    GLenum TextureUploadingAdapter_Wayland::DmabufEglImage::getTextureTarget() const
    {
        return m_textureTarget;
    }
}
