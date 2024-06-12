//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// internal
#include "impl/EffectDescriptionImpl.h"
#include "impl/EffectInputSemanticUtils.h"

// framework
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/LogMacros.h"


namespace ramses::internal
{
    EffectDescriptionImpl::EffectDescriptionImpl() = default;

    EffectDescriptionImpl::~EffectDescriptionImpl() = default;

    bool EffectDescriptionImpl::setVertexShader(std::string_view shaderSource)
    {
        m_vertexShaderSource = shaderSource;
        return true;
    }

    bool EffectDescriptionImpl::setFragmentShader(std::string_view shaderSource)
    {
        m_fragmentShaderSource = shaderSource;
        return true;
    }

    bool EffectDescriptionImpl::setGeometryShader(std::string_view shaderSource)
    {
        m_geometryShaderSource = shaderSource;
        return true;
    }

    bool EffectDescriptionImpl::setVertexShaderFromFile(std::string_view shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_vertexShaderSource))
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescription::setVertexShaderFromFile could not read file!");
            return false;
        }

        return true;
    }

    bool EffectDescriptionImpl::setFragmentShaderFromFile(std::string_view shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_fragmentShaderSource))
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescription::setFragmentShaderFromFile could not read file!");
            return false;
        }

        return true;
    }

    bool EffectDescriptionImpl::setGeometryShaderFromFile(std::string_view shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_geometryShaderSource))
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescription::setGeometryShaderFromFile could not read file!");
            return false;
        }

        return true;
    }

    bool EffectDescriptionImpl::addCompilerDefine(std::string_view define)
    {
        if (define.empty())
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescription::addCompilerDefine cannot add empty define!");
            return false;
        }

        m_compilerDefines.emplace_back(define);
        return true;
    }

    bool EffectDescriptionImpl::setSemantic(std::string_view semanticName, EFixedSemantics semanticType)
    {
        if (semanticName.empty())
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescription::setSemantic cannot set empty semantic name!");
            return false;
        }

        m_inputSemantics[std::string{semanticName}] = semanticType;
        return true;
    }

    bool EffectDescriptionImpl::setSemantic(uint32_t uniformBufferBinding, EFixedSemantics semanticType)
    {
        m_inputSemantics[UniformBufferBinding{ uniformBufferBinding }] = semanticType;
        return true;
    }

    bool EffectDescriptionImpl::setUniformSemantic(std::string_view semanticName, EEffectUniformSemantic semanticType)
    {
        const EFixedSemantics semanticTypeInternal = EffectInputSemanticUtils::GetEffectInputSemanticInternal(semanticType);
        return setSemantic(semanticName, semanticTypeInternal);
    }

    bool EffectDescriptionImpl::setUniformSemantic(uint32_t uniformBufferBinding, EEffectUniformSemantic semanticType)
    {
        const EFixedSemantics semanticTypeInternal = EffectInputSemanticUtils::GetEffectInputSemanticInternal(semanticType);
        return setSemantic(uniformBufferBinding, semanticTypeInternal);
    }

    bool EffectDescriptionImpl::setAttributeSemantic(std::string_view semanticName, EEffectAttributeSemantic semanticType)
    {
        const EFixedSemantics semanticTypeInternal = EffectInputSemanticUtils::GetEffectInputSemanticInternal(semanticType);
        return setSemantic(semanticName, semanticTypeInternal);
    }

    const char* EffectDescriptionImpl::getVertexShader() const
    {
        return m_vertexShaderSource.c_str();
    }

    const char* EffectDescriptionImpl::getFragmentShader() const
    {
        return m_fragmentShaderSource.c_str();
    }

    const char* EffectDescriptionImpl::getGeometryShader() const
    {
        return m_geometryShaderSource.c_str();
    }

    size_t EffectDescriptionImpl::getNumberOfCompilerDefines() const
    {
        return m_compilerDefines.size();
    }

    const std::vector<std::string>& EffectDescriptionImpl::getCompilerDefines() const
    {
        return m_compilerDefines;
    }

    const char* EffectDescriptionImpl::getCompilerDefine(size_t index) const
    {
        if (index < getNumberOfCompilerDefines())
        {
            return m_compilerDefines[index].c_str();
        }

        return nullptr;
    }

    const SemanticsMap& EffectDescriptionImpl::getSemanticsMap() const
    {
        return m_inputSemantics;
    }

    bool EffectDescriptionImpl::ReadFileContentsToString(std::string_view fileName, std::string& fileContents)
    {
        File inFile(fileName);
        if (!inFile.exists())
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  could not find file: {}", fileName);
            return false;
        }

        if (!inFile.open(File::Mode::ReadOnlyBinary))
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  could not open file: {}", fileName);
            return false;
        }

        size_t fileSize = 0;
        size_t readBytes = 0;
        if (!inFile.getSizeInBytes(fileSize))
        {
            LOG_ERROR(CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  error reading file info: {}", fileName);
            return false;
        }

        std::vector<char> charVector(fileSize + 1u);
        const EStatus stat = inFile.read(&charVector[0], fileSize, readBytes);
        if (stat == EStatus::Ok || stat == EStatus::Eof)
        {
            charVector[readBytes] = '\0';
            fileContents = &charVector[0];
            return true;
        }
        LOG_ERROR(CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  error reading file contents: {}", fileName);
        return false;
    }
}
