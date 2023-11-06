//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/OpenGL/Device_GL_platform.h"
#include "internal/Platform/OpenGL/ShaderUploader_GL.h"

#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/Platform/OpenGL/ShaderProgramInfo.h"
#include "internal/Core/Utils/LogMacros.h"
#include "absl/strings/str_split.h"

namespace ramses::internal
{
    bool ShaderUploader_GL::UploadShaderProgramFromBinary(const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat, ShaderProgramInfo& programShaderInfoOut, std::string& debugErrorLog)
    {
        LOG_TRACE(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromBinary:  uploading binary data");

        const GLHandle programHandle = glCreateProgram();
        if (0u == programHandle)
        {
            debugErrorLog = "ShaderGPUResource_GL::uploadFromBinary:  unable to get a shader program handle from OpenGL";
            return false;
        }

        glProgramBinary(programHandle, binaryShaderFormat.getValue(), binaryShaderData, static_cast<GLsizei>(binaryShaderDataSize));

        if (CheckShaderProgramLinkStatus(programHandle, debugErrorLog))
        {
            programShaderInfoOut.shaderProgramHandle = programHandle;
            return true;
        }
        glDeleteProgram(programHandle);
        return false;
    }


    bool ShaderUploader_GL::UploadShaderProgramFromSource(const EffectResource& effect, ShaderProgramInfo& programShaderInfoOut, std::string& debugErrorLog)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  compiling shaders for effect " << effect.getName());

        const GLHandle vertexShaderHandle = CompileShaderStage(effect.getVertexShader(), GL_VERTEX_SHADER, debugErrorLog);

        if (InvalidGLHandle == vertexShaderHandle)
        {
            LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  vertex shader failed to compile " << debugErrorLog);
            return false;
        }

        const GLHandle fragmentShaderHandle = CompileShaderStage(effect.getFragmentShader(), GL_FRAGMENT_SHADER, debugErrorLog);

        if (InvalidGLHandle == fragmentShaderHandle)
        {
            LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  fragment shader failed to compile " << debugErrorLog);
            glDeleteShader(vertexShaderHandle);
            return false;
        }

        const bool hasGeometryShader = (std::strcmp(effect.getGeometryShader(), "") != 0);
        GLHandle geometryShaderHandle = InvalidGLHandle;
        if (hasGeometryShader)
        {
            geometryShaderHandle = CompileShaderStage(effect.getGeometryShader(), GL_GEOMETRY_SHADER_EXT, debugErrorLog);

            if (InvalidGLHandle == geometryShaderHandle)
            {
                LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  geometry shader failed to compile " << debugErrorLog);
                glDeleteShader(vertexShaderHandle);
                glDeleteShader(fragmentShaderHandle);
                return false;
            }
        }

        LOG_TRACE(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  linking shader program");

        const GLHandle shaderProgramHandle = glCreateProgram();

        if (InvalidGLHandle == shaderProgramHandle)
        {
            //Code-Coverage Note: this path is not feasably coverable, since GL only could fail to allocate the GLHandle in cases like total GPU memory exhaustion or being called from outside the GL-context
            LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  glCreateProgram failed");
            debugErrorLog = "Unable to create shader program";
            glDeleteShader(vertexShaderHandle);
            glDeleteShader(fragmentShaderHandle);
            if (hasGeometryShader)
                glDeleteShader(geometryShaderHandle);
            return false;
        }

        glAttachShader(shaderProgramHandle, fragmentShaderHandle);
        glAttachShader(shaderProgramHandle, vertexShaderHandle);
        if (hasGeometryShader)
            glAttachShader(shaderProgramHandle, geometryShaderHandle);
        glLinkProgram(shaderProgramHandle);

        if (CheckShaderProgramLinkStatus(shaderProgramHandle, debugErrorLog))
        {
            programShaderInfoOut.vertexShaderHandle = vertexShaderHandle;
            programShaderInfoOut.fragmentShaderHandle = fragmentShaderHandle;
            programShaderInfoOut.shaderProgramHandle = shaderProgramHandle;
            programShaderInfoOut.geometryShaderHandle = geometryShaderHandle;
            return true;
        }
        LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  CheckShaderProgramLinkStatus failed");
        glDeleteProgram(shaderProgramHandle);
        glDeleteShader(vertexShaderHandle);
        glDeleteShader(fragmentShaderHandle);
        if (hasGeometryShader)
            glDeleteShader(geometryShaderHandle);
        return false;
    }

    bool ShaderUploader_GL::CheckShaderProgramLinkStatus(GLHandle shaderProgram, std::string& errorLogOut)
    {
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);

        if (GL_FALSE == linkStatus)
        {
            GLint charBufferSize = 0;
            glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &charBufferSize);
            if (charBufferSize > 0)
            {
                std::string str;
                // charBufferSize includes null termination character and data() returns array which is null-terminated
                str.resize(charBufferSize - 1);

                GLsizei numChars = 0;
                glGetProgramInfoLog(shaderProgram, charBufferSize, &numChars, str.data());
                // Might be useful for the case when charBufferSize reported by glGetProgramiv is bigger than the real data
                // returned by glGetProgramInfoLog. Does not affect performance, it is a case of error handling.
                str.resize(numChars);

                errorLogOut = "Failed to link shader program:\n";
                errorLogOut += str;
                errorLogOut += "\n";
            }
            else
            {
                errorLogOut = "Failed to link shader program - no info given from compiler\n";
            }

            return false;
        }
        return true;
    }

    GLHandle ShaderUploader_GL::CompileShaderStage(const char* stageSource, GLenum shaderType, std::string& errorLogOut)
    {
        GLHandle shaderHandle = glCreateShader(shaderType);

        if (InvalidGLHandle != shaderHandle)
        {
            glShaderSource(shaderHandle, 1, &stageSource, nullptr);
            glCompileShader(shaderHandle);

            GLint compilationResult = GL_FALSE;
            glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compilationResult);

            if (compilationResult == GL_FALSE)
            {
                std::string info;

                GLint charBufferSize = 0;
                glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &charBufferSize);
                if (charBufferSize > 0)
                {
                    // charBufferSize includes null termination character and data() returns array which is null-terminated
                    info.resize(charBufferSize - 1);
                    GLsizei numberChars = 0;
                    glGetShaderInfoLog(shaderHandle, charBufferSize, &numberChars, info.data());
                    // Might be useful for the case when charBufferSize reported by glGetShaderiv is bigger than the real data
                    // returned by glGetShaderInfoLog. Does not affect performance, it is a case of error handling.
                    info.resize(numberChars);
                }
                else
                {
                    info = "no info given from compiler";
                }

                errorLogOut = std::string("Unable to compile shader stage: ") + info;

                PrintShaderSourceWithLineNumbers(stageSource);
                glDeleteShader(shaderHandle);
                shaderHandle = InvalidGLHandle;
            }
        }

        return shaderHandle;
    }

    void ShaderUploader_GL::PrintShaderSourceWithLineNumbers(std::string_view source)
    {
        uint32_t lineNumber = 1;
        for (const auto& line : absl::StrSplit(source, '\n'))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_Base::PrintShaderSourceWithLineNumbers:  L" << lineNumber << ": " << line);
            ++lineNumber;
        }
    }
}
