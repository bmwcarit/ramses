//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TYPES_GL_H
#define RAMSES_TYPES_GL_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    // GLHandle is not strongly typed because most of the time its scope is limited and involves GL call
    // and it is stored in GPUResource as platform independent type, thus would require conversion every time it is used
    using GLHandle = UInt32;
    static const GLHandle InvalidGLHandle(0u);

    struct GLInputLocationTag {};
    using GLInputLocation = StronglyTypedValue<Int32, -1, GLInputLocationTag>;
    static const GLInputLocation GLInputLocationInvalid(-1);

    using TextureSlot = Int32;
}

#endif
