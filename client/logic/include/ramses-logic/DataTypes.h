//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <array>

namespace rlogic
{
    using vec2f = std::array<float, 2>;
    using vec3f = std::array<float, 3>;
    using vec4f = std::array<float, 4>;
    using vec2i = std::array<int, 2>;
    using vec3i = std::array<int, 3>;
    using vec4i = std::array<int, 4>;

    /// Data type to hold float matrix 4x4 values. Stored in column-major shape (the first 4 values correspond to the first matrix column)
    using matrix44f = std::array<float, 16>;
}
