//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/ERotationType.h"
#include "ramses-logic/EPropertyType.h"

#include "ramses-client-api/ERotationConvention.h"

#include <memory>
#include <optional>

namespace ramses
{
    enum class ERotationConvention;
}

namespace rlogic::internal
{
    class RotationUtils
    {
    public:
        static vec3f QuaternionToEulerXYZDegrees(vec4f quaternion);

        static std::optional<ERotationType> RamsesRotationConventionToRotationType(ramses::ERotationConvention convention);
        static std::optional<ramses::ERotationConvention> RotationTypeToRamsesRotationConvention(ERotationType rotationType);

        static constexpr const float  PI_f = 3.1415926535897932384626433832795028841971693993751058209749f;

        // Roughly corresponds to the estimated numeric loss from sin/cosine operations combined with the angle/degree conversion
        // needed to match ramses degree semantics
        static constexpr const float  ConversionPrecision = 0.00006f;

        inline static constexpr float Rad2Deg(float val)
        {
            return val * (180.f / PI_f);
        }
    };
}
