//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/WaylandEGLExtensionProcs.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/StringUtils.h"

#include <cassert>

namespace ramses::internal
{
    inline std::string getString(const char* str)
    {
        return str != nullptr ? str : "";
    }

    WaylandEGLExtensionProcs::WaylandEGLExtensionProcs(wl_display* waylandWindowDisplay)
        : m_eglDisplay(eglGetDisplay(waylandWindowDisplay))
        , m_eglCreateImageKHR(nullptr)
        , m_eglDestroyImageKHR(nullptr)
        , m_eglBindWaylandDisplayWL(nullptr)
        , m_eglUnbindWaylandDisplayWL(nullptr)
        , m_eglQueryWaylandBufferWL(nullptr)
        , m_extensionsSupported(false)
        , m_dmabufExtensionsSupported(false)
    {
        Init();
    }

    WaylandEGLExtensionProcs::WaylandEGLExtensionProcs(EGLDisplay eglDisplay)
        : m_eglDisplay(eglDisplay)
        , m_eglCreateImageKHR(nullptr)
        , m_eglDestroyImageKHR(nullptr)
        , m_eglBindWaylandDisplayWL(nullptr)
        , m_eglUnbindWaylandDisplayWL(nullptr)
        , m_eglQueryWaylandBufferWL(nullptr)
        , m_extensionsSupported(false)
        , m_dmabufExtensionsSupported(false)
    {
        Init();
    }

    void WaylandEGLExtensionProcs::Init()
    {
        if (m_eglDisplay == EGL_NO_DISPLAY)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::Init EGL_NO_DISPLAY");
        }
        if (!glGetString)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::Init glGetString is not available (context not initialized)");
            return;
        }

        const auto eglExtensionsString = getString(eglQueryString(m_eglDisplay, EGL_EXTENSIONS));
        const auto glExtensionsString = getString(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

        const auto eglExtensions = StringUtils::TokenizeToSet(eglExtensionsString);
        const auto glExtensions = StringUtils::TokenizeToSet(glExtensionsString);

        if (CheckExtensionAvailable(eglExtensions, "EGL_WL_bind_wayland_display"))
        {
            m_eglBindWaylandDisplayWL = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglBindWaylandDisplayWL"));
            LOG_INFO(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::Init: loaded proc eglBindWaylandDisplayWL :{}", reinterpret_cast<void*>(m_eglBindWaylandDisplayWL));
            assert(m_eglBindWaylandDisplayWL != nullptr);

            m_eglUnbindWaylandDisplayWL = reinterpret_cast<PFNEGLBINDWAYLANDDISPLAYWL>(eglGetProcAddress("eglUnbindWaylandDisplayWL"));
            LOG_INFO(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::Init: loaded proc eglUnbindWaylandDisplayWL :{}", reinterpret_cast<void*>(m_eglUnbindWaylandDisplayWL));
            assert(m_eglUnbindWaylandDisplayWL != nullptr);
        }

        if (CheckExtensionAvailable(eglExtensions, "EGL_KHR_image_base"))
        {
            m_eglCreateImageKHR = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
            LOG_INFO(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::Init: loaded proc eglCreateImageKHR :{}", reinterpret_cast<void*>(m_eglCreateImageKHR));
            assert(m_eglCreateImageKHR != nullptr);

            m_eglDestroyImageKHR = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
            LOG_INFO(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::Init: loaded proc eglDestroyImageKHR :{}", reinterpret_cast<void*>(m_eglDestroyImageKHR));
            assert(m_eglDestroyImageKHR != nullptr);

            m_eglQueryWaylandBufferWL = reinterpret_cast<PFNEGLQUERYWAYLANDBUFFERWL>(eglGetProcAddress("eglQueryWaylandBufferWL"));
            LOG_INFO(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::Init: loaded proc eglQueryWaylandBufferWL :{}", reinterpret_cast<void*>(m_eglQueryWaylandBufferWL));
            assert(m_eglQueryWaylandBufferWL != nullptr);

            m_extensionsSupported = true;
        }

        if (CheckExtensionAvailable(eglExtensions, "EGL_KHR_image_base") &&
            CheckExtensionAvailable(eglExtensions, "EGL_EXT_image_dma_buf_import"))
        {
            m_dmabufExtensionsSupported = true;
        }
    }

    bool WaylandEGLExtensionProcs::CheckExtensionAvailable(const HashSet<std::string>& eglExtensions, const std::string& extensionName)
    {
        if (eglExtensions.contains(extensionName))
        {
            LOG_INFO(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::CheckExtensionAvailable Extension {} is supported!", extensionName);
            return true;
        }
        LOG_WARN(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::CheckExtensionAvailable Extension {} not supported!", extensionName);
        return false;
    }

    EGLImageKHR WaylandEGLExtensionProcs::eglCreateImageKHR(EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLint* attributeList) const
    {
        if (m_eglCreateImageKHR)
        {
            return m_eglCreateImageKHR(m_eglDisplay, context, target, buffer, attributeList);
        }
        LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglCreateImageKHR Extension not bound!");
        return EGL_NO_IMAGE;
    }

    EGLBoolean WaylandEGLExtensionProcs::eglDestroyImageKHR(EGLImageKHR image) const
    {
        if (m_eglDestroyImageKHR)
        {
            return m_eglDestroyImageKHR(m_eglDisplay, image);
        }
        LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglDestroyImageKHR Extension not bound!");
        return EGL_FALSE;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void WaylandEGLExtensionProcs::glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image) const
    {
        if (::glEGLImageTargetTexture2DOES)
        {
            return ::glEGLImageTargetTexture2DOES(target, image);
        }
        LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::glEGLImageTargetTexture2DOES Extension not bound!");
    }

    EGLBoolean WaylandEGLExtensionProcs::eglBindWaylandDisplayWL(wl_display* waylandDisplay) const
    {
        if (m_eglBindWaylandDisplayWL)
        {
            return m_eglBindWaylandDisplayWL(m_eglDisplay, waylandDisplay);
        }
        LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglBindWaylandDisplayWL Extension not bound!");
        return EGL_FALSE;
    }

    EGLBoolean WaylandEGLExtensionProcs::eglUnbindWaylandDisplayWL(wl_display* waylandDisplay) const
    {
        if (m_eglUnbindWaylandDisplayWL)
        {
            return m_eglUnbindWaylandDisplayWL(m_eglDisplay, waylandDisplay);
        }
        LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglUnbindWaylandDisplayWL Extension not bound!");
        return EGL_FALSE;
    }

    EGLBoolean WaylandEGLExtensionProcs::eglQueryWaylandBufferWL(wl_resource* buffer, EGLint attribute, EGLint* value) const
    {
        if (m_eglQueryWaylandBufferWL)
        {
            return m_eglQueryWaylandBufferWL(m_eglDisplay, buffer, attribute, value);
        }
        LOG_ERROR(CONTEXT_RENDERER, "WaylandEGLExtensionProcs::eglQueryWaylandBufferWL Extension not bound!");
        return EGL_FALSE;
    }

    bool WaylandEGLExtensionProcs::areExtensionsSupported()const
    {
        return m_extensionsSupported;
    }

    bool WaylandEGLExtensionProcs::areDmabufExtensionsSupported() const
    {
        return m_dmabufExtensionsSupported;
    }

    const char* WaylandEGLExtensionProcs::getTextureFormatName(EGLint textureFormat)
    {
        switch (textureFormat)
        {
        case EGL_TEXTURE_RGB: return "EGL_TEXTURE_RGB";
        case EGL_TEXTURE_RGBA: return "EGL_TEXTURE_RGBA";
        default: return "<unknown>";
        }
    }
}
