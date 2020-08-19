//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_VARIANTWRAPPER_H
#define RAMSES_PLATFORMABSTRACTION_VARIANTWRAPPER_H

#include "Utils/Warnings.h"

// TODO (Violin/Tobias) Remove this warning suppression after fixed in Abseil upstream
// Warning occurs in the absl variant macro which assigns -1 to a size_t
WARNINGS_PUSH
WARNING_DISABLE_VC(4245)
#include "absl/types/variant.h"
WARNINGS_POP

#endif
