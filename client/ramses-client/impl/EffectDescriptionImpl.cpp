//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// internal
#include "EffectDescriptionImpl.h"
#include "EffectInputSemanticUtils.h"

// framework
#include "Utils/File.h"
#include "Utils/LogMacros.h"


namespace ramses
{
    EffectDescriptionImpl::EffectDescriptionImpl()
    {
    }

    EffectDescriptionImpl::~EffectDescriptionImpl()
    {
    }

    status_t EffectDescriptionImpl::setVertexShader(std::string_view shaderSource)
    {
        m_vertexShaderSource = shaderSource;
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setFragmentShader(std::string_view shaderSource)
    {
        m_fragmentShaderSource = shaderSource;
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setGeometryShader(std::string_view shaderSource)
    {
        m_geometryShaderSource = shaderSource;
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setVertexShaderFromFile(std::string_view shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_vertexShaderSource))
        {
            return addErrorEntry("EffectDescription::setVertexShaderFromFile could not read file!");
        }

        return StatusOK;
    }

    status_t EffectDescriptionImpl::setFragmentShaderFromFile(std::string_view shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_fragmentShaderSource))
        {
            return addErrorEntry("EffectDescription::setFragmentShaderFromFile could not read file!");
        }

        return StatusOK;
    }

    ramses::status_t EffectDescriptionImpl::setGeometryShaderFromFile(std::string_view shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_geometryShaderSource))
        {
            return addErrorEntry("EffectDescription::setGeometryShaderFromFile could not read file!");
        }

        return StatusOK;
    }

    status_t EffectDescriptionImpl::addCompilerDefine(std::string_view define)
    {
        if (define.empty())
        {
            return addErrorEntry("EffectDescription::addCompilerDefine cannot add empty define!");
        }

        m_compilerDefines.emplace_back(define);
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setSemantic(std::string_view semanticName, ramses_internal::EFixedSemantics semanticType)
    {
        if (semanticName.empty())
        {
            return addErrorEntry("EffectDescription::setSemantic cannot set empty semantic name!");
        }

        m_inputSemantics.put(std::string{semanticName}, semanticType);
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setUniformSemantic(std::string_view semanticName, EEffectUniformSemantic semanticType)
    {
        const ramses_internal::EFixedSemantics semanticTypeInternal = EffectInputSemanticUtils::GetEffectInputSemanticInternal(semanticType);
        return setSemantic(semanticName, semanticTypeInternal);
    }

    status_t EffectDescriptionImpl::setAttributeSemantic(std::string_view semanticName, EEffectAttributeSemantic semanticType)
    {
        const ramses_internal::EFixedSemantics semanticTypeInternal = EffectInputSemanticUtils::GetEffectInputSemanticInternal(semanticType);
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

    const EffectDescriptionImpl::SemanticsMap& EffectDescriptionImpl::getSemanticsMap() const
    {
        return m_inputSemantics;
    }

    bool EffectDescriptionImpl::ReadFileContentsToString(std::string_view fileName, std::string& fileContents)
    {
        ramses_internal::File inFile(fileName);
        if (!inFile.exists())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  could not find file: " << fileName);
            return false;
        }

        if (!inFile.open(ramses_internal::File::Mode::ReadOnlyBinary))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  could not open file: " << fileName);
            return false;
        }

        size_t fileSize = 0;
        size_t readBytes = 0;
        if (!inFile.getSizeInBytes(fileSize))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  error reading file info: " << fileName);
            return false;
        }

        std::vector<char> charVector(fileSize + 1u);
        const ramses_internal::EStatus stat = inFile.read(&charVector[0], fileSize, readBytes);
        if (stat == ramses_internal::EStatus::Ok || stat == ramses_internal::EStatus::Eof)
        {
            charVector[readBytes] = '\0';
            fileContents = &charVector[0];
            return true;
        }
        else
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  error reading file contents: " << fileName);
            return false;
        }
    }
}
