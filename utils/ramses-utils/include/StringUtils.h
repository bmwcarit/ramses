//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_STRINGUTILS_H
#define RAMSES_UTILS_STRINGUTILS_H

#include "Collections/String.h"
#include "Collections/Vector.h"

class StringUtils
{
public:
    static void GetLineTokens(const ramses_internal::String& line, char split, std::vector<ramses_internal::String>& tokens);
};

#endif
