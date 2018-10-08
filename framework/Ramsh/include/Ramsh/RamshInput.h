//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHINPUT_H
#define RAMSES_RAMSHINPUT_H

#include "Utils/StringUtils.h"

namespace ramses_internal
{

class RamshInput : private StringVector
{
public:
    RamshInput();

    Bool isValid() const;
    String toString() const;
    const String& operator[](const UInt index) const;
    String& operator[](const UInt index);
    Bool operator==(const RamshInput& other) const;

    void append(const String& c);
    void append(const Char* c);
    void append(const Char c);

    using StringVector::size;
    using StringVector::clear;

};

}// namespace ramses_internal

#endif
