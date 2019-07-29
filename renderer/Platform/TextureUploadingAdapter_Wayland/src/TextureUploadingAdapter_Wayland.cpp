//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextureUploadingAdapter_Wayland/TextureUploadingAdapter_Wayland.h"
#include "RendererAPI/IDevice.h"
#include "Utils/LogMacros.h"

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
}
