//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Math3d/ProjectionParams.h"
#include "Common/TypedMemoryHandle.h"
#include "PlatformAbstraction/PlatformMath.h"
#include <cassert>

namespace ramses_internal
{
    ProjectionParams::ProjectionParams()
    {
    }

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

    ProjectionParams ProjectionParams::Perspective(Float fieldOfViewY, Float aspectRatio, Float nearPlane, Float farPlane)
    {
        ProjectionParams params;
        params.m_projectionType = ECameraProjectionType::Perspective;
        params.nearPlane = nearPlane;
        params.farPlane = farPlane;

        const Float tangent = std::tan(PlatformMath::Deg2Rad(fieldOfViewY / 2.0f));
        params.topPlane = nearPlane * tangent;
        params.bottomPlane = -params.topPlane;
        params.rightPlane = params.topPlane * aspectRatio;
        params.leftPlane = -params.rightPlane;

        return params;
    }

    ProjectionParams ProjectionParams::Frustum(ECameraProjectionType projectionType, Float leftPlane, Float rightPlane,
        Float bottomPlane, Float topPlane, Float nearPlane, Float farPlane)
    {
        assert(projectionType != ECameraProjectionType::Renderer);

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

    Float ProjectionParams::GetPerspectiveFovY(const ProjectionParams& projectionParams)
    {
        assert(projectionParams.m_projectionType == ECameraProjectionType::Perspective);
        return PlatformMath::Rad2Deg(std::atan2(std::abs(projectionParams.topPlane), projectionParams.nearPlane) +
                                     std::atan2(std::abs(projectionParams.bottomPlane), projectionParams.nearPlane));
    }

    Float ProjectionParams::GetAspectRatio(const ProjectionParams& projectionParams)
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
