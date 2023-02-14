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
    Effect::Effect(EffectImpl& pimpl)
        : Resource(pimpl)
        , impl(pimpl)
    {
    }

    Effect::~Effect()
    {
    }

    uint32_t Effect::getUniformInputCount() const
    {
        return impl.getUniformInputCount();
    }

    uint32_t Effect::getAttributeInputCount() const
    {
        return impl.getAttributeInputCount();
    }

    status_t Effect::getUniformInput(uint32_t index, UniformInput& uniformInput) const
    {
        return impl.getUniformInput(index, uniformInput.impl);
    }

    status_t Effect::findUniformInput(EEffectUniformSemantic uniformSemantic, UniformInput& uniformInput) const
    {
        return impl.findUniformInput(uniformSemantic, uniformInput.impl);
    }

    status_t Effect::getAttributeInput(uint32_t index, AttributeInput& attributeInput) const
    {
        return impl.getAttributeInput(index, attributeInput.impl);
    }

    status_t Effect::findAttributeInput(EEffectAttributeSemantic attributeSemantic, AttributeInput& attributeInput) const
    {
        return impl.findAttributeInput(attributeSemantic, attributeInput.impl);
    }

    bool Effect::hasGeometryShader() const
    {
        return impl.hasGeometryShader();
    }

    status_t Effect::getGeometryShaderInputType(EDrawMode& expectedGeometryInputType)
    {
        return impl.getGeometryShaderInputType(expectedGeometryInputType);
    }

    status_t Effect::findUniformInput(const char* inputName, UniformInput& uniformInput) const
    {
        return impl.findUniformInput(inputName, uniformInput.impl);
    }

    status_t Effect::findAttributeInput(const char* inputName, AttributeInput& attributeInput) const
    {
        return impl.findAttributeInput(inputName, attributeInput.impl);
    }
}
