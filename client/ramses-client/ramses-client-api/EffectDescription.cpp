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
        : StatusObject(*new EffectDescriptionImpl())
        , impl(static_cast<EffectDescriptionImpl&>(StatusObject::impl))
    {
    }

    status_t EffectDescription::setVertexShader(const char* shaderSource)
    {
        const status_t status = impl.setVertexShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource)
        return status;
    }

    status_t EffectDescription::setFragmentShader(const char* shaderSource)
    {
        const status_t status = impl.setFragmentShader(shaderSource);
        LOG_HL_CLIENT_API1(status, shaderSource)
        return status;
    }

    status_t EffectDescription::setVertexShaderFromFile(const char* shaderSourceFileName)
    {
        const status_t status = impl.setVertexShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName)
        return status;
    }

    status_t EffectDescription::setFragmentShaderFromFile(const char* shaderSourceFileName)
    {
        const status_t status = impl.setFragmentShaderFromFile(shaderSourceFileName);
        LOG_HL_CLIENT_API1(status, shaderSourceFileName)
        return status;
    }

    status_t EffectDescription::addCompilerDefine(const char* define)
    {
        const status_t status = impl.addCompilerDefine(define);
        LOG_HL_CLIENT_API1(status, define)
        return status;
    }

    status_t EffectDescription::setUniformSemantic(const char* inputName, EEffectUniformSemantic semanticType)
    {
        const status_t status = impl.setUniformSemantic(inputName, semanticType);
        LOG_HL_CLIENT_API2(status, inputName, semanticType)
        return status;
    }

    status_t EffectDescription::setAttributeSemantic(const char* inputName, EEffectAttributeSemantic semanticType)
    {
        const status_t status = impl.setAttributeSemantic(inputName, semanticType);
        LOG_HL_CLIENT_API2(status, inputName, semanticType)
        return status;
    }

    const char* EffectDescription::getVertexShader() const
    {
        return impl.getVertexShader();
    }

    const char* EffectDescription::getFragmentShader() const
    {
        return impl.getFragmentShader();
    }

    uint32_t EffectDescription::getNumberOfCompilerDefines() const
    {
        return impl.getNumberOfCompilerDefines();
    }

    const char* EffectDescription::getCompilerDefine(uint32_t index) const
    {
        return impl.getCompilerDefine(index);
    }
}
