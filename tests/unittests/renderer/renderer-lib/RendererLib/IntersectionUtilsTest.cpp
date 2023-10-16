//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/IntersectionUtils.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "internal/RendererLib/RendererScenes.h"
#include "SceneAllocateHelper.h"
#include "internal/Core/Math3d/ProjectionParams.h"
#include "internal/Core/Math3d/CameraMatrixHelper.h"
#include "glm/gtx/transform.hpp"

namespace ramses::internal
{
    static DataBufferHandle prepareGeometryBuffer(TransformationLinkCachedScene& scene, SceneAllocateHelper& sceneAllocator, const float vertexPositionsTriangle[], const uint32_t bufferSize)
    {
        DataBufferHandle geometryBuffer = sceneAllocator.allocateDataBuffer(EDataBufferType::VertexBuffer, EDataType::Vector3F, bufferSize);
        const auto* data = reinterpret_cast<const std::byte*>(vertexPositionsTriangle);
        scene.updateDataBuffer(geometryBuffer, 0, bufferSize, data);
        return geometryBuffer;
    }

    static CameraHandle preparePickableCamera(TransformationLinkCachedScene& scene, SceneAllocateHelper& sceneAllocator, const glm::ivec2 viewportOffset, const glm::ivec2 viewportSize, const glm::vec3 translation, const glm::vec3 rotation, const glm::vec3 scale)
    {
        NodeHandle cameraNodeHandle = sceneAllocator.allocateNode();
        const auto dataLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference}, DataFieldInfo{EDataType::DataReference} }, {});
        const auto dataInstance = sceneAllocator.allocateDataInstance(dataLayout);
        const auto vpDataRefLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2I} }, {});
        const auto vpOffsetInstance = sceneAllocator.allocateDataInstance(vpDataRefLayout);
        const auto vpSizeInstance = sceneAllocator.allocateDataInstance(vpDataRefLayout);
        const auto frustumPlanesLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector4F} }, {});
        const auto frustumPlanes = sceneAllocator.allocateDataInstance(frustumPlanesLayout);
        const auto frustumNearFarLayout = sceneAllocator.allocateDataLayout({ DataFieldInfo{EDataType::Vector2F} }, {});
        const auto frustumNearFar = sceneAllocator.allocateDataInstance(frustumNearFarLayout);
        scene.setDataReference(dataInstance, Camera::ViewportOffsetField, vpOffsetInstance);
        scene.setDataReference(dataInstance, Camera::ViewportSizeField, vpSizeInstance);
        scene.setDataReference(dataInstance, Camera::FrustumPlanesField, frustumPlanes);
        scene.setDataReference(dataInstance, Camera::FrustumNearFarPlanesField, frustumNearFar);
        const CameraHandle cameraHandle = sceneAllocator.allocateCamera(ECameraProjectionType::Perspective, cameraNodeHandle, dataInstance);
        scene.setDataSingleVector2i(vpOffsetInstance, DataFieldHandle{ 0 }, viewportOffset);
        scene.setDataSingleVector2i(vpSizeInstance, DataFieldHandle{ 0 }, viewportSize);
        const ProjectionParams params = ProjectionParams::Perspective(19.f, static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y), 0.1f, 100.f);
        scene.setDataSingleVector4f(frustumPlanes, DataFieldHandle{ 0 }, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane });
        scene.setDataSingleVector2f(frustumNearFar, DataFieldHandle{ 0 }, { params.nearPlane, params.farPlane });

        TransformHandle cameraTransformation = sceneAllocator.allocateTransform(cameraNodeHandle);
        scene.setTranslation(cameraTransformation, translation);
        scene.setRotation(cameraTransformation, glm::vec4(rotation, 1.f), ERotationType::Euler_XYZ);
        scene.setScaling(cameraTransformation, scale);
        return cameraHandle;
    }

    static void preparePickableObject(TransformationLinkCachedScene& scene, SceneAllocateHelper& sceneAllocator, DataBufferHandle geometryBuffer, CameraHandle cameraHandle, PickableObjectId pickableId, const glm::vec3 translation, const glm::vec3 rotation, const glm::vec3 scale)
    {
        const NodeHandle pickableNodeHandle = sceneAllocator.allocateNode();
        const PickableObjectHandle pickableHandle = sceneAllocator.allocatePickableObject(geometryBuffer, pickableNodeHandle, pickableId);
        scene.setPickableObjectCamera(pickableHandle, cameraHandle);
        TransformHandle pickableTransformation = sceneAllocator.allocateTransform(pickableNodeHandle);
        scene.setTranslation(pickableTransformation, translation);
        scene.setRotation(pickableTransformation, glm::vec4(rotation, 1.f), ERotationType::Euler_XYZ);
        scene.setScaling(pickableTransformation, scale);
    }

    static void checkSceneForIntersectedPickableObjects(const TransformationLinkCachedScene& scene, const glm::vec2 coords, const glm::ivec2 dispResolution, const PickableObjectIds& expectedPickables)
    {
        const auto xCoordDisplaySpace = static_cast<int32_t>(std::lroundf((coords.x + 1.f) * static_cast<float>(dispResolution.x) / 2.f));
        const auto yCoordDisplaySpace = static_cast<int32_t>(std::lroundf((coords.y + 1.f) * static_cast<float>(dispResolution.y) / 2.f));

        PickableObjectIds resultPickables;
        IntersectionUtils::CheckSceneForIntersectedPickableObjects(scene, {xCoordDisplaySpace, yCoordDisplaySpace}, resultPickables);

        EXPECT_EQ(resultPickables, expectedPickables);
    }

    TEST(IntersectionUtilsTest, canIntersectTriangle1)
    {
        const glm::vec3 rayOrig(0, 0, 0);
        const glm::vec3 v0(0, 0, 5);
        const glm::vec3 v1(1, 0, 5);
        const glm::vec3 v2(0, 1, 5);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayHit(0, 0, 1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayHit, intersectionPoint, distance);
        EXPECT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(5.f, distance);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.x);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.y);
        EXPECT_FLOAT_EQ(5.f, intersectionPoint.z);
    }

    TEST(IntersectionUtilsTest, canIntersectTriangle2)
    {
        const glm::vec3 rayOrig(1, 1, 1);
        const glm::vec3 v0(1, 1, 5);
        const glm::vec3 v1(2, 1, 5);
        const glm::vec3 v2(1, 2, 5);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayHit(0, 0, 1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayHit, intersectionPoint, distance);
        EXPECT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(4.f, distance);
        EXPECT_FLOAT_EQ(1.f, intersectionPoint.x);
        EXPECT_FLOAT_EQ(1.f, intersectionPoint.y);
        EXPECT_FLOAT_EQ(5.f, intersectionPoint.z);
    }

    TEST(IntersectionUtilsTest, noIntersectRayNotPointingToTriangle)
    {
        const glm::vec3 rayOrig(0, 0, 0);
        const glm::vec3 v0(0, 0, 5);
        const glm::vec3 v1(1, 0, 5);
        const glm::vec3 v2(0, 1, 5);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayMiss(0, 20, 1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, glm::normalize(rayMiss), intersectionPoint, distance);
        EXPECT_FALSE(intersectionHappened);
    }

    TEST(IntersectionUtilsTest, noIntersectRayPrallelToTriangle)
    {
        const glm::vec3 rayOrig(0, 0, 0);
        const glm::vec3 v0(0, 0, 5);
        const glm::vec3 v1(1, 0, 5);
        const glm::vec3 v2(0, 1, 5);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayParallel(0, 1, 0);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayParallel, intersectionPoint, distance);
        EXPECT_FALSE(intersectionHappened);
    }

    TEST(IntersectionUtilsTest, noIntersectTriangleBehindRay1)
    {
        const glm::vec3 rayOrig(0, 0, 1);
        const glm::vec3 v0(0, 0, 0);
        const glm::vec3 v1(1, 0, 0);
        const glm::vec3 v2(0, 1, 0);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayDir(0, 0, 1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayDir, intersectionPoint, distance);
        EXPECT_FALSE(intersectionHappened);
    }

    TEST(IntersectionUtilsTest, noIntersectTriangleBehindRay2)
    {
        const glm::vec3 rayOrig(0, 0, 1);
        const glm::vec3 v0(0, 0, 0);
        const glm::vec3 v1(1, 0, 0);
        const glm::vec3 v2(0, 1, 0);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayDir(0, 0, -1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayDir, intersectionPoint, distance);
        EXPECT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(1.f, distance);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.x);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.y);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.z);
    }

    TEST(IntersectionUtilsTest, noIntersectTriangleBehindRay3)
    {
        const glm::vec3 rayOrig(0, 0, -1);
        const glm::vec3 v0(0, 0, 0);
        const glm::vec3 v1(1, 0, 0);
        const glm::vec3 v2(0, 1, 0);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayDir(0, 0, 1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayDir, intersectionPoint, distance);
        EXPECT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(1.f, distance);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.x);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.y);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.z);
    }

    TEST(IntersectionUtilsTest, noIntersectTriangleBehindRay4)
    {
        const glm::vec3 rayOrig(0, 0, -1);
        const glm::vec3 v0(0, 0, 0);
        const glm::vec3 v1(1, 0, 0);
        const glm::vec3 v2(0, 1, 0);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayDir(0, 0, -1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayDir, intersectionPoint, distance);
        EXPECT_FALSE(intersectionHappened);
    }

    TEST(IntersectionUtilsTest, noIntersectTriangleBehindRay5)
    {
        const glm::vec3 rayOrig(1, 1, 3);
        const glm::vec3 v0(1, 1, 1);
        const glm::vec3 v1(2, 1, 1);
        const glm::vec3 v2(1, 2, 1);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        const glm::vec3 rayDir(0, 0, 1);
        glm::vec3 intersectionPoint;
        float distance = NAN;
        bool          intersectionHappened = IntersectionUtils::IntersectRayVsTriangle(triangle, rayOrig, rayDir, intersectionPoint, distance);
        EXPECT_FALSE(intersectionHappened);
    }

    TEST(IntersectionUtilsTest, canCalculateNormalOfPlane)
    {
        const glm::vec3 v0(0, 0, 5);
        const glm::vec3 v1(1, 0, 5);
        const glm::vec3 v2(0, 1, 5);
        const IntersectionUtils::Triangle triangle{ v0, v1, v2 };
        glm::vec3       normal = IntersectionUtils::CalculatePlaneNormal(triangle);
        glm::vec3       expectedNormal(0.0f, 0.0f, 1.0f);

        for (uint32_t i = 0; i < 3; ++i)
        {
            EXPECT_FLOAT_EQ(normal[i], expectedNormal[i]);
        }
    }

    TEST(IntersectionUtilsTest, canPickSingleTriangle)
    {
        const std::vector<float> triangleData{ -1.0f, 0.0f, 0.0f,
                                                1.0f, 0.0f, 0.0f,
                                                0.0f, 1.0f, 0.0f };

        const auto modelMatrix                = glm::identity<glm::mat4>();
        const auto cameraTransformationMatrix = glm::translate(glm::vec3{ 0.0f, 0.0f, 1.0f });
        const auto viewMatrix                 = glm::inverse(cameraTransformationMatrix);
        const auto projectionMatrix           = glm::identity<glm::mat4>();

        const glm::vec2 ndsPickCoords(0.0f, 0.0f);

        glm::vec3 intersectionPoint;
        bool intersectionHappened =
            IntersectionUtils::TestGeometryPicked(ndsPickCoords, triangleData.data(), triangleData.size(), modelMatrix, viewMatrix, projectionMatrix, intersectionPoint);
        EXPECT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(intersectionPoint.x, 0.f);
        EXPECT_FLOAT_EQ(intersectionPoint.y, 0.f);
        EXPECT_FLOAT_EQ(intersectionPoint.z, 0.f);

        const auto scalingModelMatrix = glm::scale(glm::vec3{ 1.0f, 2.0f, 3.0f });

        intersectionHappened =
            IntersectionUtils::TestGeometryPicked(ndsPickCoords, triangleData.data(), triangleData.size(), scalingModelMatrix, viewMatrix, projectionMatrix, intersectionPoint);
        EXPECT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(intersectionPoint.x, 0.f);
        EXPECT_FLOAT_EQ(intersectionPoint.y, 0.f);
        EXPECT_FLOAT_EQ(intersectionPoint.z, 0.f);
    }

    TEST(IntersectionUtilsTest, noPickSingleTriangleTranslatedAwayFromPick)
    {
        const std::vector<float> triangle1Data{ -1.0f, 0.0f, 0.0f,
                                                1.0f, 0.0f, 0.0f,
                                                0.0f, 1.0f, 0.0f };

        const auto modelMatrix = glm::translate(glm::vec3{ 0.0f, 1.0f, 0.0f });
        const auto cameraTransformationMatrix = glm::translate(glm::vec3{ 0.0f, 0.0f, 1.0f });
        const auto viewMatrix       = glm::inverse(cameraTransformationMatrix);
        const auto projectionMatrix = glm::identity<glm::mat4>();
        const glm::vec2 ndsPickCoords(0.0f, 0.0f);

        glm::vec3 intersectionPoint;
        bool intersectionHappened =
            IntersectionUtils::TestGeometryPicked(ndsPickCoords, triangle1Data.data(), triangle1Data.size(), modelMatrix, viewMatrix, projectionMatrix, intersectionPoint);
        EXPECT_FALSE(intersectionHappened);

        const std::vector<float> triangle2Data{-4.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 7.0f, 0.0f};

        intersectionHappened =
            IntersectionUtils::TestGeometryPicked(ndsPickCoords, triangle2Data.data(), triangle2Data.size(), modelMatrix, viewMatrix, projectionMatrix, intersectionPoint);
        EXPECT_FALSE(intersectionHappened);
    }

    TEST(IntersectionUtilsTest, canIntersectMultipleTriangleGeometry)
    {
        const std::vector<float> triangle1Data{   0.0f, -1.0f, 0.0f,
                                                -1.0f, 0.0f, 0.0f,
                                                0.0f, 1.0f, 0.0f };
        const std::vector<float> triangle2Data{   0.0f, -1.0f, 0.0f,
                                                0.0f, 1.0f, 0.0f,
                                                1.0f, 0.0f, 0.0f };
        //quad including origin
        std::vector<float> quad;
        quad.insert(quad.end(), triangle1Data.begin(), triangle1Data.end());
        quad.insert(quad.end(), triangle2Data.begin(), triangle2Data.end());

        const auto modelMatrix                = glm::identity<glm::mat4>();
        const auto cameraTransformationMatrix = glm::translate(glm::vec3{0.0f, 0.0f, 1.0f});
        const auto viewMatrix = glm::inverse(cameraTransformationMatrix);
        const auto projectionMatrix           = glm::identity<glm::mat4>();
        const glm::vec2 ndsPickCoords(0.0f, 0.0f);

        glm::vec3 intersectionPoint;
        bool intersectionHappened =
            IntersectionUtils::TestGeometryPicked(ndsPickCoords, quad.data(), quad.size(), modelMatrix, viewMatrix, projectionMatrix, intersectionPoint);
        EXPECT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(intersectionPoint.x, 0.f);
        EXPECT_FLOAT_EQ(intersectionPoint.y, 0.f);
        EXPECT_FLOAT_EQ(intersectionPoint.z, 0.f);
    }

    TEST(IntersectionUtilsTest, noIntersectPickNotPointingToMultipleTriangles)
    {
        const std::vector<float> triangle1Data{ -2.0f, 0.0f, 0.0f,
                                                -2.0f, 1.0f, 0.0f,
                                                -1.0f, 1.0f, 0.0f };
        const std::vector<float> triangle2Data{ -1.0f, 0.0f, 0.0f,
                                                -2.0f, 0.0f, 0.0f,
                                                -1.0f, 1.0f, 0.0f };
        // quad not including origin
        std::vector<float> quad;
        quad.insert(quad.end(), triangle1Data.begin(), triangle1Data.end());
        quad.insert(quad.end(), triangle2Data.begin(), triangle2Data.end());

        const auto modelMatrix = glm::identity<glm::mat4>();
        const auto cameraTransformationMatrix = glm::translate(glm::vec3{0.0f, 0.0f, 1.0f});
        const auto viewMatrix = glm::inverse(cameraTransformationMatrix);
        const auto projectionMatrix = glm::identity<glm::mat4>();
        const glm::vec2 ndsPickCoords(0.0f, 0.0f);

        glm::vec3 intersectionPoint;
        bool intersectionHappened =
            IntersectionUtils::TestGeometryPicked(ndsPickCoords, quad.data(), quad.size(), modelMatrix, viewMatrix, projectionMatrix, intersectionPoint);
        EXPECT_FALSE(intersectionHappened);
    }

    TEST(IntersectionUtilsTest, findsPickedObjectInSceneWhenIntersected)
    {
        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes(rendererEventCollector);
        TransformationLinkCachedScene scene(rendererScenes.getSceneLinksManager(), {});
        SceneAllocateHelper sceneAllocator(scene);
        float vertexPositionsTriangle[] = { -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f };
        const glm::ivec2 dispResolution = { 1280, 480 };

        //Prepare PickableCamera
        const CameraHandle cameraHandle = preparePickableCamera(scene, sceneAllocator, { 0, 0 }, dispResolution, { -4.f, 0.f, 11.f }, { 0.f, -40.f, 0.f }, { 1.f, 1.f, 1.f });

        //Prepare GeometryBuffer
        DataBufferHandle geometryBuffer = prepareGeometryBuffer(scene, sceneAllocator, vertexPositionsTriangle, sizeof(vertexPositionsTriangle));

        //Prepare PickableObject
        PickableObjectId pickableId(341u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, cameraHandle, pickableId, { 0.1f, 1.0f, -1.0f }, { -70.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 10.0f });

        //Testing scene for intersection
        const glm::vec2 coordsInViewportSpaceHit1 = {0.310937f, 0.354166f};
        const glm::vec2 coordsInViewportSpaceMiss1_1 = {0.309374f, 0.395833f};
        const glm::vec2 coordsInViewportSpaceMiss1_2 = {0.312500f, 0.312500f};

        const glm::vec2 coordsInViewportSpaceHit2 = { -0.603125f, 0.945833f };
        const glm::vec2 coordsInViewportSpaceMiss2 = { -0.603125f, 0.987500f };

        const glm::vec2 coordsInViewportSpaceHit3 = { -0.970313f, 0.533333f };
        const glm::vec2 coordsInViewportSpaceMiss3 = { -0.968750f, 0.495833f };

        //Test first area of PickableObject
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHit1, dispResolution, { pickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss1_1, dispResolution, {});
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss1_2, dispResolution, {});

        //Test second area of PickableObject
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHit2, dispResolution, { pickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss2, dispResolution, {});

        //Test third area of PickableObject
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHit3, dispResolution, { pickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss3, dispResolution, {});
    }

    TEST(IntersectionUtilsTest, findsMultiplePickedObjectsNotOverlappingInSceneWhenIntersected)
    {
        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes(rendererEventCollector);
        TransformationLinkCachedScene scene(rendererScenes.getSceneLinksManager(), {});
        SceneAllocateHelper sceneAllocator(scene);
        float vertexPositionsTriangle[] = { -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f };
        const glm::ivec2 dispResolution = { 1280, 480 };

        //Prepare PickableCamera
        const CameraHandle cameraHandle = preparePickableCamera(scene, sceneAllocator, { 0, 0 }, dispResolution, { -4.f, 0.f, 11.f }, { 0.f, -40.f, 0.f }, { 1.f, 1.f, 1.f });

        //Prepare GeometryBuffer
        DataBufferHandle geometryBuffer = prepareGeometryBuffer(scene, sceneAllocator, vertexPositionsTriangle, sizeof(vertexPositionsTriangle));

        //Prepare PickableObject1
        PickableObjectId pickableId(341u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, cameraHandle, pickableId, { 0.1f, 1.0f, -1.0f }, { -70.0f, 0.0f, 0.0f }, { 3.0f, 3.0f, 3.0f });

        //Prepare PickableObject2
        PickableObjectId pickableId2(235u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, cameraHandle, pickableId2, { 6.0f, 0.9f, -1.0f }, { -70.0f, 0.0f, 0.0f }, { 3.0f, 3.0f, 3.0f });

        //Testing scene for intersection
        const glm::vec2 coordsInViewportSpaceHitPickable1 = { -0.382812f, 0.441667f };
        const glm::vec2 coordsInViewportSpaceHitPickable2 = { -0.379687f, 0.395833f };
        const glm::vec2 coordsInViewportSpaceMiss1 = { -0.382812f, 0.466667f };
        const glm::vec2 coordsInViewportSpaceMiss2 = { -0.378125f, 0.425000f };
        const glm::vec2 coordsInViewportSpaceMiss3 = { -0.379687f, 0.383333f };

        //Test PickableObjects
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHitPickable1, dispResolution, { pickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHitPickable2, dispResolution, { pickableId2 });

        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss1, dispResolution, {});
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss2, dispResolution, {});
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss3, dispResolution, {});
    }

    TEST(IntersectionUtilsTest, findsMultiplePickedObjectsOverlappingInSceneWhenIntersected)
    {
        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes(rendererEventCollector);
        TransformationLinkCachedScene scene(rendererScenes.getSceneLinksManager(), {});
        SceneAllocateHelper sceneAllocator(scene);
        float vertexPositionsTriangle[] = { -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f };
        const glm::ivec2 dispResolution = { 1280, 480 };

        //Prepare PickableCamera
        const CameraHandle cameraHandle = preparePickableCamera(scene, sceneAllocator, { 0, 0 }, dispResolution, { -4.f, 0.f, 11.f }, { 0.f, -40.f, 0.f }, { 1.f, 1.f, 1.f });

        //Prepare GeometryBuffer
        DataBufferHandle geometryBuffer = prepareGeometryBuffer(scene, sceneAllocator, vertexPositionsTriangle, sizeof(vertexPositionsTriangle));

        //Prepare PickableObject1
        PickableObjectId pickableId(341u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, cameraHandle, pickableId, { 0.1f, 1.0f, -1.0f }, { -70.0f, 0.0f, 0.0f }, { 3.0f, 3.0f, 3.0f });

        //Prepare PickableObject2
        PickableObjectId pickableId2(235u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, cameraHandle, pickableId2, { 4.0f, 0.9f, -1.0f }, { -70.0f, 0.0f, 0.0f }, { 3.0f, 3.0f, 3.0f });

        //Testing scene for intersection
        const glm::vec2 coordsInViewportSpaceHitPickable1 = { -0.585938f, 0.575000f };
        const glm::vec2 coordsInViewportSpaceHitPickable2 = { -0.590625f, 0.458333f };
        const glm::vec2 coordsInViewportSpaceHitPickable1and2 = { -0.582812f, 0.512500f };
        const glm::vec2 coordsInViewportSpaceMiss1 = { -0.585938f, 0.595833f };
        const glm::vec2 coordsInViewportSpaceMiss2 = { -0.607812f, 0.408333f };

        //Test PickableObjects
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHitPickable1, dispResolution, { pickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHitPickable2, dispResolution, { pickableId2 });
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceHitPickable1and2, dispResolution, { pickableId, pickableId2 });

        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss1, dispResolution, {});
        checkSceneForIntersectedPickableObjects(scene, coordsInViewportSpaceMiss2, dispResolution, {});
    }

    TEST(IntersectionUtilsTest, picksIntersectionPointAtNearestTriangleInPickable)
    {
        const std::vector<float> trianglesVertices{
                                                -1.0f, 0.0f, -10.0f,
                                                1.0f, 0.0f, -10.0f,
                                                0.0f, 1.0f, -10.0f,

                                                -1.0f, 0.0f, -12.0f,
                                                1.0f, 0.0f, -12.0f,
                                                0.0f, 1.0f, -12.0f,

                                                0.0f, -1.0f, -9.0f,
                                                1.0f,  0.0f, -9.0f,
                                                0.0f, 1.0f, -9.0f,

                                                0.0f, -1.0f, -3.0f,
                                                1.0f,  0.0f, -3.0f,
                                                0.0f, 1.0f, -3.0f,

                                                0.0f, -1.0f, -15.0f,
                                                1.0f,   0.0f, -15.0f,
                                                0.0f,  1.0f, -15.0f
                                                };

        const auto modelMatrix = glm::identity<glm::mat4>();
        const auto cameraTransformationMatrix = glm::translate(glm::vec3{ 0.0f, 0.0f, 1.0f });
        const auto viewMatrix = glm::inverse(cameraTransformationMatrix);
        const auto projectionMatrix(CameraMatrixHelper::ProjectionMatrix(ProjectionParams::Perspective(30.f, 1.f, 0.1f, 100.f)));
        const glm::vec2 ndsPickCoords(0.0f, 0.0f);

        glm::vec3 intersectionPoint;
        const bool intersectionHappened =
            IntersectionUtils::TestGeometryPicked(ndsPickCoords, trianglesVertices.data(), trianglesVertices.size(), modelMatrix, viewMatrix, projectionMatrix, intersectionPoint);
        ASSERT_TRUE(intersectionHappened);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.x);
        EXPECT_FLOAT_EQ(0.f, intersectionPoint.y);
        EXPECT_FLOAT_EQ(-3.f, intersectionPoint.z);
    }

    TEST(IntersectionUtilsTest, canSortPickedPickablesWithDistanceWhenPickablesUseDifferentCameras)
    {
        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes(rendererEventCollector);
        TransformationLinkCachedScene scene(rendererScenes.getSceneLinksManager(), {});
        SceneAllocateHelper sceneAllocator(scene);
        float vertexPositionsTriangle[] = { -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f };
        const glm::ivec2 dispResolution = { 1280, 480 };

        //Prepare PickableCamera
        const CameraHandle cameraFrontPickableHandle = preparePickableCamera(scene, sceneAllocator, { 0, 0 }, dispResolution, { 0.f, 0.f, 11.f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f });
        const CameraHandle cameraBackPickableHandle = preparePickableCamera(scene, sceneAllocator, { 0, 0 }, dispResolution, { 15.f, 0.f, 25.f }, { 0.f, 0.f, 0.f }, { 1.f, 5.f, .01f });

        //Prepare GeometryBuffer
        const DataBufferHandle geometryBuffer = prepareGeometryBuffer(scene, sceneAllocator, vertexPositionsTriangle, sizeof(vertexPositionsTriangle));

        //Prepare pibckable that appears in the front
        const PickableObjectId frontPickableId(341u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, cameraFrontPickableHandle, frontPickableId, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 1.0f, 1.0f, 1.0f });

        //Prepare pibckable that appears in the back (even though the distance between pickable and camera is 1.0, the camera is scaled big enough to make this pickable partially occluded by the former pickable)
        const PickableObjectId backPickableId(235u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, cameraBackPickableHandle, backPickableId, { 12.5f, 0.f, 24.0f }, { 0.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 1.0f });

        //Testing scene for intersection
        const glm::vec2 coordsInNDSHitFrontPickable = { -0.0625f, 0.16666666666f };
        const glm::vec2 coordsInNDSHitBackPickable = { -0.21875f, 0.02083333333f };
        const glm::vec2 coordsInNDSHitBothPickables = { -0.1875f, 0.02083333333f };

        //Test PickableObjects
        checkSceneForIntersectedPickableObjects(scene, coordsInNDSHitFrontPickable, dispResolution, { frontPickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInNDSHitBackPickable, dispResolution, { backPickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInNDSHitBothPickables, dispResolution, {frontPickableId, backPickableId});
    }

    TEST(IntersectionUtilsTest, handlesPickableViewportCorrectly)
    {
        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes(rendererEventCollector);
        TransformationLinkCachedScene scene(rendererScenes.getSceneLinksManager(), {});
        SceneAllocateHelper sceneAllocator(scene);
        float vertexPositionsTriangle[] = { -1.f, -1.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 0.f };
        const glm::ivec2 dispResolution = { 1280, 480 };

        //Prepare two cameras with overlapping viewports
        const glm::ivec2 viewportSize             = { 1280 * 2 / 3, 480 * 2 / 3 };
        const glm::ivec2 viewportOffset1          = { 0, 0 };
        const glm::ivec2 viewportOffset2          = { 1280 / 3, 480 / 3 };
        const CameraHandle bottomLeftCamera     = preparePickableCamera(scene, sceneAllocator, viewportOffset1, viewportSize, { 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f });
        const CameraHandle topRightCamera       = preparePickableCamera(scene, sceneAllocator, viewportOffset2, viewportSize, { 0.f, 0.f, 1.01f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f });

        //Prepare GeometryBuffer
        const DataBufferHandle geometryBuffer = prepareGeometryBuffer(scene, sceneAllocator, vertexPositionsTriangle, sizeof(vertexPositionsTriangle));

        const PickableObjectId bottomLeftPickableId(341u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, bottomLeftCamera, bottomLeftPickableId, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 1.0f, 1.0f, 1.0f });

        const PickableObjectId topRightPickableId(235u);
        preparePickableObject(scene, sceneAllocator, geometryBuffer, topRightCamera, topRightPickableId, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 1.0f, 1.0f, 1.0f });

        //Testing scene for intersection
        const glm::vec2 coordsInNDSHitBottomLeftPickable      = { -.5f, -.5f };
        const glm::vec2 coordsInNDSHitTopRightPickable        = { .5f,  .5f };
        const glm::vec2 coordsInNDSHitBothPickables           = { 0.f, 0.f };
        const glm::vec2 coordsInNDSMissPickablesInBottomRight = { .5f, -.5f };
        const glm::vec2 coordsInNDSMissPickablesInTopLeft     = { -.5f, .5f };

        //Test PickableObjects
        checkSceneForIntersectedPickableObjects(scene, coordsInNDSHitBottomLeftPickable, dispResolution, { bottomLeftPickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInNDSHitTopRightPickable, dispResolution, { topRightPickableId });
        checkSceneForIntersectedPickableObjects(scene, coordsInNDSHitBothPickables, dispResolution, { bottomLeftPickableId, topRightPickableId });

        checkSceneForIntersectedPickableObjects(scene, coordsInNDSMissPickablesInTopLeft, dispResolution, {});
        checkSceneForIntersectedPickableObjects(scene, coordsInNDSMissPickablesInBottomRight, dispResolution, {});
    }
}
