//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEBUGOUTPUT_H
#define RAMSES_DEBUGOUTPUT_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Device_GL/Device_GL_platform.h"

namespace ramses_internal
{
    class IContext;

    class DebugOutput final
    {
    public:
        Bool enable(const IContext& context);
        Bool isAvailable() const;
        Bool checkAndResetError() const;

    private:
        Bool loadExtensionFunctionPointer(const IContext& context);

#if defined(__linux__) || defined(__ghs__)
        PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallback = nullptr;
        PFNGLDEBUGMESSAGECONTROLKHRPROC  glDebugMessageControl  = nullptr;
#else
        PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = 0;
        PFNGLDEBUGMESSAGECONTROLPROC  glDebugMessageControl  = 0;
#endif
        mutable Bool m_errorOccured = false;
    };
}

#endif
