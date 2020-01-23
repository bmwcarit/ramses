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
#include "Math3d/Vector3.h"
#include "Math3d/Vector2.h"
#include "TransformationLinkCachedScene.h"

namespace ramses_internal
{
    class IntersectionUtils
    {
    public:
        struct Triangle
        {
            Vector3 v0;
            Vector3 v1;
            Vector3 v2;
        };

        static Vector3 CalculatePlaneNormal(const Triangle& triangle);
        static bool IntersectRayVsTriangle(const Triangle& triangle, const Vector3& rayOrigin, const Vector3& rayDir, Vector3& intersectionPointInModelSpace, float& distanceRayOriginToIntersection);
        static bool TestGeometryPicked(const Vector2& pickCoordsNDS, const float* geometry, const size_t geometrySize, const Matrix44f& modelMatrix, const Matrix44f& viewMatrix, const Matrix44f& projectionMatrix, Vector3& intersectionPointInModelSpace);
        static void CheckSceneForIntersectedPickableObjects(const TransformationLinkCachedScene& scene, const Vector2i coordsInBufferSpace, PickableObjectIds& pickedObjects);

    private:
        static bool TestPointInTriangle(const Triangle& triangle, const Vector3& planeNormal, const Vector3& testPoint);
        static bool CalculateRayVsPlaneIntersection(const Vector3& triangleVertex,
            const Vector3& triangleNormal,
            const Vector3& rayOrigin,
            const Vector3& rayDir,
            Vector3& intersection,
            float& distanceRayOriginToIntersection);
    };
}
#endif
