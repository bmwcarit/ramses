//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/UniformInput.h"
#include "EffectInputImpl.h"

namespace ramses
{
    UniformInput::UniformInput()
        : EffectInput{ std::make_unique<EffectInputImpl>() }
    {
    }

    UniformInput::~UniformInput() = default;

    UniformInput::UniformInput(const UniformInput& other)
        : EffectInput{ std::make_unique<EffectInputImpl>(other.m_impl) }
    {
    }

    UniformInput::UniformInput(UniformInput&& other) noexcept
        : EffectInput{ std::unique_ptr<EffectInputImpl>(static_cast<EffectInputImpl*>(other.StatusObject::m_impl.release())) }
    {
    }

    UniformInput& UniformInput::operator=(const UniformInput& other)
    {
        StatusObject::m_impl = std::make_unique<EffectInputImpl>(other.m_impl);
        m_impl = static_cast<EffectInputImpl&>(*StatusObject::m_impl);
        return *this;
    }

    UniformInput& UniformInput::operator=(UniformInput&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<EffectInputImpl&>(*StatusObject::m_impl);
        return *this;
    }

    EEffectUniformSemantic UniformInput::getSemantics() const
    {
        return m_impl.get().getUniformSemantics();
    }

    size_t UniformInput::getElementCount() const
    {
        return m_impl.get().getElementCount();
    }
}
