//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "UniformInputWrapper.h"

#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"

UniformInputWrapper::UniformInputWrapper(const char* inputName, ramses::Appearance& appearance)
    :m_appearance(appearance)
{
    m_appearance.getEffect().findUniformInput(inputName, m_input);
}

void UniformInputWrapper::setInputValueFloat(float x)
{
    m_appearance.setInputValue(m_input, x);
}



