//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "glslEffectBlock/GLSlang.h"
#include "SceneAPI/EShaderStage.h"
#include "SceneAPI/EShaderWarningCategory.h"

#include "Collections/String.h"
#include <vector>

namespace ramses_internal
{
    class GlslParser
    {
    public:
        struct Warning
        {
            EShaderStage stage;
            EShaderWarningCategory category;
            std::string msg;
        };
        using Warnings = std::vector<Warning>;

        GlslParser(const String& vertexShader, const String& fragmentShader, const String& geometryShader = {}, const std::vector<String>& compilerDefines = {});

        Warnings generateWarnings();

        String getErrors() const;

        const glslang::TProgram* getProgram() const;

    private:
        struct ShaderParts
        {
            String version;
            String defines;
            String userCode;
        };

        static String CreateDefineString(const std::vector<String>& compilerDefines);
        bool createShaderParts(ShaderParts& outParts, const String& defineString, const String& userShader, const String& shaderName) const;
        bool parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const String& shaderName);
        void linkProgram();

        mutable StringOutputStream m_errorMessages;
        std::unique_ptr<glslang::TShader>  m_vertexShader;
        std::unique_ptr<glslang::TShader>  m_fragmentShader;
        std::unique_ptr<glslang::TShader>  m_geometryShader;
        std::unique_ptr<glslang::TProgram> m_program;
    };
}
