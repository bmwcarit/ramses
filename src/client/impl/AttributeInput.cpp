//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/AttributeInput.h"
#include "impl/EffectInputImpl.h"

namespace ramses
{
    AttributeInput::AttributeInput()
        : EffectInput{ std::make_unique<internal::EffectInputImpl>() }
    {
    }

    AttributeInput::~AttributeInput() = default;

    AttributeInput::AttributeInput(const AttributeInput& other) = default;

    AttributeInput::AttributeInput(AttributeInput&& other) noexcept = default;

    AttributeInput& AttributeInput::operator=(const AttributeInput& other) = default;

    AttributeInput& AttributeInput::operator=(AttributeInput&& other) noexcept = default;

    EEffectAttributeSemantic AttributeInput::getSemantics() const
    {
        return m_impl->getAttributeSemantics();
    }
}
