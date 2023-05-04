//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"

// internal
#include "EffectImpl.h"

namespace ramses
{
    Effect::Effect(std::unique_ptr<EffectImpl> impl)
        : Resource{ std::move(impl) }
        , m_impl{ static_cast<EffectImpl&>(Resource::m_impl) }
    {
    }

    uint32_t Effect::getUniformInputCount() const
    {
        return m_impl.getUniformInputCount();
    }

    uint32_t Effect::getAttributeInputCount() const
    {
        return m_impl.getAttributeInputCount();
    }

    status_t Effect::getUniformInput(uint32_t index, UniformInput& uniformInput) const
    {
        return m_impl.getUniformInput(index, uniformInput.m_impl);
    }

    status_t Effect::findUniformInput(EEffectUniformSemantic uniformSemantic, UniformInput& uniformInput) const
    {
        return m_impl.findUniformInput(uniformSemantic, uniformInput.m_impl);
    }

    status_t Effect::getAttributeInput(uint32_t index, AttributeInput& attributeInput) const
    {
        return m_impl.getAttributeInput(index, attributeInput.m_impl);
    }

    status_t Effect::findAttributeInput(EEffectAttributeSemantic attributeSemantic, AttributeInput& attributeInput) const
    {
        return m_impl.findAttributeInput(attributeSemantic, attributeInput.m_impl);
    }

    bool Effect::hasGeometryShader() const
    {
        return m_impl.hasGeometryShader();
    }

    status_t Effect::getGeometryShaderInputType(EDrawMode& expectedGeometryInputType) const
    {
        return m_impl.getGeometryShaderInputType(expectedGeometryInputType);
    }

    status_t Effect::findUniformInput(const char* inputName, UniformInput& uniformInput) const
    {
        return m_impl.findUniformInput(inputName, uniformInput.m_impl);
    }

    status_t Effect::findAttributeInput(const char* inputName, AttributeInput& attributeInput) const
    {
        return m_impl.findAttributeInput(inputName, attributeInput.m_impl);
    }
}
