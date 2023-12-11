//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "internal/Core/Utils/RamsesLogger.h"

namespace ramses::internal
{
    class RamsesLogger;

    RAMSES_IMPL_EXPORT RamsesLogger& GetRamsesLogger();
}
