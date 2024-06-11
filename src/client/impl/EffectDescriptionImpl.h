//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// API
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/EffectInputSemantic.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"

#include <string_view>
#include <string>
#include <vector>

namespace ramses::internal
{
    class EffectDescriptionImpl
    {
    public:
        EffectDescriptionImpl();
        ~EffectDescriptionImpl();

        [[nodiscard]] bool setVertexShader(std::string_view shaderSource);
        [[nodiscard]] bool setFragmentShader(std::string_view shaderSource);
        [[nodiscard]] bool setGeometryShader(std::string_view shaderSource);
        [[nodiscard]] bool setVertexShaderFromFile(std::string_view shaderSourceFileName);
        [[nodiscard]] bool setFragmentShaderFromFile(std::string_view shaderSourceFileName);
        [[nodiscard]] bool setGeometryShaderFromFile(std::string_view shaderSourceFileName);
        [[nodiscard]] bool addCompilerDefine(std::string_view define);
        [[nodiscard]] bool setUniformSemantic(std::string_view semanticName, EEffectUniformSemantic semanticType);
        [[nodiscard]] bool setUniformSemantic(uint32_t uniformBufferBinding, EEffectUniformSemantic semanticType);
        [[nodiscard]] bool setAttributeSemantic(std::string_view semanticName, EEffectAttributeSemantic semanticType);

        [[nodiscard]] const char*                          getVertexShader() const;
        [[nodiscard]] const char*                          getFragmentShader() const;
        [[nodiscard]] const char*                          getGeometryShader() const;
        [[nodiscard]] size_t                               getNumberOfCompilerDefines() const;
        [[nodiscard]] const std::vector<std::string>&      getCompilerDefines() const;
        [[nodiscard]] const char*                          getCompilerDefine(size_t index) const;
        [[nodiscard]] const SemanticsMap&                  getSemanticsMap() const;

        [[nodiscard]] static bool ReadFileContentsToString(std::string_view fileName, std::string& fileContents);

    private:
        [[nodiscard]] bool setSemantic(std::string_view semanticName, EFixedSemantics semanticType);
        [[nodiscard]] bool setSemantic(uint32_t uniformBufferBinding, EFixedSemantics semanticType);

        std::string m_vertexShaderSource;
        std::string m_fragmentShaderSource;
        std::string m_geometryShaderSource;

        std::vector<std::string> m_compilerDefines;
        SemanticsMap             m_inputSemantics;
    };
}
