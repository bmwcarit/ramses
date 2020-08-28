//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/IntersectionUtils.h"
#include "Math3d/Matrix44f.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector4.h"
#include "Math3d/CameraMatrixHelper.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    Vector3 IntersectionUtils::CalculatePlaneNormal(const Triangle& triangle)
    {
        const Vector3 edge0 = triangle.v1 - triangle.v0;
        const Vector3 edge1 = triangle.v2 - triangle.v0;
        return edge0.cross(edge1);
    }

    bool IntersectionUtils::TestPointInTriangle(const Triangle& triangle, const Vector3& planeNormal, const Vector3& testPoint)
    {
        Vector3 VecPerpToPlane;
        const Vector3 edge0     = triangle.v1 - triangle.v0;
        const Vector3 v0Inters0 = testPoint - triangle.v0;
        VecPerpToPlane          = edge0.cross(v0Inters0);
        if (planeNormal.dot(VecPerpToPlane) < 0)
        {
            return false;
        }

        const Vector3 edge1     = triangle.v2 - triangle.v1;
        const Vector3 v1inters1 = testPoint - triangle.v1;
        VecPerpToPlane          = edge1.cross(v1inters1);
        if (planeNormal.dot(VecPerpToPlane) < 0)
        {
            return false;
        }

        const Vector3 edge2     = triangle.v0 - triangle.v2;
        const Vector3 v2inters2 = testPoint - triangle.v2;
        VecPerpToPlane          = edge2.cross(v2inters2);
        if (planeNormal.dot(VecPerpToPlane) < 0)
        {
            return false;
        }

        return true;
    }

    bool IntersectionUtils::CalculateRayVsPlaneIntersection(const Vector3& triangleVertex,
                                                const Vector3& triangleNormal,
                                                const Vector3& rayOrigin,
                                                const Vector3& rayDir,
                                                Vector3&       intersection,
                                                float& distanceRayOriginToIntersection)
    {
        // make sure rayDir is normalized
        assert(std::abs(rayDir.length() - 1) < std::numeric_limits<float>::epsilon() * 10);

        const float nDotRay = triangleNormal.dot(rayDir);
        if (std::abs(nDotRay) <= std::numeric_limits<float>::epsilon() * 10) // some epsilon that could still produce valid numbers later on
        {
            return false; // ray and triangle parallel
        }

        distanceRayOriginToIntersection = (triangleVertex - rayOrigin).dot(triangleNormal) / nDotRay;
        if (distanceRayOriginToIntersection < 0)
        {
            return false; // triangle behind ray
        }

        intersection = rayOrigin + distanceRayOriginToIntersection * rayDir;

        return true;
    }

    bool IntersectionUtils::IntersectRayVsTriangle(const Triangle& triangle, const Vector3& rayOrigin, const Vector3& rayDir, Vector3& intersectionPointInModelSpace, float& distanceRayOriginToIntersection)
    {
        const Vector3 planeNormal = CalculatePlaneNormal(triangle);

        if (!CalculateRayVsPlaneIntersection(triangle.v0, planeNormal, rayOrigin, rayDir, intersectionPointInModelSpace, distanceRayOriginToIntersection))
        {
            return false;
        }

        return TestPointInTriangle(triangle, planeNormal, intersectionPointInModelSpace);
    }

    bool IntersectionUtils::TestGeometryPicked(const Vector2& pickCoordsNDS, const float* geometry, const size_t geometrySize, const Matrix44f& modelMatrix, const Matrix44f& viewMatrix, const Matrix44f& projectionMatrix, Vector3& intersectionPointInModelSpace)
    {
        assert(geometrySize % 9 == 0);
        // 4D homogeneous Clip Coordinates
        const Vector4 ray_orig_clip(pickCoordsNDS.x, pickCoordsNDS.y, -1.0f, 1.0f);
        const Vector4 ray_target_clip(pickCoordsNDS.x, pickCoordsNDS.y, 1.0f, 1.0f);

        // 4D Camera Coordinates
        Vector4 ray_orig_camera(projectionMatrix.inverse() * ray_orig_clip);
        Vector4 ray_target_camera(projectionMatrix.inverse() * ray_target_clip);
        ray_orig_camera /=  ray_orig_camera.w;
        ray_target_camera /= ray_target_camera.w;
        ray_orig_camera.w = 1.f;
        ray_target_camera.w = 1.f;

        // 4D World Coordinates --> for ray and camera
        const Matrix44f inverseViewMatrix = viewMatrix.inverse();
        const Vector4 ray_orig_world(inverseViewMatrix * ray_orig_camera);
        const Vector4 ray_target_world(inverseViewMatrix * ray_target_camera);

        // 3D Model Coordinates
        const Matrix44f inverseModelMatrix = modelMatrix.inverse();
        Vector3 ray_orig_model(inverseModelMatrix * ray_orig_world);
        Vector3 ray_target_model(inverseModelMatrix * ray_target_world);
        const auto ray_dir_model = (ray_target_model - ray_orig_model).normalize();

        bool intersectionResult = false;
        float distanceInModelSpace = std::numeric_limits<float>::max();

        for (size_t fltIdx = 0u; fltIdx < geometrySize; fltIdx += 9)
        {
            const float* triData = &geometry[fltIdx];
            Triangle triangle;
            std::copy(triData + 0, triData + 3, triangle.v0.data);
            std::copy(triData + 3, triData + 6, triangle.v1.data);
            std::copy(triData + 6, triData + 9, triangle.v2.data);

            float distanceResult = 0.f;
            Vector3 intersectionPoint;
            if (IntersectRayVsTriangle(triangle, ray_orig_model, ray_dir_model, intersectionPoint, distanceResult))
            {
                intersectionResult = true;
                if (distanceResult < distanceInModelSpace)
                {
                    intersectionPointInModelSpace = intersectionPoint;
                    distanceInModelSpace = distanceResult;
                }
            }
        }
        return intersectionResult;
    }

    void IntersectionUtils::CheckSceneForIntersectedPickableObjects(const TransformationLinkCachedScene& scene, const Vector2i coordsInBufferSpace, PickableObjectIds& pickedObjects)
    {
        assert(pickedObjects.empty());

        struct PickedObjectEntry
        {
            PickableObjectId id;
            float distance;
        };
        std::vector<PickedObjectEntry> pickedObjectEntries;

        for (PickableObjectHandle pickableHandle(0); pickableHandle < scene.getPickableObjectCount(); ++pickableHandle)
        {
            if (scene.isPickableObjectAllocated(pickableHandle))
            {
                const PickableObject& pickableObject = scene.getPickableObject(pickableHandle);
                if (!pickableObject.isEnabled || !pickableObject.cameraHandle.isValid())
                    continue;

                const Camera& pickableCamera = scene.getCamera(pickableObject.cameraHandle);

                // get viewport data here and pass to next function
                const auto vpOffsetRef = scene.getDataReference(pickableCamera.dataInstance, Camera::ViewportOffsetField);
                const auto vpSizeRef = scene.getDataReference(pickableCamera.dataInstance, Camera::ViewportSizeField);
                const auto& vpOffset = scene.getDataSingleVector2i(vpOffsetRef, DataFieldHandle{ 0 });
                const auto& vpSize = scene.getDataSingleVector2i(vpSizeRef, DataFieldHandle{ 0 });

                const Vector2i coordsInViewportSpace = coordsInBufferSpace - vpOffset;
                //if pick event happened outside of viewport: ignore it
                if (coordsInViewportSpace.x < 0 || coordsInViewportSpace.y < 0 || coordsInViewportSpace.x > vpSize.x || coordsInViewportSpace.y > vpSize.y)
                    continue;

                const Vector2 coordsNDS = { 2.f * coordsInViewportSpace.x / vpSize.x - 1.f, 2.f * coordsInViewportSpace.y / vpSize.y - 1.f };

                const Matrix44f cameraViewMatrix = scene.updateMatrixCacheWithLinks(
                    ETransformationMatrixType_Object, pickableCamera.node);
                const Matrix44f modelMatrix = scene.updateMatrixCacheWithLinks(
                    ETransformationMatrixType_World, pickableObject.nodeHandle);

                const auto frustumPlanesRef = scene.getDataReference(pickableCamera.dataInstance, Camera::FrustumPlanesField);
                const auto frustumNearFarRef = scene.getDataReference(pickableCamera.dataInstance, Camera::FrustumNearFarPlanesField);
                const auto& frustumPlanes = scene.getDataSingleVector4f(frustumPlanesRef, DataFieldHandle{ 0 });
                const auto& frustumNearFar = scene.getDataSingleVector2f(frustumNearFarRef, DataFieldHandle{ 0 });

                const Matrix44f projectionMatrix = CameraMatrixHelper::ProjectionMatrix(
                    ProjectionParams::Frustum(pickableCamera.projectionType, frustumPlanes.x, frustumPlanes.y, frustumPlanes.z, frustumPlanes.w, frustumNearFar.x, frustumNearFar.y));

                const GeometryDataBuffer& geometryBuffer =
                    scene.getDataBuffer(pickableObject.geometryHandle);
                assert(geometryBuffer.bufferType == EDataBufferType::VertexBuffer);
                assert(geometryBuffer.dataType == EDataType::Vector3F);
                const float*  geometryBufferFloat =
                    reinterpret_cast<const float*>(geometryBuffer.data.data());
                const UInt32 geometrySize = geometryBuffer.usedSize / sizeof(float);
                assert(0 == geometrySize % 9);

                Vector3 intersectionPointInModelSpace;
                if (IntersectionUtils::TestGeometryPicked(coordsNDS,
                                                            geometryBufferFloat,
                                                            geometrySize,
                                                            modelMatrix,
                                                            cameraViewMatrix,
                                                            projectionMatrix,
                                                            intersectionPointInModelSpace))
                {
                    const Vector4 intersectionPointInClipSpace = projectionMatrix * cameraViewMatrix * modelMatrix * Vector4(intersectionPointInModelSpace);
                    const Vector4 intersectionPointInNDS = intersectionPointInClipSpace / intersectionPointInClipSpace.w;

                    assert(std::abs(intersectionPointInNDS.x - coordsNDS.x) <= std::numeric_limits<float>::epsilon() * 10);
                    assert(std::abs(intersectionPointInNDS.y - coordsNDS.y) <= std::numeric_limits<float>::epsilon() * 10);
                    const float intersectionDepthInNDS = intersectionPointInNDS.z;

                    pickedObjectEntries.push_back({ pickableObject.id , intersectionDepthInNDS });
                }
            }
        }

        pickedObjects.resize(pickedObjectEntries.size());
        std::sort(pickedObjectEntries.begin(), pickedObjectEntries.end(), [](const PickedObjectEntry& a, const PickedObjectEntry& b) { return a.distance < b.distance; });
        std::transform(pickedObjectEntries.cbegin(), pickedObjectEntries.cend(), pickedObjects.begin(), [](const PickedObjectEntry& e) { return e.id; });
    }
}

