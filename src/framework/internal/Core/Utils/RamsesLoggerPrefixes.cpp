//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/RamsesLoggerPrefixes.h"
#include "impl/RamsesLoggerImpl.h"

namespace ramses::internal
{
    void RamsesLoggerPrefixes::SetRamsesLoggerPrefixes(std::string_view instance, std::string_view thread, std::string_view additional)
    {
        RamsesLoggerPrefixesExported::SetRamsesLoggerPrefixesExported(instance, thread, additional);
        RamsesLogger::SetPrefixes(instance, thread, additional);
    }

    void RamsesLoggerPrefixes::SetRamsesLoggerPrefixAdditional(std::string_view additional)
    {
        RamsesLoggerPrefixesExported::SetRamsesLoggerPrefixAdditionalExported(additional);
        RamsesLogger::SetPrefixAdditional(additional);
    }
}
