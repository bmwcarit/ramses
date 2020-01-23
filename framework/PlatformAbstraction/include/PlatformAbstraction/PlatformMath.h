//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMMATH_H
#define RAMSES_PLATFORMMATH_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <cmath>
#include <cstdlib>

#undef min
#undef max

namespace ramses_internal
{

    template<class T>
    const T& min(const T& a, const T& b)
    {
        return (b < a) ? b : a;
    }

    template<class T>
    const T& max(const T& a, const T& b)
    {
        return (b > a) ? b : a;
    }

    template<class T>
    const T& clamp(const T& val, const T& minVal, const T& maxVal)
    {
        return (min(max(minVal, val), maxVal));
    }

    class PlatformMath
    {
    public:

        static const Float  PI_f;
        static const Double PI_d;

        static Float  Rad2Deg(Float val);
        static Double Rad2Deg(Double val);
        static Float  Deg2Rad(Float val);
        static Double Deg2Rad(Double val);
    };

    inline float PlatformMath::Rad2Deg(float val)
    {
        return val * (180.f / PI_f);
    }

    inline double PlatformMath::Rad2Deg(double val)
    {
        return val * (180.0 / PI_d);
    }

    inline float PlatformMath::Deg2Rad(float val)
    {
        return val * (PI_f / 180.f);
    }

    inline double PlatformMath::Deg2Rad(double val)
    {
        return val * (PI_d / 180.0);
    }
}

#endif
