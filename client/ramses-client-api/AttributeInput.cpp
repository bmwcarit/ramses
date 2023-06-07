//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/AttributeInput.h"
#include "EffectInputImpl.h"

namespace ramses
{
    AttributeInput::AttributeInput()
        : EffectInput{ std::make_unique<EffectInputImpl>() }
    {
    }

    AttributeInput::~AttributeInput() = default;

    AttributeInput::AttributeInput(const AttributeInput& other)
        : EffectInput{ std::make_unique<EffectInputImpl>(other.m_impl) }
    {
    }

    AttributeInput::AttributeInput(AttributeInput&& other) noexcept
        : EffectInput{ std::unique_ptr<EffectInputImpl>(static_cast<EffectInputImpl*>(other.StatusObject::m_impl.release())) }
    {
    }

    AttributeInput& AttributeInput::operator=(const AttributeInput& other)
    {
        StatusObject::m_impl = std::make_unique<EffectInputImpl>(other.m_impl);
        m_impl = static_cast<EffectInputImpl&>(*StatusObject::m_impl);
        return *this;
    }

    AttributeInput& AttributeInput::operator=(AttributeInput&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<EffectInputImpl&>(*StatusObject::m_impl);
        return *this;
    }

    EEffectAttributeSemantic AttributeInput::getSemantics() const
    {
        return m_impl.get().getAttributeSemantics();
    }
}
