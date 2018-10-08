//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CAMERAMATRIXHELPER_H
#define RAMSES_CAMERAMATRIXHELPER_H

#include "ProjectionParams.h"

namespace ramses_internal
{
    class CameraMatrixHelper
    {
    public:
        static Matrix44f ProjectionMatrix(const ProjectionParams& params)
        {
            const Float l = params.leftPlane;
            const Float r = params.rightPlane;
            const Float b = params.bottomPlane;
            const Float t = params.topPlane;
            const Float n = params.nearPlane;
            const Float f = params.farPlane;

            assert(PlatformMath::Abs(r - l) > std::numeric_limits<Float>::epsilon());
            assert(PlatformMath::Abs(t - b) > std::numeric_limits<Float>::epsilon());
            assert(PlatformMath::Abs(f - n) > std::numeric_limits<Float>::epsilon());

            switch (params.getProjectionType())
            {
            case ECameraProjectionType_Orthographic:

                return Matrix44f(
                    2.0f / (r - l), 0.f, 0.f, -(r + l) / (r - l),
                    0.f, 2.0f / (t - b), 0.f, -(t + b) / (t - b),
                    0.f, 0.f, -2.0f / (f - n), -(f + n) / (f - n),
                    0.f, 0.f, 0.f, 1.f
                );

            case ECameraProjectionType_Perspective:

                return Matrix44f(
                    (2.0f * n) / (r - l), 0.0f, (r + l) / (r - l), 0.0f
                    , 0.0f, (2.0f * n) / (t - b), (t + b) / (t - b), 0.0f
                    , 0.0f, 0.0f, -(f + n) / (f - n), (-2.0f * f * n) / (f - n)
                    , 0.0f, 0.0f, -1.0f, 0.0f
                );

            default:
                assert(false);
                return Matrix44f::Identity;
            }
        }

    };
}

#endif
