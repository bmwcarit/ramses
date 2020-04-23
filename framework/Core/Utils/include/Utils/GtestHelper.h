//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_GTESTHELPER_H
#define RAMSES_UTILS_GTESTHELPER_H

#include "gtest/gtest.h"
#include <cassert>

/// Helper macro when clang-tidy does not understand that execution does not continue on nullptr
#define ASSERT_NOT_NULL(ptr)     \
    do                           \
    {                            \
        ASSERT_NE(ptr, nullptr); \
        assert(ptr);             \
    }  while (0)

#endif
