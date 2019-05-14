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

    status_t EffectDescriptionImpl::setVertexShader(const char* shaderSource)
    {
        m_vertexShaderSource = shaderSource;
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setFragmentShader(const char* shaderSource)
    {
        m_fragmentShaderSource = shaderSource;
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setVertexShaderFromFile(const char* shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_vertexShaderSource))
        {
            return addErrorEntry("EffectDescription::setVertexShaderFromFile could not read file!");
        }

        return StatusOK;
    }

    status_t EffectDescriptionImpl::setFragmentShaderFromFile(const char* shaderSourceFileName)
    {
        if (!ReadFileContentsToString(shaderSourceFileName, m_fragmentShaderSource))
        {
            return addErrorEntry("EffectDescription::setFragmentShaderFromFile could not read file!");
        }

        return StatusOK;
    }

    status_t EffectDescriptionImpl::addCompilerDefine(const char* define)
    {
        const ramses_internal::String defineStr(define);
        if (defineStr.getLength() == 0u)
        {
            return addErrorEntry("EffectDescription::addCompilerDefine cannot add empty define!");
        }

        m_compilerDefines.push_back(defineStr);
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setSemantic(const char* semanticName, ramses_internal::EFixedSemantics semanticType)
    {
        const ramses_internal::String semanticNameStr(semanticName);
        if (semanticNameStr.getLength() == 0u)
        {
            return addErrorEntry("EffectDescription::setSemantic cannot set empty semantic name!");
        }

        m_inputSemantics.put(semanticNameStr, semanticType);
        return StatusOK;
    }

    status_t EffectDescriptionImpl::setUniformSemantic(const char* semanticName, EEffectUniformSemantic semanticType)
    {
        const ramses_internal::EFixedSemantics semanticTypeInternal = EffectInputSemanticUtils::GetEffectInputSemanticInternal(semanticType);
        return setSemantic(semanticName, semanticTypeInternal);
    }

    status_t EffectDescriptionImpl::setAttributeSemantic(const char* semanticName, EEffectAttributeSemantic semanticType)
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

    uint32_t EffectDescriptionImpl::getNumberOfCompilerDefines() const
    {
        return static_cast<uint32_t>(m_compilerDefines.size());
    }

    const ramses_internal::StringVector& EffectDescriptionImpl::getCompilerDefines() const
    {
        return m_compilerDefines;
    }

    const char* EffectDescriptionImpl::getCompilerDefine(uint32_t index) const
    {
        if (index < getNumberOfCompilerDefines())
        {
            return m_compilerDefines[index].c_str();
        }

        return NULL;
    }

    const EffectDescriptionImpl::SemanticsMap& EffectDescriptionImpl::getSemanticsMap() const
    {
        return m_inputSemantics;
    }

    bool EffectDescriptionImpl::ReadFileContentsToString(const char* fileName, ramses_internal::String& fileContents)
    {
        ramses_internal::File inFile(fileName);
        if (!inFile.exists())
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  could not find file: " << fileName);
            return false;
        }

        inFile.open(ramses_internal::EFileMode_ReadOnly);
        ramses_internal::UInt fileSize = 0;
        ramses_internal::UInt readBytes = 0;
        ramses_internal::EStatus stat = inFile.getSizeInBytes(fileSize);
        if (stat != ramses_internal::EStatus_RAMSES_OK)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "EffectDescriptionImpl::ReadFileContentsToString:  error reading file info: " << fileName);
            return false;
        }

        std::vector<char> charVector(fileSize + 1u);
        stat = inFile.read(&charVector[0], fileSize, readBytes);
        if (stat == ramses_internal::EStatus_RAMSES_OK || stat == ramses_internal::EStatus_RAMSES_EOF)
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
