//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UNIFORMINPUTWRAPPER_H
#define RAMSES_UNIFORMINPUTWRAPPER_H

#include "ramses-client-api/UniformInput.h"

namespace ramses
{
    class Appearance;
}

class UniformInputWrapper
{
public:
    UniformInputWrapper(const char* inputName, ramses::Appearance& appearance);

    void setInputValueFloat(float x);

private:
    ramses::UniformInput m_input;
    ramses::Appearance& m_appearance;

};

#endif
