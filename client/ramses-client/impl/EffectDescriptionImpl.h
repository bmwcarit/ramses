//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTDESCRIPTIONIMPL_H
#define RAMSES_EFFECTDESCRIPTIONIMPL_H

// API
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/EffectInputSemantic.h"

// internal
#include "StatusObjectImpl.h"

// ramses framework
#include "Collections/HashMap.h"
#include "SceneAPI/EFixedSemantics.h"

#include <string_view>
#include <string>
#include <vector>

namespace ramses
{
    class EffectDescriptionImpl : public StatusObjectImpl
    {
    public:
        using SemanticsMap = ramses_internal::HashMap<std::string, ramses_internal::EFixedSemantics>;

        EffectDescriptionImpl();
        ~EffectDescriptionImpl() override;

        status_t setVertexShader(std::string_view shaderSource);
        status_t setFragmentShader(std::string_view shaderSource);
        status_t setGeometryShader(std::string_view shaderSource);
        status_t setVertexShaderFromFile(std::string_view shaderSourceFileName);
        status_t setFragmentShaderFromFile(std::string_view shaderSourceFileName);
        status_t setGeometryShaderFromFile(std::string_view shaderSourceFileName);
        status_t addCompilerDefine(std::string_view define);
        status_t setUniformSemantic(std::string_view semanticName, EEffectUniformSemantic semanticType);
        status_t setAttributeSemantic(std::string_view semanticName, EEffectAttributeSemantic semanticType);

        const char*                          getVertexShader() const;
        const char*                          getFragmentShader() const;
        const char*                          getGeometryShader() const;
        size_t                               getNumberOfCompilerDefines() const;
        const std::vector<std::string>&      getCompilerDefines() const;
        const char*                          getCompilerDefine(size_t index) const;
        const SemanticsMap&                  getSemanticsMap() const;

        static bool ReadFileContentsToString(std::string_view fileName, std::string& fileContents);

    private:
        status_t setSemantic(std::string_view semanticName, ramses_internal::EFixedSemantics semanticType);

        std::string m_vertexShaderSource;
        std::string m_fragmentShaderSource;
        std::string m_geometryShaderSource;

        std::vector<std::string> m_compilerDefines;
        SemanticsMap             m_inputSemantics;
    };
}

#endif
