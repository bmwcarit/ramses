//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/EffectDescription.h"
#include "impl/APILoggingMacros.h"

// internal
#include "impl/EffectDescriptionImpl.h"

namespace ramses
{
    EffectDescription::EffectDescription()
        : m_impl{ std::make_unique<internal::EffectDescriptionImpl>() }
    {
    }

    EffectDescription::~EffectDescription() = default;

    EffectDescription::EffectDescription(const EffectDescription& other)
        : m_impl{ std::make_unique<internal::EffectDescriptionImpl>(*other.m_impl) }
    {
    }

    EffectDescription::EffectDescription(EffectDescription&& other) noexcept = default;

    EffectDescription& EffectDescription::operator=(const EffectDescription& other)
    {
        m_impl = std::make_unique<internal::EffectDescriptionImpl>(*other.m_impl);
        return *this;
    }

    EffectDescription& EffectDescription::operator=(EffectDescription&& other) noexcept = default;

    bool EffectDescription::setVertexShader(std::string_view shaderSource)
    {
        const auto status = m_impl->setVertexShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource.size());
        return status;
    }

    bool EffectDescription::setFragmentShader(std::string_view shaderSource)
    {
        const auto status = m_impl->setFragmentShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource.size());
        return status;
    }

    bool EffectDescription::setGeometryShader(std::string_view shaderSource)
    {
        const auto status = m_impl->setGeometryShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource.size());
        return status;
    }

    bool EffectDescription::setVertexShaderFromFile(std::string_view shaderSourceFileName)
    {
        const auto status = m_impl->setVertexShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName);
        return status;
    }

    bool EffectDescription::setFragmentShaderFromFile(std::string_view shaderSourceFileName)
    {
        const auto status = m_impl->setFragmentShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName);
        return status;
    }

    bool EffectDescription::setGeometryShaderFromFile(std::string_view shaderSourceFileName)
    {
        const auto status = m_impl->setGeometryShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName);
        return status;
    }

    bool EffectDescription::addCompilerDefine(std::string_view define)
    {
        const auto status = m_impl->addCompilerDefine(define);
        LOG_HL_CLIENT_API1(status, define);
        return status;
    }

    bool EffectDescription::setUniformSemantic(std::string_view inputName, EEffectUniformSemantic semanticType)
    {
        const auto status = m_impl->setUniformSemantic(inputName, semanticType);
        LOG_HL_CLIENT_API2(status, inputName, semanticType);
        return status;
    }

    bool EffectDescription::setAttributeSemantic(std::string_view inputName, EEffectAttributeSemantic semanticType)
    {
        const auto status = m_impl->setAttributeSemantic(inputName, semanticType);
        LOG_HL_CLIENT_API2(status, inputName, semanticType);
        return status;
    }

    const char* EffectDescription::getVertexShader() const
    {
        return m_impl->getVertexShader();
    }

    const char* EffectDescription::getFragmentShader() const
    {
        return m_impl->getFragmentShader();
    }

    const char* EffectDescription::getGeometryShader() const
    {
        return m_impl->getGeometryShader();
    }

    size_t EffectDescription::getNumberOfCompilerDefines() const
    {
        return m_impl->getNumberOfCompilerDefines();
    }

    const char* EffectDescription::getCompilerDefine(size_t index) const
    {
        return m_impl->getCompilerDefine(index);
    }

    internal::EffectDescriptionImpl& EffectDescription::impl()
    {
        return *m_impl;
    }

    const internal::EffectDescriptionImpl& EffectDescription::impl() const
    {
        return *m_impl;
    }
}
