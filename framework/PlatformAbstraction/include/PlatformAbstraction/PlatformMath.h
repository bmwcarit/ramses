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
        static const Float  EPSILON;

        static const float LN2_f;
        static const double LN2_d;

        static Float  Ceil(Float  val);
        static Double Ceil(Double val);
        static Float  Floor(Float  val);
        static Double Floor(Double val);
        static Float  Abs(Float  val);
        static Double Abs(Double val);
        static Int    Abs(Int val);
        static Float  Sqrt(Float  val);
        static Double Sqrt(Double val);
        static Float  Pow2(Float  val);
        static Double Pow2(Double val);
        static Float  Pow(Float val, Float exp);
        static Double Pow(Double val, Double exp);
        static Float  Cos(Float  val);
        static Double Cos(Double val);
        static Float  Sin(Float  val);
        static Double Sin(Double val);
        static Float  Tan(Float  val);
        static Double Tan(Double val);
        static Float  ArcCos(Float val);
        static Double ArcCos(Double val);
        static Float  ArcSin(Float val);
        static Double ArcSin(Double val);
        static Float  ArcTan(Float val);
        static Double ArcTan(Double val);
        static Float  ArcTan2(Float valy, Float valx);
        static Double ArcTan2(Double valy, Double valx);

        static Float  Rad2Deg(Float val);
        static Double Rad2Deg(Double val);
        static Float  Deg2Rad(Float val);
        static Double Deg2Rad(Double val);

        static Float  Log2(Float val);
        static Double Log2(Double val);

        static Float  Exp(Float val);
        static Double Exp(Double val);
    private:

    };

    inline float PlatformMath::Ceil(float val)
    {
        return ::std::ceil(val);
    }

    inline double PlatformMath::Ceil(double val)
    {
        return ::std::ceil(val);
    }

    inline float PlatformMath::Floor(float val)
    {
        return ::std::floor(val);
    }

    inline double PlatformMath::Floor(double val)
    {
        return ::std::floor(val);
    }

    inline float PlatformMath::Abs(float val)
    {
        return ::std::fabs(val);
    }

    inline double PlatformMath::Abs(double val)
    {
        return ::std::fabs(val);
    }

    inline Int PlatformMath::Abs(Int val)
    {
        return ::std::abs(val);
    }

    inline float PlatformMath::Sqrt(float val)
    {
        return ::std::sqrt(val);
    }

    inline double PlatformMath::Sqrt(double val)
    {
        return ::std::sqrt(val);
    }

    inline float PlatformMath::Pow2(float val)
    {
        return val * val;
    }

    inline double PlatformMath::Pow2(double val)
    {
        return val * val;
    }

    inline float PlatformMath::Cos(float val)
    {
        return ::std::cos(val);
    }

    inline float PlatformMath::Pow(float val, float exponent)
    {
        return ::std::pow(val, exponent);
    }

    inline double PlatformMath::Pow(double val, double exponent)
    {
        return ::std::pow(val, exponent);
    }

    inline double PlatformMath::Cos(double val)
    {
        return ::std::cos(val);
    }

    inline float PlatformMath::Sin(float val)
    {
        return ::std::sin(val);
    }

    inline double PlatformMath::Sin(double val)
    {
        return ::std::sin(val);
    }

    inline float PlatformMath::Tan(float val)
    {
        return ::std::tan(val);
    }

    inline double PlatformMath::Tan(double val)
    {
        return ::std::tan(val);
    }

    inline float PlatformMath::ArcCos(float val)
    {
        return ::std::acos(val);
    }

    inline double PlatformMath::ArcCos(double val)
    {
        return ::std::acos(val);
    }

    inline float PlatformMath::ArcSin(float val)
    {
        return ::std::asin(val);
    }

    inline double PlatformMath::ArcSin(double val)
    {
        return ::std::asin(val);
    }

    inline float PlatformMath::ArcTan(float val)
    {
        return ::std::atan(val);
    }

    inline double PlatformMath::ArcTan(double val)
    {
        return ::std::atan(val);
    }


    inline Float  PlatformMath::ArcTan2(Float valy, Float valx)
    {
        return ::std::atan2(valy, valx);
    }

    inline Double PlatformMath::ArcTan2(Double valy, Double valx)
    {
        return ::std::atan2(valy, valx);
    }

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

    inline float PlatformMath::Log2(float val)
    {
        return ::std::log(val) / LN2_f;

    }

    inline double PlatformMath::Log2(double val)
    {
        return ::std::log(val) / LN2_d;
    }

    inline float PlatformMath::Exp(float val)
    {
        return ::std::exp(val);
    }

    inline double PlatformMath::Exp(double val)
    {
        return ::std::exp(val);
    }
}

#endif
