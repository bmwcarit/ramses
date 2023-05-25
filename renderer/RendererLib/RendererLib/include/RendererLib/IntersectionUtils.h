//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERSECTIONUTILS_H
#define RAMSES_INTERSECTIONUTILS_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "TransformationLinkCachedScene.h"

namespace ramses_internal
{
    class IntersectionUtils
    {
    public:
        struct Triangle
        {
            glm::vec3 v0;
            glm::vec3 v1;
            glm::vec3 v2;
        };

        static glm::vec3 CalculatePlaneNormal(const Triangle& triangle);
        static bool IntersectRayVsTriangle(const Triangle& triangle, const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& intersectionPointInModelSpace, float& distanceRayOriginToIntersection);
        static bool TestGeometryPicked(const glm::vec2& pickCoordsNDS, const float* geometry, const size_t geometrySize, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3& intersectionPointInModelSpace);
        static void CheckSceneForIntersectedPickableObjects(const TransformationLinkCachedScene& scene, const glm::ivec2 coordsInBufferSpace, PickableObjectIds& pickedObjects);

    private:
        static bool TestPointInTriangle(const Triangle& triangle, const glm::vec3& planeNormal, const glm::vec3& testPoint);
        static bool CalculateRayVsPlaneIntersection(const glm::vec3& triangleVertex,
            const glm::vec3& triangleNormal,
            const glm::vec3& rayOrigin,
            const glm::vec3& rayDir,
            glm::vec3& intersection,
            float& distanceRayOriginToIntersection);
    };
}
#endif
