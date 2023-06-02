//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLSLEFFECT_H
#define RAMSES_GLSLEFFECT_H

#include "Collections/StringOutputStream.h"
#include "Collections/Vector.h"
#include "Collections/HashMap.h"
#include "SceneAPI/EFixedSemantics.h"
#include "SceneAPI/RenderState.h"
#include "Resource/IResource.h"

#include <string>
#include <string_view>

struct TBuiltInResource;
namespace glslang
{
    class TProgram;
    class TShader;
    class TPoolAllocator;
}

namespace ramses_internal
{
    class EffectResource;

    class GlslEffect
    {
    public:
        GlslEffect(std::string_view vertexShader,
            std::string_view fragmentShader,
            std::string_view geometryShader,
            const std::vector<std::string>& compilerDefines,
            const HashMap<std::string, EFixedSemantics>& semanticInputs,
            std::string_view name);

        ~GlslEffect();

        EffectResource* createEffectResource(ResourceCacheFlag cacheFlag);

        UInt32 getShadingLanguageVersion() const;
        std::string getEffectErrorMessages() const;

    private:
        struct ShaderParts
        {
            std::string version;
            std::string defines;
            std::string userCode;
        };

        const std::string m_vertexShader;
        const std::string m_fragmentShader;
        const std::string m_geometryShader;
        const std::vector<std::string> m_compilerDefines;
        const HashMap<std::string, EFixedSemantics> m_semanticInputs;
        const std::string m_name;

        mutable StringOutputStream m_errorMessages;
        EffectResource* m_effectResource;
        UInt32 m_shadingLanguageVersion;

        std::string createDefineString() const;
        bool createShaderParts(ShaderParts& outParts, const std::string& defineString, const std::string& userShader) const;
        std::string mergeShaderParts(const ShaderParts& shaderParts) const;
        bool parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const std::string& shaderName);
        glslang::TProgram* linkProgram(glslang::TShader* vertexShader, glslang::TShader* fragmentShader, glslang::TShader* geometryShader) const;
        bool extractAndCheckShaderVersions(const glslang::TProgram* program);
        bool extractAndCheckExtensions(const glslang::TProgram* program);
        bool isSupportedExtension(const std::string& extension) const;
    };

}

#endif
