//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORKTESTUTILS_TESTPNGHEADER_H
#define RAMSES_FRAMEWORKTESTUTILS_TESTPNGHEADER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <vector>

namespace ramses_internal
{
    namespace TestPngHeader
    {
        std::vector<Byte> GetValidHeader();
        std::vector<Byte> GetValidHeaderWithFakeData();
        std::vector<Byte> GetInvalidHeader();
    }
}

#endif
