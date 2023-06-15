//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/EffectDescription.h"

// internal
#include "EffectDescriptionImpl.h"

namespace ramses
{
    EffectDescription::EffectDescription()
        : StatusObject{ std::make_unique<EffectDescriptionImpl>() }
        , m_impl{ static_cast<EffectDescriptionImpl&>(*StatusObject::m_impl) }
    {
    }

    EffectDescription::~EffectDescription() = default;

    EffectDescription::EffectDescription(const EffectDescription& other)
        : StatusObject{ std::make_unique<EffectDescriptionImpl>(other.m_impl) }
        , m_impl{ static_cast<EffectDescriptionImpl&>(*StatusObject::m_impl) }
    {
    }

    EffectDescription::EffectDescription(EffectDescription&& other) noexcept
        : StatusObject{ std::move(other.StatusObject::m_impl) }
        , m_impl{ static_cast<EffectDescriptionImpl&>(*StatusObject::m_impl) }
    {
    }

    EffectDescription& EffectDescription::operator=(const EffectDescription& other)
    {
        StatusObject::m_impl = std::make_unique<EffectDescriptionImpl>(other.m_impl);
        m_impl = static_cast<EffectDescriptionImpl&>(*StatusObject::m_impl);
        return *this;
    }

    EffectDescription& EffectDescription::operator=(EffectDescription&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<EffectDescriptionImpl&>(*StatusObject::m_impl);
        return *this;
    }

    status_t EffectDescription::setVertexShader(std::string_view shaderSource)
    {
        const status_t status = m_impl.get().setVertexShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource);
        return status;
    }

    status_t EffectDescription::setFragmentShader(std::string_view shaderSource)
    {
        const status_t status = m_impl.get().setFragmentShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource);
        return status;
    }

    status_t EffectDescription::setGeometryShader(std::string_view shaderSource)
    {
        const status_t status = m_impl.get().setGeometryShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource);
        return status;
    }

    status_t EffectDescription::setVertexShaderFromFile(std::string_view shaderSourceFileName)
    {
        const status_t status = m_impl.get().setVertexShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName);
        return status;
    }

    status_t EffectDescription::setFragmentShaderFromFile(std::string_view shaderSourceFileName)
    {
        const status_t status = m_impl.get().setFragmentShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName);
        return status;
    }

    status_t EffectDescription::setGeometryShaderFromFile(std::string_view shaderSourceFileName)
    {
        const status_t status = m_impl.get().setGeometryShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName);
        return status;
    }

    status_t EffectDescription::addCompilerDefine(std::string_view define)
    {
        const status_t status = m_impl.get().addCompilerDefine(define);
        LOG_HL_CLIENT_API1(status, define);
        return status;
    }

    status_t EffectDescription::setUniformSemantic(std::string_view inputName, EEffectUniformSemantic semanticType)
    {
        const status_t status = m_impl.get().setUniformSemantic(inputName, semanticType);
        LOG_HL_CLIENT_API2(status, inputName, semanticType);
        return status;
    }

    status_t EffectDescription::setAttributeSemantic(std::string_view inputName, EEffectAttributeSemantic semanticType)
    {
        const status_t status = m_impl.get().setAttributeSemantic(inputName, semanticType);
        LOG_HL_CLIENT_API2(status, inputName, semanticType);
        return status;
    }

    const char* EffectDescription::getVertexShader() const
    {
        return m_impl.get().getVertexShader();
    }

    const char* EffectDescription::getFragmentShader() const
    {
        return m_impl.get().getFragmentShader();
    }

    const char* EffectDescription::getGeometryShader() const
    {
        return m_impl.get().getGeometryShader();
    }

    size_t EffectDescription::getNumberOfCompilerDefines() const
    {
        return m_impl.get().getNumberOfCompilerDefines();
    }

    const char* EffectDescription::getCompilerDefine(size_t index) const
    {
        return m_impl.get().getCompilerDefine(index);
    }
}
