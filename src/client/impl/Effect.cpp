//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"

// internal
#include "impl/EffectImpl.h"

namespace ramses
{
    Effect::Effect(std::unique_ptr<internal::EffectImpl> impl)
        : Resource{ std::move(impl) }
        , m_impl{ static_cast<internal::EffectImpl&>(Resource::m_impl) }
    {
    }

    size_t Effect::getUniformInputCount() const
    {
        return m_impl.getUniformInputCount();
    }

    size_t Effect::getAttributeInputCount() const
    {
        return m_impl.getAttributeInputCount();
    }

    std::optional<UniformInput> Effect::getUniformInput(size_t index) const
    {
        return m_impl.getUniformInput(index);
    }

    std::optional<UniformInput> Effect::findUniformInput(EEffectUniformSemantic uniformSemantic) const
    {
        return m_impl.findUniformInput(uniformSemantic);
    }

    std::optional<AttributeInput> Effect::getAttributeInput(size_t index) const
    {
        return m_impl.getAttributeInput(index);
    }

    std::optional<AttributeInput> Effect::findAttributeInput(EEffectAttributeSemantic attributeSemantic) const
    {
        return m_impl.findAttributeInput(attributeSemantic);
    }

    bool Effect::hasGeometryShader() const
    {
        return m_impl.hasGeometryShader();
    }

    bool Effect::getGeometryShaderInputType(EDrawMode& expectedGeometryInputType) const
    {
        return m_impl.getGeometryShaderInputType(expectedGeometryInputType);
    }

    std::optional<UniformInput> Effect::findUniformInput(std::string_view inputName) const
    {
        return m_impl.findUniformInput(inputName);
    }

    std::optional<AttributeInput> Effect::findAttributeInput(std::string_view inputName) const
    {
        return m_impl.findAttributeInput(inputName);
    }

    internal::EffectImpl& Effect::impl()
    {
        return m_impl;
    }

    const internal::EffectImpl& Effect::impl() const
    {
        return m_impl;
    }
}
