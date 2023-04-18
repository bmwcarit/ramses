//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/quaternion.hpp"

namespace rlogic
{
    using vec2f = glm::vec2;
    using vec3f = glm::vec3;
    using vec4f = glm::vec4;
    using vec2i = glm::ivec2;
    using vec3i = glm::ivec3;
    using vec4i = glm::ivec4;
    /// Data type to hold float matrix 4x4 values. Stored in column-major shape (the first 4 values correspond to the first matrix column)
    using matrix44f = glm::mat4;
    using quat = glm::quat;
}
