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
#include "Utils/StringUtils.h"

namespace ramses
{
    class EffectDescriptionImpl : public StatusObjectImpl
    {
    public:
        using SemanticsMap = ramses_internal::HashMap<ramses_internal::String, ramses_internal::EFixedSemantics>;

        EffectDescriptionImpl();
        virtual ~EffectDescriptionImpl();

        status_t setVertexShader(const char* shaderSource);
        status_t setFragmentShader(const char* shaderSource);
        status_t setGeometryShader(const char* shaderSource);
        status_t setVertexShaderFromFile(const char* shaderSourceFileName);
        status_t setFragmentShaderFromFile(const char* shaderSourceFileName);
        status_t setGeometryShaderFromFile(const char* shaderSourceFileName);
        status_t addCompilerDefine(const char* define);
        status_t setUniformSemantic(const char* semanticName, EEffectUniformSemantic semanticType);
        status_t setAttributeSemantic(const char* semanticName, EEffectAttributeSemantic semanticType);

        const char*                          getVertexShader() const;
        const char*                          getFragmentShader() const;
        const char*                          getGeometryShader() const;
        uint32_t                             getNumberOfCompilerDefines() const;
        const ramses_internal::StringVector& getCompilerDefines() const;
        const char*                          getCompilerDefine(uint32_t index) const;
        const SemanticsMap&                  getSemanticsMap() const;

        static bool ReadFileContentsToString(const char* fileName, ramses_internal::String& fileContents);

    private:
        status_t setSemantic(const char* semanticName, ramses_internal::EFixedSemantics semanticType);

        ramses_internal::String m_vertexShaderSource;
        ramses_internal::String m_fragmentShaderSource;
        ramses_internal::String m_geometryShaderSource;

        ramses_internal::StringVector m_compilerDefines;
        SemanticsMap                  m_inputSemantics;
    };
}

#endif
