//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/StronglyTypedValue.h"

#include <cstdint>

namespace ramses::internal
{
    // GLHandle is not strongly typed because most of the time its scope is limited and involves GL call
    // and it is stored in GPUResource as platform independent type, thus would require conversion every time it is used
    using GLHandle = uint32_t;
    static const GLHandle InvalidGLHandle(0u);

    struct GLInputLocationTag {};
    using GLInputLocation = StronglyTypedValue<int32_t, -1, GLInputLocationTag>;

    using TextureSlot = int32_t;

    // TODO Violin fix this
    using GLenum = unsigned int;
}
