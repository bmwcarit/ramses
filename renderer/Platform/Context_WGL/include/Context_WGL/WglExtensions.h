//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WGLEXTENSIONS_H
#define RAMSES_WGLEXTENSIONS_H

#include "RendererAPI/Types.h"
#include "Collections/HashSet.h"

#include "PlatformAbstraction/MinimalWindowsH.h"
#include "GL/GL.h"
#include "GL/wgl.h"

#include <string>

namespace ramses_internal
{
    struct Procs
    {
        Procs();

        PFNWGLCHOOSEPIXELFORMATARBPROC      wglChoosePixelFormatARB;
        PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
        PFNWGLCREATECONTEXTATTRIBSARBPROC   wglCreateContextAttribsARB;
        PFNWGLGETEXTENSIONSSTRINGARBPROC    wglGetExtensionsStringARB;
        PFNWGLSWAPINTERVALEXTPROC           wglSwapIntervalEXT;
    };

    class WglExtensions
    {
    public:
        WglExtensions();

        bool areLoaded() const;
        bool isExtensionAvailable(const std::string& extensionName);

        Procs procs;

    private:
        HashSet<std::string> m_extensionNames;
        bool m_loaded;
    };
}

#endif
