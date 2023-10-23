//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ProjectionParams.h"
#include <cassert>

namespace ramses::internal
{
    class CameraMatrixHelper
    {
    public:
        static glm::mat4 ProjectionMatrix(const ProjectionParams& params)
        {
            const float l = params.leftPlane;
            const float r = params.rightPlane;
            const float b = params.bottomPlane;
            const float t = params.topPlane;
            const float n = params.nearPlane;
            const float f = params.farPlane;

            assert(std::abs(r - l) > std::numeric_limits<float>::epsilon());
            assert(std::abs(t - b) > std::numeric_limits<float>::epsilon());
            assert(std::abs(f - n) > std::numeric_limits<float>::epsilon());

            switch (params.getProjectionType())
            {
            case ECameraProjectionType::Orthographic:

                return glm::mat4(   {(2.0f / (r - l)), 0.f, 0.f, 0.f},
                                    {0.f, (2.0f / (t - b)), 0.f, 0.f},
                                    {0.f, 0.f, (-2.0f / (f - n)), 0.f},
                                    {(-(r + l) / (r - l)), (-(t + b) / (t - b)), (-(f + n) / (f - n)), 1.f});

            case ECameraProjectionType::Perspective:

                return glm::mat4(   {((2.0f * n) / (r - l)), 0.f, 0.f, 0.f},
                                    {0.f, ((2.0f * n) / (t - b)), 0.f, 0.f},
                                    {((r + l) / (r - l)), ((t + b) / (t - b)), (-(f + n) / (f - n)), -1.f},
                                    {0.f, 0.f, ((-2.0f * f * n) / (f - n)), 0.f});

            default:
                assert(false);
                return glm::identity<glm::mat4>();
            }
        }

    };
}
