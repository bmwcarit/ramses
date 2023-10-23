//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Math3d/ProjectionParams.h"
#include "internal/Core/Common/TypedMemoryHandle.h"
#include "internal/PlatformAbstraction/PlatformMath.h"
#include <cassert>

namespace ramses::internal
{
    bool ProjectionParams::operator==(const ProjectionParams& other) const
    {
        return m_projectionType == other.getProjectionType()
            && leftPlane == other.leftPlane
            && rightPlane == other.rightPlane
            && bottomPlane == other.bottomPlane
            && topPlane == other.topPlane
            && nearPlane == other.nearPlane
            && farPlane == other.farPlane;
    }

    bool ProjectionParams::operator!=(const ProjectionParams& other) const
    {
        return !(*this == other);
    }

    ProjectionParams ProjectionParams::Perspective(float fieldOfViewY, float aspectRatio, float nearPlane, float farPlane)
    {
        ProjectionParams params;
        params.m_projectionType = ECameraProjectionType::Perspective;
        params.nearPlane = nearPlane;
        params.farPlane = farPlane;

        const float tangent = std::tan(PlatformMath::Deg2Rad(fieldOfViewY / 2.0f));
        params.topPlane = nearPlane * tangent;
        params.bottomPlane = -params.topPlane;
        params.rightPlane = params.topPlane * aspectRatio;
        params.leftPlane = -params.rightPlane;

        return params;
    }

    ProjectionParams ProjectionParams::Frustum(ECameraProjectionType projectionType, float leftPlane, float rightPlane,
        float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        ProjectionParams params;
        params.m_projectionType = projectionType;
        params.leftPlane = leftPlane;
        params.rightPlane = rightPlane;
        params.bottomPlane = bottomPlane;
        params.topPlane = topPlane;
        params.nearPlane = nearPlane;
        params.farPlane = farPlane;

        return params;
    }

    float ProjectionParams::GetPerspectiveFovY(const ProjectionParams& projectionParams)
    {
        assert(projectionParams.m_projectionType == ECameraProjectionType::Perspective);
        return PlatformMath::Rad2Deg(std::atan2(std::abs(projectionParams.topPlane), projectionParams.nearPlane) +
                                     std::atan2(std::abs(projectionParams.bottomPlane), projectionParams.nearPlane));
    }

    float ProjectionParams::GetAspectRatio(const ProjectionParams& projectionParams)
    {
        return (projectionParams.rightPlane - projectionParams.leftPlane) / (projectionParams.topPlane - projectionParams.bottomPlane);
    }

    ECameraProjectionType ProjectionParams::getProjectionType() const
    {
        return m_projectionType;
    }

    bool ProjectionParams::isValid() const
    {
        return leftPlane < rightPlane
            && bottomPlane < topPlane
            && nearPlane < farPlane
            && nearPlane > std::numeric_limits<float>::epsilon();
    }
}
