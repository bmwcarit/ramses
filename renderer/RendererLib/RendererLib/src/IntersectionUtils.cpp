//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/IntersectionUtils.h"
#include "Math3d/CameraMatrixHelper.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/Types.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses_internal
{
    glm::vec3 IntersectionUtils::CalculatePlaneNormal(const Triangle& triangle)
    {
        const glm::vec3 edge0 = triangle.v1 - triangle.v0;
        const glm::vec3 edge1 = triangle.v2 - triangle.v0;
        return glm::cross(edge0, edge1);
    }

    bool IntersectionUtils::TestPointInTriangle(const Triangle& triangle, const glm::vec3& planeNormal, const glm::vec3& testPoint)
    {
        glm::vec3 VecPerpToPlane;
        const glm::vec3 edge0     = triangle.v1 - triangle.v0;
        const glm::vec3 v0Inters0 = testPoint - triangle.v0;
        VecPerpToPlane          = glm::cross(edge0, v0Inters0);
        if (glm::dot(planeNormal, VecPerpToPlane) < 0)
        {
            return false;
        }

        const glm::vec3 edge1     = triangle.v2 - triangle.v1;
        const glm::vec3 v1inters1 = testPoint - triangle.v1;
        VecPerpToPlane          = glm::cross(edge1, v1inters1);
        if (glm::dot(planeNormal, VecPerpToPlane) < 0)
        {
            return false;
        }

        const glm::vec3 edge2     = triangle.v0 - triangle.v2;
        const glm::vec3 v2inters2 = testPoint - triangle.v2;
        VecPerpToPlane          = glm::cross(edge2, v2inters2);
        if (glm::dot(planeNormal, VecPerpToPlane) < 0)
        {
            return false;
        }

        return true;
    }

    bool IntersectionUtils::CalculateRayVsPlaneIntersection(const glm::vec3& triangleVertex,
                                                const glm::vec3& triangleNormal,
                                                const glm::vec3& rayOrigin,
                                                const glm::vec3& rayDir,
                                                glm::vec3&       intersection,
                                                float& distanceRayOriginToIntersection)
    {
        // make sure rayDir is normalized
        assert(std::abs(glm::length(rayDir) - 1) < std::numeric_limits<float>::epsilon() * 10);

        const float nDotRay = glm::dot(triangleNormal, rayDir);
        if (std::abs(nDotRay) <= std::numeric_limits<float>::epsilon() * 10) // some epsilon that could still produce valid numbers later on
        {
            return false; // ray and triangle parallel
        }

        distanceRayOriginToIntersection = glm::dot(triangleVertex - rayOrigin, triangleNormal) / nDotRay;
        if (distanceRayOriginToIntersection < 0)
        {
            return false; // triangle behind ray
        }

        intersection = rayOrigin + distanceRayOriginToIntersection * rayDir;

        return true;
    }

    bool IntersectionUtils::IntersectRayVsTriangle(const Triangle& triangle, const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec3& intersectionPointInModelSpace, float& distanceRayOriginToIntersection)
    {
        const glm::vec3 planeNormal = CalculatePlaneNormal(triangle);

        if (!CalculateRayVsPlaneIntersection(triangle.v0, planeNormal, rayOrigin, rayDir, intersectionPointInModelSpace, distanceRayOriginToIntersection))
        {
            return false;
        }

        return TestPointInTriangle(triangle, planeNormal, intersectionPointInModelSpace);
    }

    bool IntersectionUtils::TestGeometryPicked(const glm::vec2& pickCoordsNDS, const float* geometry, const size_t geometrySize, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, glm::vec3& intersectionPointInModelSpace)
    {
        assert(geometrySize % 9 == 0);
        // 4D homogeneous Clip Coordinates
        const glm::vec4 ray_orig_clip(pickCoordsNDS.x, pickCoordsNDS.y, -1.0f, 1.0f);
        const glm::vec4 ray_target_clip(pickCoordsNDS.x, pickCoordsNDS.y, 1.0f, 1.0f);

        // 4D Camera Coordinates
        glm::vec4 ray_orig_camera(glm::inverse(projectionMatrix) * ray_orig_clip);
        glm::vec4 ray_target_camera(glm::inverse(projectionMatrix) * ray_target_clip);
        ray_orig_camera /=  ray_orig_camera.w;
        ray_target_camera /= ray_target_camera.w;
        ray_orig_camera.w = 1.f;
        ray_target_camera.w = 1.f;

        // 4D World Coordinates --> for ray and camera
        const auto inverseViewMatrix = glm::inverse(viewMatrix);
        const glm::vec4 ray_orig_world(inverseViewMatrix * ray_orig_camera);
        const glm::vec4 ray_target_world(inverseViewMatrix * ray_target_camera);

        // 3D Model Coordinates
        const auto inverseModelMatrix = glm::inverse(modelMatrix);
        glm::vec3 ray_orig_model(inverseModelMatrix * ray_orig_world);
        glm::vec3 ray_target_model(inverseModelMatrix * ray_target_world);
        const auto ray_dir_model = glm::normalize(ray_target_model - ray_orig_model);

        bool intersectionResult = false;
        float distanceInModelSpace = std::numeric_limits<float>::max();

        for (size_t fltIdx = 0u; fltIdx < geometrySize; fltIdx += 9)
        {
            const float* triData = &geometry[fltIdx];
            Triangle triangle;
            std::copy(triData + 0, triData + 3, glm::value_ptr(triangle.v0));
            std::copy(triData + 3, triData + 6, glm::value_ptr(triangle.v1));
            std::copy(triData + 6, triData + 9, glm::value_ptr(triangle.v2));

            float distanceResult = 0.f;
            glm::vec3 intersectionPoint;
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

    void IntersectionUtils::CheckSceneForIntersectedPickableObjects(const TransformationLinkCachedScene& scene, const glm::ivec2 coordsInBufferSpace, PickableObjectIds& pickedObjects)
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

                const glm::ivec2 coordsInViewportSpace = coordsInBufferSpace - vpOffset;
                //if pick event happened outside of viewport: ignore it
                if (coordsInViewportSpace.x < 0 || coordsInViewportSpace.y < 0 || coordsInViewportSpace.x > vpSize.x || coordsInViewportSpace.y > vpSize.y)
                    continue;

                const glm::vec2 coordsNDS = { 2.f * coordsInViewportSpace.x / vpSize.x - 1.f, 2.f * coordsInViewportSpace.y / vpSize.y - 1.f };

                const auto cameraViewMatrix = scene.updateMatrixCacheWithLinks(
                    ETransformationMatrixType_Object, pickableCamera.node);
                const auto modelMatrix = scene.updateMatrixCacheWithLinks(
                    ETransformationMatrixType_World, pickableObject.nodeHandle);

                const auto frustumPlanesRef = scene.getDataReference(pickableCamera.dataInstance, Camera::FrustumPlanesField);
                const auto frustumNearFarRef = scene.getDataReference(pickableCamera.dataInstance, Camera::FrustumNearFarPlanesField);
                const auto& frustumPlanes = scene.getDataSingleVector4f(frustumPlanesRef, DataFieldHandle{ 0 });
                const auto& frustumNearFar = scene.getDataSingleVector2f(frustumNearFarRef, DataFieldHandle{ 0 });

                const auto projectionMatrix = CameraMatrixHelper::ProjectionMatrix(
                    ProjectionParams::Frustum(pickableCamera.projectionType, frustumPlanes.x, frustumPlanes.y, frustumPlanes.z, frustumPlanes.w, frustumNearFar.x, frustumNearFar.y));

                const GeometryDataBuffer& geometryBuffer =
                    scene.getDataBuffer(pickableObject.geometryHandle);
                assert(geometryBuffer.bufferType == EDataBufferType::VertexBuffer);
                assert(geometryBuffer.dataType == EDataType::Vector3F);
                const float*  geometryBufferFloat =
                    reinterpret_cast<const float*>(geometryBuffer.data.data());
                const UInt32 geometrySize = geometryBuffer.usedSize / sizeof(float);
                assert(0 == geometrySize % 9);

                glm::vec3 intersectionPointInModelSpace;
                if (IntersectionUtils::TestGeometryPicked(coordsNDS,
                                                            geometryBufferFloat,
                                                            geometrySize,
                                                            modelMatrix,
                                                            cameraViewMatrix,
                                                            projectionMatrix,
                                                            intersectionPointInModelSpace))
                {
                    const glm::vec4 intersectionPointInClipSpace = projectionMatrix * cameraViewMatrix * modelMatrix * glm::vec4(intersectionPointInModelSpace, 1.f);
                    const glm::vec4 intersectionPointInNDS = intersectionPointInClipSpace / intersectionPointInClipSpace.w;

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

