//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_PICKABLEOBJECT_H
#define RAMSES_INTERNAL_PICKABLEOBJECT_H

#include "SceneAPI/SceneTypes.h"

namespace ramses_internal
{
    struct PickableObject
    {
        DataBufferHandle geometryHandle;
        NodeHandle nodeHandle;
        CameraHandle cameraHandle;
        PickableObjectId id;
        bool isEnabled = true;
    };

    static_assert(std::is_nothrow_move_constructible<PickableObject>::value, "PickableObject must be movable");
    static_assert(std::is_nothrow_move_assignable<PickableObject>::value, "PickableObject must be movable");
}

#endif
