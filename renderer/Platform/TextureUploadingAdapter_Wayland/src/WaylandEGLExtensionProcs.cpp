//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TextureUploadingAdapter_Wayland/WaylandEGLExtensionProcs.h"
#include "Utils/LogMacros.h"
#include <cassert>

namespace ramses_internal

{
    WaylandEGLExtensionProcs::WaylandEGLExtensionProcs(wl_display* waylandWindowDisplay)
        : m_eglDisplay(eglGetDisplay(waylandWindowDisplay))
        , m_eglCreateImageKHR(nullptr)
        , m_eglDestroyImageKHR(nullptr)
        , m_glEGLImageTargetTexture2DOES(nullptr)
        , m_eglBindWaylandDisplayWL(nullptr)
        , m_eglUnbindWaylandDisplayWL(nullptr)
        , m_extensionsSupported(false)
        , m_dmabufExtensionsSupported(false)
    {
        Init();
    }

    WaylandEGLExtensionProcs::WaylandEGLExtensionProcs(EGLDisplay eglDisplay)
        : m_eglDisplay(eglDisplay)
        , m_eglCreateImageKHR(nullptr)
        , m_eglDestroyImageKHR(nullptr)
        , m_glEGLImageTargetTexture2DOES(nullptr)
        , m_eglBindWaylandDisplayWL(nullptr)
        , m_eglUnbindWaylandDisplayWL(nullptr)
        , m_extensionsSupported(false)
        , m_dmabufExtensionsSupported(false)
    {
        Init();
    }

    void WaylandEGLExtensionProcs::Init()
    {
        ramses_internal::String eglExtensions(eglQueryString(m_eglDisplay, EGL_EXTENSIONS));
        ramses_internal::String glExtensions(reinterpret_cast<const ramses_internal::Char*>(glGetString(GL_EXTENSIONS)));

        if (CheckExtensionAvailable(glExtensions, "GL_OES_EGL_image") &&
            CheckExtensionAvailable(eglExtensions, "EGL_KHR_image_base") &&
            CheckExtensionAvailable(eglExtensions, "EGL_WL_bind_wayland_display"))
        {
            m_glEGLImageTargetTexture2DOES = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
            assert(m_glEGLImageTargetTexture2DOES != nullptr);

            m_eglCreateImageKHR = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
            assert(m_eglCreateImageKHR != nullptr);

            m_eglDestroyImageKHR = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
            assert(m_eglDestroyImageKHR != nullptr);

            m_eglBindWaylandDisplayWL = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
            assert(m_eglBindWaylandDisplayWL != nullptr);

            m_eglUnbindWaylandDisplayWL = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
            assert(m_eglUnbindWaylandDisplayWL != nullptr);

            m_extensionsSupported = true;
        }

        if (CheckExtensionAvailable(glExtensions, "GL_OES_EGL_image") &&
            CheckExtensionAvailable(eglExtensions, "EGL_KHR_image_base") &&
            CheckExtensionAvailable(eglExtensions, "EGL_EXT_image_dma_buf_import"))
        {
            m_dmabufExtensionsSupported = true;
        }
    }

    bool WaylandEGLExtensionProcs::CheckExtensionAvailable(const ramses_internal::String& eglExtensions, const ramses_internal::String& extensionName)
    {
        if (eglExtensions.find(extensionName) >= 0)
        {
            return true;
        }
        else
        {
            LOG_INFO(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::CheckExtensionAvailable Extension " << extensionName << " not supported!");
            return false;
        }
    }

    EGLImageKHR WaylandEGLExtensionProcs::eglCreateImageKHR(EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLint* attributeList) const
    {
        if (m_eglCreateImageKHR)
        {
            return m_eglCreateImageKHR(m_eglDisplay, context, target, buffer, attributeList);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglCreateImageKHR Extension not bound!");
            assert(false);
            return EGL_NO_IMAGE;
        }
    }

    EGLBoolean WaylandEGLExtensionProcs::eglDestroyImageKHR(EGLImageKHR image) const
    {
        if (m_eglDestroyImageKHR)
        {
            return m_eglDestroyImageKHR(m_eglDisplay, image);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglDestroyImageKHR Extension not bound!");
            assert(false);
            return EGL_FALSE;
        }
    }

    void WaylandEGLExtensionProcs::glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image) const
    {
        if (m_glEGLImageTargetTexture2DOES)
        {
            return m_glEGLImageTargetTexture2DOES(target, image);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::glEGLImageTargetTexture2DOES Extension not bound!");
            assert(false);
        }
    }

    EGLBoolean WaylandEGLExtensionProcs::eglBindWaylandDisplayWL(wl_display* waylandDisplay) const
    {
        if (m_eglBindWaylandDisplayWL)
        {
            return m_eglBindWaylandDisplayWL(m_eglDisplay, waylandDisplay);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglBindWaylandDisplayWL Extension not bound!");
            assert(false);
            return EGL_FALSE;
        }
    }

    EGLBoolean WaylandEGLExtensionProcs::eglUnbindWaylandDisplayWL(wl_display* waylandDisplay) const
    {
        if (m_eglUnbindWaylandDisplayWL)
        {
            return m_eglUnbindWaylandDisplayWL(m_eglDisplay, waylandDisplay);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglUnbindWaylandDisplayWL Extension not bound!");
            assert(false);
            return EGL_FALSE;
        }
    }

    bool WaylandEGLExtensionProcs::areExtensionsSupported()const
    {
        return m_extensionsSupported;
    }

    bool WaylandEGLExtensionProcs::areDmabufExtensionsSupported() const
    {
        return m_dmabufExtensionsSupported;
    }
}
