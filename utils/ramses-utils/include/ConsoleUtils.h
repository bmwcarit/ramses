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

#define PRINT_ERROR( ...) ramses_internal::Console::Print(ramses_internal::ConsoleColor::Red, "erro: {}", fmt::format(__VA_ARGS__))
#define PRINT_INFO(...) ramses_internal::Console::Print(ramses_internal::ConsoleColor::Green,"info: {}", fmt::format(__VA_ARGS__))
#define PRINT_HINT(...) ramses_internal::Console::Print(ramses_internal::ConsoleColor::Yellow, "hint: {}", fmt::format(__VA_ARGS__))
#define PRINT(...) ramses_internal::Console::Print(ramses_internal::ConsoleColor::Cyan, "{}", fmt::format(__VA_ARGS__))

#endif
