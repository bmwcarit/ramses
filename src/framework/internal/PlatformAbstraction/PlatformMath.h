//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cmath>

namespace ramses::internal
{
    namespace PlatformMath
    {
        static constexpr const float  PI_f = 3.1415926535897932384626433832795028841971693993751058209749f;
        static constexpr const double PI_d = 3.1415926535897932384626433832795028841971693993751058209749;

        inline constexpr float Rad2Deg(float val)
        {
            return val * (180.f / PI_f);
        }

        inline constexpr double Rad2Deg(double val)
        {
            return val * (180.0 / PI_d);
        }

        inline constexpr float Deg2Rad(float val)
        {
            return val * (PI_f / 180.f);
        }

        inline constexpr double Deg2Rad(double val)
        {
            return val * (PI_d / 180.0);
        }
    };
}
