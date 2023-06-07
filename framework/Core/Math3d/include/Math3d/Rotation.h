//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneAPI/ERotationType.h"
#include "DataTypesImpl.h"

namespace ramses_internal
{
    namespace Math3d
    {
        glm::mat4 Rotation(const glm::vec4& rotation, ERotationType rotationType);
    }
}
