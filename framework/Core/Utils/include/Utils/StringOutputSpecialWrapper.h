//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_STRINGOUTPUTSPECIALWRAPPER_H
#define RAMSES_UTILS_STRINGOUTPUTSPECIALWRAPPER_H

#include "Common/StronglyTypedValue.h"

#include "ramses-framework-api/RamsesFrameworkTypes.h"

#define MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(stronglyType, compatibleOutputType) \
    MAKE_STRONGLYTYPEDVALUE_PRINTABLE(stronglyType)

#endif
