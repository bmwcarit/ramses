//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_ASSERTMOVABLE_H
#define RAMSES_UTILS_ASSERTMOVABLE_H

#define INT_RAMSES_ASSERT_MOVABLE(name) \
    static_assert(std::is_move_constructible<name>::value, #name " must be movable"); \
    static_assert(std::is_move_assignable<name>::value, #name " must be movable");

#define INT_RAMSES_ASSERT_NOTHROW_MOVABLE(name) \
    static_assert(std::is_nothrow_move_constructible<name>::value, #name " must be movable"); \
    static_assert(std::is_nothrow_move_assignable<name>::value, #name " must be movable");

// C++14 does not require noexcept move constructor/assignments for std::vector moves
// But with the exception of ghs compiler all std libs of our supported compilers
// guarantee noexcept for this, so we can assert no throw movable for our objects
#ifdef __ghs__
#   define ASSERT_MOVABLE(x) INT_RAMSES_ASSERT_MOVABLE(x)
#else
#   define ASSERT_MOVABLE(x) INT_RAMSES_ASSERT_NOTHROW_MOVABLE(x)
#endif

#endif
