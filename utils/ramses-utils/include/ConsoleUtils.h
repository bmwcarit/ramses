//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_CONSOLEUTILS_H
#define RAMSES_UTILS_CONSOLEUTILS_H

#include "PlatformAbstraction/PlatformConsole.h"

#define PRINT_ERROR(format, ...) ramses_internal::Console::Print(ramses_internal::Console::RED, "erro: " format, ##__VA_ARGS__)
#define PRINT_INFO(format, ...) ramses_internal::Console::Print(ramses_internal::Console::GREEN,"info: " format, ##__VA_ARGS__)
#define PRINT_HINT(format, ...) ramses_internal::Console::Print(ramses_internal::Console::YELLOW, "hint: " format, ##__VA_ARGS__)
#define PRINT(format, ...) ramses_internal::Console::Print(ramses_internal::Console::AQUA, format, ##__VA_ARGS__)

#endif
