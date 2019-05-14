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

    class CameraNodeImpl final : public NodeImpl
    {
    public:
        CameraNodeImpl(SceneImpl& scene, const char* cameraName, ERamsesObjectType cameraType);
        virtual ~CameraNodeImpl();

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

        virtual status_t validate(uint32_t indent) const override;
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
        status_t unbindViewportOffset();
        status_t unbindViewportSize();
        bool isViewportOffsetBound() const;
        bool isViewportSizeBound() const;

    private:
        ramses_internal::ProjectionParams getProjectionParams() const;
        void                              updateProjectionParamsOnScene(const ramses_internal::ProjectionParams& params);

        ramses_internal::CameraHandle m_cameraHandle;
        ramses_internal::DataLayoutHandle m_viewportDataLayout;
        ramses_internal::DataInstanceHandle m_viewportDataInstance;
        ramses_internal::DataLayoutHandle m_viewportDataReferenceLayout;
        ramses_internal::DataInstanceHandle m_viewportOffsetDataReference;
        ramses_internal::DataInstanceHandle m_viewportSizeDataReference;

        bool m_frustumInitialized;
        bool m_viewportInitialized;
    };
}

#endif
