//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADERUPLOADER_GL_H
#define RAMSES_SHADERUPLOADER_GL_H

#include "RendererAPI/Types.h"
#include "Types_GL.h"

namespace ramses_internal
{
    struct ShaderProgramInfo;
    class EffectResource;

    class ShaderUploader_GL
    {
    public:
        static bool UploadShaderProgramFromSource(const EffectResource& effect, ShaderProgramInfo& programShaderInfoOut, String& debugErrorLog);
        static bool UploadShaderProgramFromBinary(const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat, ShaderProgramInfo& programShaderInfoOut, String& debugErrorLog);

    private:
        static GLHandle CompileShaderStage(const char* stageSource, GLenum shaderType, String& errorLogOut);
        static bool CheckShaderProgramLinkStatus(GLHandle shaderProgram, String& errorLogOut);
        static void PrintShaderSourceWithLineNumbers(const String& source);
    };
}

#endif
