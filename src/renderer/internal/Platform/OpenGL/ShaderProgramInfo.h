//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/OpenGL/Types_GL.h"

#include <cstdint>

namespace ramses::internal
{
    struct ShaderProgramInfo
    {
        ShaderProgramInfo()
            : shaderProgramHandle(InvalidGLHandle)
            , vertexShaderHandle(InvalidGLHandle)
            , fragmentShaderHandle(InvalidGLHandle)
            , geometryShaderHandle(InvalidGLHandle)
        {
        }

        GLHandle shaderProgramHandle;
        GLHandle vertexShaderHandle;
        GLHandle fragmentShaderHandle;
        GLHandle geometryShaderHandle;
    };
}
