//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// API
#include "ramses/framework/RamsesFrameworkTypes.h"

// internal
#include "impl/NodeImpl.h"
#include "internal/SceneGraph/SceneAPI/ECameraProjectionType.h"
#include "internal/Core/Math3d/ProjectionParams.h"

#include <string_view>

namespace ramses
{
    class DataObject;
}

namespace ramses::internal
{
    class CameraNodeImpl final : public NodeImpl
    {
    public:
        CameraNodeImpl(SceneImpl& scene, ERamsesObjectType cameraType, std::string_view cameraName);
        ~CameraNodeImpl() override;

        // Common for all camera types
        [[nodiscard]] ECameraProjectionType getProjectionType() const;

        bool setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
        [[nodiscard]] int32_t getViewportX() const;
        [[nodiscard]] int32_t getViewportY() const;
        [[nodiscard]] uint32_t getViewportWidth() const;
        [[nodiscard]] uint32_t getViewportHeight() const;

        void initializeFrameworkData();
        void deinitializeFrameworkData() override;
        bool serialize(IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(IInputStream& inStream, DeserializationContext& serializationContext) override;

        void onValidate(ValidationReportImpl& report) const override;
        [[nodiscard]] CameraHandle getCameraHandle() const;

        bool setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane);
        [[nodiscard]] float    getLeftPlane() const;
        [[nodiscard]] float    getRightPlane() const;
        [[nodiscard]] float    getBottomPlane() const;
        [[nodiscard]] float    getTopPlane() const;
        [[nodiscard]] float    getNearPlane() const;
        [[nodiscard]] float    getFarPlane() const;

        bool setPerspectiveFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane);
        [[nodiscard]] float    getVerticalFieldOfView() const;
        [[nodiscard]] float    getAspectRatio() const;

        bool getProjectionMatrix(matrix44f& projectionMatrix) const;

        bool bindViewportOffset(const DataObject& offsetData);
        bool bindViewportSize(const DataObject& sizeData);
        bool bindFrustumPlanes(const DataObject& frustumPlanesData, const DataObject& nearFarData);
        bool unbindViewportOffset();
        bool unbindViewportSize();
        bool unbindFrustumPlanes();
        [[nodiscard]] bool isViewportOffsetBound() const;
        [[nodiscard]] bool isViewportSizeBound() const;
        [[nodiscard]] bool isFrustumPlanesBound() const;

        [[nodiscard]] DataInstanceHandle getViewportOffsetHandle() const;
        [[nodiscard]] DataInstanceHandle getViewportSizeHandle() const;
        [[nodiscard]] DataInstanceHandle getFrustumPlanesHandle() const;
        [[nodiscard]] DataInstanceHandle getFrustumNearFarPlanesHandle() const;

    private:
        [[nodiscard]] ProjectionParams getProjectionParams() const;
        void                           updateProjectionParamsOnScene(const ProjectionParams& params);

        CameraHandle m_cameraHandle;
        // Data layout/instance for data references to VP offset, VP size, frustum planes, frustum near/far planes.
        // By default each reference points to a data instance with values settable/gettable via API (declared below),
        // each can be however bound to external data reference (data object on HL).
        DataLayoutHandle m_dataLayout;
        DataInstanceHandle m_dataInstance;
        // VP offset and size data instances holding values
        DataLayoutHandle m_viewportDataReferenceLayout;
        DataInstanceHandle m_viewportOffsetDataReference;
        DataInstanceHandle m_viewportSizeDataReference;
        // Frustum planes data instance holding values (left, right, bottom, top)
        DataLayoutHandle m_frustumPlanesDataReferenceLayout;
        DataInstanceHandle m_frustumPlanesDataReference;
        // Frustum near/far planes data instance holding values (near, far)
        DataLayoutHandle m_frustumNearFarDataReferenceLayout;
        DataInstanceHandle m_frustumNearFarDataReference;

        bool m_frustumInitialized = false;
        bool m_viewportInitialized = false;
    };
}
