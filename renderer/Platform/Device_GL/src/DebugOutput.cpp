//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Device_GL/DebugOutput.h"

#include "RendererAPI/IContext.h"

#include "Utils/ThreadLocalLogForced.h"
#include <array>
#include <cassert>

namespace ramses_internal
{
#if defined(__linux__)
    #define GL_DEBUG_TYPE_ERROR                 GL_DEBUG_TYPE_ERROR_KHR
    #define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR    GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR
    #define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR   GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR
    #define GL_DEBUG_TYPE_PORTABILITY           GL_DEBUG_TYPE_PORTABILITY_KHR
    #define GL_DEBUG_TYPE_PERFORMANCE           GL_DEBUG_TYPE_PERFORMANCE_KHR
    #define GL_DEBUG_TYPE_MARKER                GL_DEBUG_TYPE_MARKER_KHR
    #define GL_DEBUG_TYPE_PUSH_GROUP            GL_DEBUG_TYPE_PUSH_GROUP_KHR
    #define GL_DEBUG_TYPE_POP_GROUP             GL_DEBUG_TYPE_POP_GROUP_KHR
    #define GL_DEBUG_TYPE_OTHER                 GL_DEBUG_TYPE_OTHER_KHR

    #define GL_DEBUG_SOURCE_API                 GL_DEBUG_SOURCE_API_KHR
    #define GL_DEBUG_TYPE_PERFORMANCE           GL_DEBUG_TYPE_PERFORMANCE_KHR
    #define GL_DEBUG_OUTPUT                     GL_DEBUG_OUTPUT_KHR
    #define GL_DEBUG_OUTPUT_SYNCHRONOUS         GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR

    #define APIENTRY                            GL_APIENTRY
#endif

    static void APIENTRY debugCallback(GLenum /*source*/,
                                       GLenum type,
                                       GLuint /*id*/,
                                       GLenum /*severity*/,
                                       GLsizei /*length*/,
                                       const GLchar* message,
                                       const void* userParam)
    {
        assert(userParam);

        // NOTE (tobias) work around case where callback is called from another thread
        // despite requesting synchronous dispatch via GL_DEBUG_OUTPUT_SYNCHRONOUS. This
        // can happen when the driver is bugger or does not support synchronous operation.
        // Prevent assert on log by ensuring there is always a valig TLS log prefix but
        // set it to a very clear invalid value.
        if (ThreadLocalLog::GetPrefixUnchecked() == -1)
        {
            ThreadLocalLog::SetPrefix(-2);
            LOG_WARN(CONTEXT_RENDERER, "Detected broken OpenGL driver ignoring GL_DEBUG_OUTPUT_SYNCHRONOUS!");
        }

        switch (type)
        {
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        case GL_DEBUG_TYPE_ERROR:
            LOG_ERROR(CONTEXT_RENDERER, "OpenGL error: " << message);
            *(const_cast<Bool*>(static_cast<const Bool*>(userParam))) = true;
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        case GL_DEBUG_TYPE_PORTABILITY:
        case GL_DEBUG_TYPE_PERFORMANCE:
            LOG_WARN(CONTEXT_RENDERER, "OpenGL warning: " << message);
            break;
        case GL_DEBUG_TYPE_MARKER:
        case GL_DEBUG_TYPE_PUSH_GROUP:
        case GL_DEBUG_TYPE_POP_GROUP:
        case GL_DEBUG_TYPE_OTHER:
        default:
            LOG_TRACE(CONTEXT_RENDERER, "OpenGL info: " << message);
        }
    }

    Bool DebugOutput::loadExtensionFunctionPointer(const IContext& context)
    {
#if defined(__linux__)
        glDebugMessageCallback =
            reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKKHRPROC>(context.getProcAddress("glDebugMessageCallbackKHR"));
        glDebugMessageControl =
            reinterpret_cast<PFNGLDEBUGMESSAGECONTROLKHRPROC>(context.getProcAddress("glDebugMessageControlKHR"));
#else
        glDebugMessageCallback =
            reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(context.getProcAddress("glDebugMessageCallback"));
        glDebugMessageControl =
            reinterpret_cast<PFNGLDEBUGMESSAGECONTROLPROC>(context.getProcAddress("glDebugMessageControl"));
#endif

        return isAvailable();
    }

    Bool DebugOutput::enable(const IContext& context)
    {
        if (!loadExtensionFunctionPointer(context))
        {
            LOG_INFO(CONTEXT_RENDERER, "Could not found OpenGL debug output extension");
            return false;
        }

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // pass flag per userParam so we can set it in case of an error
        glDebugMessageCallback(debugCallback, &m_errorOccured);

        // enable all debug messages...
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

        // ... except redundant state change warnings on nvidia cards
        const std::array<const GLuint, 1> messageIds{{8}};
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, static_cast<GLsizei>(messageIds.size()), messageIds.data(), GL_FALSE);

        return true;
    }

    Bool DebugOutput::isAvailable() const
    {
        return (glDebugMessageCallback != nullptr && glDebugMessageControl != nullptr);
    }

    Bool DebugOutput::checkAndResetError() const
    {
        if (m_errorOccured)
        {
            m_errorOccured = false;
            return true;
        }
        return false;
    }
}
