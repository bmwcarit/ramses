//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Device_GL/Device_GL_platform.h"
#include "Device_GL/ShaderUploader_GL.h"

#include "Resource/EffectResource.h"
#include "Device_GL/ShaderProgramInfo.h"

namespace ramses_internal
{
    Bool ShaderUploader_GL::UploadShaderProgramFromBinary(const UInt8* binaryShaderData, UInt32 binaryShaderDataSize, UInt32 binaryShaderFormat, ShaderProgramInfo& programShaderInfoOut, String& debugErrorLog)
    {
        LOG_TRACE(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromBinary:  uploading binary data");

        const GLHandle programHandle = glCreateProgram();
        if (0u == programHandle)
        {
            debugErrorLog = "ShaderGPUResource_GL::uploadFromBinary:  unable to get a shader program handle from OpenGL";
            return false;
        }

        glProgramBinary(programHandle, binaryShaderFormat, binaryShaderData, binaryShaderDataSize);

        if (CheckShaderProgramLinkStatus(programHandle, debugErrorLog))
        {
            programShaderInfoOut.shaderProgramHandle = programHandle;
            return true;
        }
        else
        {
            glDeleteProgram(programHandle);
            return false;
        }
    }

    Bool ShaderUploader_GL::UploadShaderProgramFromSource(const EffectResource& effect, ShaderProgramInfo& programShaderInfoOut, String& debugErrorLog)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  compiling shaders for effect " << effect.getName());

        const GLHandle vertexShaderHandle = CompileShaderStage(effect.getVertexShader(), GL_VERTEX_SHADER, debugErrorLog);

        if (InvalidGLHandle == vertexShaderHandle)
        {
            LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  vertex shader failed to compile " << debugErrorLog.c_str());
            return false;
        }

        const GLHandle fragmentShaderHandle = CompileShaderStage(effect.getFragmentShader(), GL_FRAGMENT_SHADER, debugErrorLog);

        if (InvalidGLHandle == fragmentShaderHandle)
        {
            LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  fragment shader failed to compile " << debugErrorLog.c_str());
            glDeleteShader(vertexShaderHandle);
            return false;
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
            return false;
        }

        glAttachShader(shaderProgramHandle, fragmentShaderHandle);
        glAttachShader(shaderProgramHandle, vertexShaderHandle);
        glLinkProgram(shaderProgramHandle);

        GLint linkStatus;
        glGetProgramiv(shaderProgramHandle, GL_LINK_STATUS, &linkStatus);

        if (CheckShaderProgramLinkStatus(shaderProgramHandle, debugErrorLog))
        {
            programShaderInfoOut.vertexShaderHandle = vertexShaderHandle;
            programShaderInfoOut.fragmentShaderHandle = fragmentShaderHandle;
            programShaderInfoOut.shaderProgramHandle = shaderProgramHandle;
            return true;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "ShaderUploader_GL::UploadShaderProgramFromSource:  CheckShaderProgramLinkStatus failed");
            glDeleteProgram(shaderProgramHandle);
            glDeleteShader(vertexShaderHandle);
            glDeleteShader(fragmentShaderHandle);
            return false;
        }
    }

    Bool ShaderUploader_GL::CheckShaderProgramLinkStatus(GLHandle shaderProgram, String& errorLogOut)
    {
        GLint linkStatus;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);

        if (GL_FALSE == linkStatus)
        {
            Int32 infoLength;
            glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLength);
            if (infoLength > 0)
            {
                String str;
                str.resize(infoLength);

                Int32 numChars;
                glGetProgramInfoLog(shaderProgram, infoLength, &numChars, str.data());
                str.resize(numChars);

                errorLogOut = "Failed to link shader program:\n";
                errorLogOut += str;
                errorLogOut += "\n";
            }
            else
                errorLogOut = "Failed to link shader program - no info given from compiler\n";

            return false;
        }
        else
        {
            return true;
        }
    }

    GLHandle ShaderUploader_GL::CompileShaderStage(const char* stageSource, GLenum shaderType, String& errorLogOut)
    {
        GLHandle shaderHandle = glCreateShader(shaderType);

        if (InvalidGLHandle != shaderHandle)
        {
            glShaderSource(shaderHandle, 1, &stageSource, NULL);
            glCompileShader(shaderHandle);

            GLint compilationResult = GL_FALSE;
            glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compilationResult);

            if (compilationResult == GL_FALSE)
            {
                Int32 infoLength;
                Int32 numberChars;
                glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLength);

                // Allocate Log Space
                Char* info = new Char[infoLength];
                glGetShaderInfoLog(shaderHandle, infoLength, &numberChars, info);
                errorLogOut = String("Unable to compile shader stage: ") + String(info);
                delete[] info;

                PrintShaderSourceWithLineNumbers(stageSource);
                glDeleteShader(shaderHandle);
                shaderHandle = InvalidGLHandle;
            }
        }

        return shaderHandle;
    }

    void ShaderUploader_GL::PrintShaderSourceWithLineNumbers(const String& source)
    {
        UInt32 lineNumber = 1;
        Int prevNewLine = 0;
        Int nextNewLine = source.find("\n");

        while (-1 != nextNewLine)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Device_Base::PrintShaderSourceWithLineNumbers:  L" << lineNumber << ": " << source.substr(prevNewLine, nextNewLine - prevNewLine));

            prevNewLine = nextNewLine + 1;
            nextNewLine = source.find("\n", prevNewLine);
            ++lineNumber;
        }
    }
}
