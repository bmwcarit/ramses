//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/glslEffectBlock/GLSlang.h"
#include "internal/SceneGraph/SceneAPI/EShaderStage.h"
#include "internal/SceneGraph/SceneAPI/EShaderWarningCategory.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/SceneGraph/Resource/SPIRVShaders.h"

#include "ramses/framework/ERenderBackendCompatibility.h"

#include <string>
#include <vector>
#include <utility>
#include <memory>

namespace ramses::internal
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

        GlslParser(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader = {}, const std::vector<std::string>& compilerDefines = {}, ERenderBackendCompatibility compatibility = ERenderBackendCompatibility::OpenGL);

        [[nodiscard]] bool valid() const;

        [[nodiscard]] const std::string& getVertexShader() const;
        [[nodiscard]] const std::string& getFragmentShader() const;
        [[nodiscard]] const std::string& getGeometryShader() const;

        [[nodiscard]] const SPIRVShaders& getSPIRVShaders() const;

        [[nodiscard]] Warnings generateWarnings() const;

        [[nodiscard]] std::string getErrors() const;

        [[nodiscard]] const glslang::TProgram* getProgram() const;

    private:
        struct ShaderParts
        {
            std::string version;
            std::string defines;
            std::string userCode;
        };

        static std::string CreateDefineString(const std::vector<std::string>& compilerDefines);
        void setShadersCompatibility(ERenderBackendCompatibility compatibility);
        bool createShaderParts(ShaderParts& outParts, const std::string& defineString, const std::string& userShader, const std::string& shaderName) const;
        bool parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const std::string& shaderName);
        inline bool linkProgram();
        void generateSPIRV(ERenderBackendCompatibility compatibility);
        static std::string MergeShaderParts(const ShaderParts& shaderParts);

        mutable StringOutputStream m_errorMessages;
        std::unique_ptr<glslang::TShader>  m_vertexShader;
        std::unique_ptr<glslang::TShader>  m_fragmentShader;
        std::unique_ptr<glslang::TShader>  m_geometryShader;
        std::unique_ptr<glslang::TProgram> m_program;

        std::string m_vertexShaderFromParts;
        std::string m_fragmentShaderFromParts;
        std::string m_geometryShaderFromParts;

        SPIRVShaders m_spirvShaders;
    };
}
