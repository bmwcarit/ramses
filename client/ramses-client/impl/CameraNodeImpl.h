//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CAMERANODEIMPL_H
#define RAMSES_CAMERANODEIMPL_H

// API
#include "ramses-framework-api/RamsesFrameworkTypes.h"

// internal
#include "NodeImpl.h"
#include "SceneAPI/ECameraProjectionType.h"
#include "Math3d/ProjectionParams.h"

namespace ramses
{
    class DataVector2i;
    class DataVector2f;
    class DataVector4f;

    class CameraNodeImpl final : public NodeImpl
    {
    public:
        CameraNodeImpl(SceneImpl& scene, ERamsesObjectType cameraType, const char* cameraName);
        virtual ~CameraNodeImpl() override;

        // Common for all camera types
        ramses_internal::ECameraProjectionType getProjectionType() const;

        status_t setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
        int32_t getViewportX() const;
        int32_t getViewportY() const;
        uint32_t getViewportWidth() const;
        uint32_t getViewportHeight() const;

        void             initializeFrameworkData();
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;
        ramses_internal::CameraHandle getCameraHandle() const;

        status_t setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane);
        float    getLeftPlane() const;
        float    getRightPlane() const;
        float    getBottomPlane() const;
        float    getTopPlane() const;
        float    getNearPlane() const;
        float    getFarPlane() const;

        status_t setPerspectiveFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane);
        float    getVerticalFieldOfView() const;
        float    getAspectRatio() const;

        status_t getProjectionMatrix(float(&projectionMatrix)[16]) const;

        status_t bindViewportOffset(const DataVector2i& offsetData);
        status_t bindViewportSize(const DataVector2i& sizeData);
        status_t bindFrustumPlanes(const DataVector4f& frustumPlanesData, const DataVector2f& nearFarData);
        status_t unbindViewportOffset();
        status_t unbindViewportSize();
        status_t unbindFrustumPlanes();
        bool isViewportOffsetBound() const;
        bool isViewportSizeBound() const;
        bool isFrustumPlanesBound() const;

    private:
        ramses_internal::ProjectionParams getProjectionParams() const;
        void                              updateProjectionParamsOnScene(const ramses_internal::ProjectionParams& params);

        ramses_internal::CameraHandle m_cameraHandle;
        // Data layout/instance for data references to VP offset, VP size, frustum planes, frustum near/far planes.
        // By default each reference points to a data instance with values settable/gettable via API (declared below),
        // each can be however bound to external data reference (data object on HL).
        ramses_internal::DataLayoutHandle m_dataLayout;
        ramses_internal::DataInstanceHandle m_dataInstance;
        // VP offset and size data instances holding values
        ramses_internal::DataLayoutHandle m_viewportDataReferenceLayout;
        ramses_internal::DataInstanceHandle m_viewportOffsetDataReference;
        ramses_internal::DataInstanceHandle m_viewportSizeDataReference;
        // Frustum planes data instance holding values (left, right, bottom, top)
        ramses_internal::DataLayoutHandle m_frustumPlanesDataReferenceLayout;
        ramses_internal::DataInstanceHandle m_frustumPlanesDataReference;
        // Frustum near/far planes data instance holding values (near, far)
        ramses_internal::DataLayoutHandle m_frustumNearFarDataReferenceLayout;
        ramses_internal::DataInstanceHandle m_frustumNearFarDataReference;

        bool m_frustumInitialized = false;
        bool m_viewportInitialized = false;
    };
}

#endif
