//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/UniformInput.h"
#include "impl/EffectInputImpl.h"

namespace ramses
{
    UniformInput::UniformInput()
        : EffectInput{ std::make_unique<internal::EffectInputImpl>() }
    {
    }

    EEffectUniformSemantic UniformInput::getSemantics() const
    {
        return m_impl->getUniformSemantics();
    }

    size_t UniformInput::getElementCount() const
    {
        return m_impl->getElementCount();
    }
}
