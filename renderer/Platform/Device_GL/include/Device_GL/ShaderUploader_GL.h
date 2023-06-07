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

#include <string>
#include <string_view>

namespace ramses_internal
{
    struct ShaderProgramInfo;
    class EffectResource;

    class ShaderUploader_GL
    {
    public:
        static bool UploadShaderProgramFromSource(const EffectResource& effect, ShaderProgramInfo& programShaderInfoOut, std::string& debugErrorLog);
        static bool UploadShaderProgramFromBinary(const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat, ShaderProgramInfo& programShaderInfoOut, std::string& debugErrorLog);

    private:
        static GLHandle CompileShaderStage(const char* stageSource, GLenum shaderType, std::string& errorLogOut);
        static bool CheckShaderProgramLinkStatus(GLHandle shaderProgram, std::string& errorLogOut);
        static void PrintShaderSourceWithLineNumbers(std::string_view source);
    };
}

#endif
