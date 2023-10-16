//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/EffectInput.h"
#include "impl/EffectInputImpl.h"

namespace ramses
{
    EffectInput::~EffectInput() = default;

    EffectInput::EffectInput(std::unique_ptr<internal::EffectInputImpl> effectInputImpl)
        : m_impl{ std::move(effectInputImpl) }
    {
    }

    EffectInput::EffectInput(const EffectInput& other)
        : EffectInput{ std::make_unique<internal::EffectInputImpl>(*other.m_impl) }
    {
    }

    EffectInput::EffectInput(EffectInput&& other) noexcept = default;

    EffectInput& EffectInput::operator=(const EffectInput& other)
    {
        EffectInput::m_impl = std::make_unique<internal::EffectInputImpl>(*other.m_impl);
        return *this;
    }

    EffectInput& EffectInput::operator=(EffectInput&& other) noexcept = default;


    const char* EffectInput::getName() const
    {
        return m_impl->getName().c_str();
    }

    EDataType EffectInput::getDataType() const
    {
        return m_impl->getDataType();
    }

    internal::EffectInputImpl& EffectInput::impl()
    {
        return *m_impl;
    }

    const internal::EffectInputImpl& EffectInput::impl() const
    {
        return *m_impl;
    }
}
