//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADERPROGRAMINFO_H
#define RAMSES_SHADERPROGRAMINFO_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Device_GL/Types_GL.h"

namespace ramses_internal
{
    struct ShaderProgramInfo
    {
        ShaderProgramInfo()
            : shaderProgramHandle(InvalidGLHandle)
            , vertexShaderHandle(InvalidGLHandle)
            , fragmentShaderHandle(InvalidGLHandle)
        {
        }

        GLHandle shaderProgramHandle;
        GLHandle vertexShaderHandle;
        GLHandle fragmentShaderHandle;
    };
}

#endif
