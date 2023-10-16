//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "UniformInputWrapper.h"

#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"

UniformInputWrapper::UniformInputWrapper(const char* inputName, ramses::Appearance& appearance)
    : m_input{appearance.getEffect().findUniformInput(inputName)}
    , m_appearance{appearance}
{
    assert(m_input.has_value());
}

void UniformInputWrapper::setInputValueFloat(float x)
{
    m_appearance.setInputValue(*m_input, x);
}



