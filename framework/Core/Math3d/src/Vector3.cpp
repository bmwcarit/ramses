//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{
    Vector3::Vector3(const Vector4& other)
        : x(other.x)
        , y(other.y)
        , z(other.z)
    {
    }
}
