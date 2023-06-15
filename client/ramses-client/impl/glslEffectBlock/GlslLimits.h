//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLSLLIMITS_H
#define RAMSES_GLSLLIMITS_H

#include "PlatformAbstraction/PlatformTypes.h"

#include <cstdlib>
#include <cctype>

struct TBuiltInResource;

namespace ramses_internal
{
    class GlslLimits
    {
    public:

        static void InitCompilationResources(TBuiltInResource& glslCompilationResources, std::string_view glslVersion)
        {
            SetDefaults(glslCompilationResources);

            const uint32_t version = GetVersionFromString(glslVersion);
            const bool isES = IsESVersion(glslVersion);

            if (isES)
            {
                if (version >= 300)
                {
                    SetLimitsOpenGL_ES_3_0(glslCompilationResources);
                }
                else
                {
                    // This is not the most correct fall-back, but since we now officially only supports ES 3.0 it does not matter.
                    SetLimitsOpenGL_2_0(glslCompilationResources);
                }
            }
            else // OpenGL "desktop" versions
            {
                if (version >= 420)
                {
                    SetLimitsOpenGL_4_2(glslCompilationResources);
                }
                else
                {
                    SetLimitsOpenGL_2_0(glslCompilationResources);
                }
            }
        }

        static uint32_t GetVersionFromString(std::string_view glslVersion)
        {
            std::array<char, 16> buffer;
            uint32_t n = 0;

            for (size_t i = 0; i < glslVersion.size(); i++)
            {
                const char& c = glslVersion[i];

                if (isdigit(c))
                {
                    buffer[n++] = c;
                }

                if (n >= buffer.size())
                {
                    // Avoid out of bounds access, in case the shader contains some crazy version number
                    return 0;
                }
            }

            buffer[n] = 0;
            return std::atoi(buffer.data());
        }

        static bool IsESVersion(std::string_view glslVersion)
        {
            // The input parameter can contain newlines characters, so checking for endsWith() can fail.
            static constexpr auto esIdentifier{" es"};
            return glslVersion.find(esIdentifier, 0u) != std::string_view::npos;
        }

    private:

        static void SetDefaults(TBuiltInResource& glslCompilationResources)
        {
            memset(&glslCompilationResources, 0, sizeof(TBuiltInResource));

            glslCompilationResources.limits.nonInductiveForLoops = true;
            glslCompilationResources.limits.whileLoops = true;
            glslCompilationResources.limits.doWhileLoops = true;
            glslCompilationResources.limits.generalUniformIndexing = true;
            glslCompilationResources.limits.generalAttributeMatrixVectorIndexing = true;
            glslCompilationResources.limits.generalVaryingIndexing = true;
            glslCompilationResources.limits.generalSamplerIndexing = true;
            glslCompilationResources.limits.generalVariableIndexing = true;
            glslCompilationResources.limits.generalConstantMatrixVectorIndexing = true;
        }

        static void SetLimitsOpenGL_2_0(TBuiltInResource& glslCompilationResources)
        {
            // From https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.10.pdf (section 7.4)

            glslCompilationResources.maxLights = 8;
            glslCompilationResources.maxClipPlanes = 6;
            glslCompilationResources.maxTextureUnits = 2;
            glslCompilationResources.maxTextureCoords = 2;
            glslCompilationResources.maxVertexAttribs = 16;
            glslCompilationResources.maxVertexUniformComponents = 512;
            glslCompilationResources.maxVaryingFloats = 32;
            glslCompilationResources.maxVertexTextureImageUnits = 0;
            glslCompilationResources.maxCombinedTextureImageUnits = 2;
            glslCompilationResources.maxTextureImageUnits = 2;
            glslCompilationResources.maxFragmentUniformComponents = 64;
            glslCompilationResources.maxDrawBuffers = 1;
        }

        static void SetLimitsOpenGL_ES_3_0(TBuiltInResource& glslCompilationResources)
        {
            // From https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf (section 7.3)

            glslCompilationResources.maxVertexAttribs = 16;
            glslCompilationResources.maxVertexUniformVectors = 256;
            glslCompilationResources.maxVertexOutputVectors = 16;
            glslCompilationResources.maxFragmentInputVectors = 15;
            glslCompilationResources.maxVertexTextureImageUnits = 16;
            glslCompilationResources.maxCombinedTextureImageUnits = 32;
            glslCompilationResources.maxTextureImageUnits = 16;
            glslCompilationResources.maxFragmentUniformVectors = 224;
            glslCompilationResources.maxDrawBuffers = 4;
            glslCompilationResources.minProgramTexelOffset = -8;
            glslCompilationResources.maxProgramTexelOffset = 7;
            // https://www.khronos.org/registry/OpenGL-Refpages/es3/html/glGet.xhtml
            glslCompilationResources.maxGeometryOutputVertices = 256;
        }

        static void SetLimitsOpenGL_4_2(TBuiltInResource& glslCompilationResources)
        {
            // From https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.20.pdf (section 7.3)

            glslCompilationResources.maxVertexAttribs = 16;
            glslCompilationResources.maxVertexUniformComponents = 1024;
            glslCompilationResources.maxVaryingComponents = 60;
            glslCompilationResources.maxVertexOutputComponents = 64;
            glslCompilationResources.maxGeometryInputComponents = 64;
            glslCompilationResources.maxGeometryOutputComponents = 128;
            glslCompilationResources.maxFragmentInputComponents = 128;
            glslCompilationResources.maxVertexTextureImageUnits = 16;
            glslCompilationResources.maxCombinedTextureImageUnits = 80;
            glslCompilationResources.maxTextureImageUnits = 16;
            glslCompilationResources.maxImageUnits = 8;
            glslCompilationResources.maxCombinedImageUnitsAndFragmentOutputs = 8;
            glslCompilationResources.maxImageSamples = 0;
            glslCompilationResources.maxVertexImageUniforms = 0;
            glslCompilationResources.maxTessControlImageUniforms = 0;
            glslCompilationResources.maxTessEvaluationImageUniforms = 0;
            glslCompilationResources.maxGeometryImageUniforms = 0;
            glslCompilationResources.maxFragmentImageUniforms = 0;
            glslCompilationResources.maxCombinedImageUniforms = 0;
            glslCompilationResources.maxFragmentUniformComponents = 1024;
            glslCompilationResources.maxDrawBuffers = 8;
            glslCompilationResources.maxClipDistances = 8;
            glslCompilationResources.maxGeometryTextureImageUnits = 16;
            glslCompilationResources.maxGeometryOutputVertices = 256;
            glslCompilationResources.maxGeometryTotalOutputComponents = 1024;
            glslCompilationResources.maxGeometryUniformComponents = 1024;
            glslCompilationResources.maxGeometryVaryingComponents = 64;
            glslCompilationResources.maxTessControlInputComponents = 128;
            glslCompilationResources.maxTessControlOutputComponents = 128;
            glslCompilationResources.maxTessControlTextureImageUnits = 16;
            glslCompilationResources.maxTessControlUniformComponents = 1024;
            glslCompilationResources.maxTessControlTotalOutputComponents = 4096;
            glslCompilationResources.maxTessEvaluationInputComponents = 128;
            glslCompilationResources.maxTessEvaluationOutputComponents = 128;
            glslCompilationResources.maxTessEvaluationTextureImageUnits = 16;
            glslCompilationResources.maxTessEvaluationUniformComponents = 1024;
            glslCompilationResources.maxTessPatchComponents = 120;
            glslCompilationResources.maxPatchVertices = 32;
            glslCompilationResources.maxTessGenLevel = 64;
            glslCompilationResources.maxViewports = 16;
            glslCompilationResources.maxVertexUniformVectors = 256;
            glslCompilationResources.maxFragmentUniformVectors = 256;
            glslCompilationResources.maxVaryingVectors = 15;
            glslCompilationResources.maxVertexAtomicCounters = 0;
            glslCompilationResources.maxTessControlAtomicCounters = 0;
            glslCompilationResources.maxTessEvaluationAtomicCounters = 0;
            glslCompilationResources.maxGeometryAtomicCounters = 0;
            glslCompilationResources.maxFragmentAtomicCounters = 8;
            glslCompilationResources.maxCombinedAtomicCounters = 8;
            glslCompilationResources.maxAtomicCounterBindings = 1;
            glslCompilationResources.maxVertexAtomicCounterBuffers = 0;
            glslCompilationResources.maxTessControlAtomicCounterBuffers = 0;
            glslCompilationResources.maxTessEvaluationAtomicCounterBuffers = 0;
            glslCompilationResources.maxGeometryAtomicCounterBuffers = 0;
            glslCompilationResources.maxFragmentAtomicCounterBuffers = 1;
            glslCompilationResources.maxCombinedAtomicCounterBuffers = 1;
            glslCompilationResources.maxAtomicCounterBufferSize = 16384;
            glslCompilationResources.minProgramTexelOffset = -8;
            glslCompilationResources.maxProgramTexelOffset = 7;
        }
    };
}

#endif
