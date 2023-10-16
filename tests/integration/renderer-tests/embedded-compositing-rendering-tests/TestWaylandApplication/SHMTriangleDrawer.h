//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include "ETriangleColor.h"
#include <array>

namespace ramses::internal
{
    class SHMBuffer;

    class SHMTriangleDrawer
    {
    public:
        explicit SHMTriangleDrawer(ETriangleColor triangleColor);

        void draw(SHMBuffer* buffer);

    private:
        std::array<uint8_t, 4> m_triangleColorBGRA{};
    };
}
