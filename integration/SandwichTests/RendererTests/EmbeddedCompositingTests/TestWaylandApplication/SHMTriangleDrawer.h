//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHMTRIANGLEDRAWER_H
#define RAMSES_SHMTRIANGLEDRAWER_H

#include "stdint.h"
#include "ETriangleColor.h"
class SHMBuffer;

namespace ramses_internal
{
    class SHMTriangleDrawer
    {
    public:
        SHMTriangleDrawer(ETriangleColor triangleColor);
        ~SHMTriangleDrawer();

        void draw(SHMBuffer* buffer);

    private:
        uint8_t m_triangleColorBGRA[4];
    };
}

#endif
