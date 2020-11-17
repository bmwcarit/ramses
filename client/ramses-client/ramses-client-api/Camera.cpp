//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector4f.h"

// internal
#include "CameraNodeImpl.h"

namespace ramses
{
    Camera::Camera(CameraNodeImpl& pimpl)
        : Node(pimpl)
        , impl(pimpl)
    {
    }

    Camera::~Camera() = default;

    status_t Camera::setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        const status_t status = impl.setFrustum(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        LOG_HL_CLIENT_API6(status, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        return status;
    }

    status_t Camera::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = impl.setViewport(x, y, width, height);
        LOG_HL_CLIENT_API4(status, x, y, width, height);
        return status;
    }

    int32_t Camera::getViewportX() const
    {
        return impl.getViewportX();
    }

    int32_t Camera::getViewportY() const
    {
        return impl.getViewportY();
    }

    uint32_t Camera::getViewportWidth() const
    {
        return impl.getViewportWidth();
    }

    uint32_t Camera::getViewportHeight() const
    {
        return impl.getViewportHeight();
    }

    float Camera::getLeftPlane() const
    {
        return impl.getLeftPlane();
    }

    float Camera::getRightPlane() const
    {
        return impl.getRightPlane();
    }

    float Camera::getBottomPlane() const
    {
        return impl.getBottomPlane();
    }

    float Camera::getTopPlane() const
    {
        return impl.getTopPlane();
    }

    float Camera::getNearPlane() const
    {
        return impl.getNearPlane();
    }

    float Camera::getFarPlane() const
    {
        return impl.getFarPlane();
    }

    status_t Camera::getProjectionMatrix(float(&projectionMatrix)[16]) const
    {
        return impl.getProjectionMatrix(projectionMatrix);
    }

    status_t Camera::bindViewportOffset(const DataVector2i& offsetData)
    {
        const status_t status = impl.bindViewportOffset(offsetData);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(offsetData));
        return status;
    }

    status_t Camera::bindViewportSize(const DataVector2i& sizeData)
    {
        const status_t status = impl.bindViewportSize(sizeData);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(sizeData));
        return status;
    }

    status_t Camera::bindFrustumPlanes(const DataVector4f& frustumPlanesData, const DataVector2f& nearFarPlanesData)
    {
        const status_t status = impl.bindFrustumPlanes(frustumPlanesData, nearFarPlanesData);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(frustumPlanesData), LOG_API_RAMSESOBJECT_STRING(nearFarPlanesData));
        return status;
    }

    status_t Camera::unbindViewportOffset()
    {
        const status_t status = impl.unbindViewportOffset();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t Camera::unbindViewportSize()
    {
        const status_t status = impl.unbindViewportSize();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t Camera::unbindFrustumPlanes()
    {
        const status_t status = impl.unbindFrustumPlanes();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool Camera::isViewportOffsetBound() const
    {
        return impl.isViewportOffsetBound();
    }

    bool Camera::isViewportSizeBound() const
    {
        return impl.isViewportSizeBound();
    }

    bool Camera::isFrustumPlanesBound() const
    {
        return impl.isFrustumPlanesBound();
    }
}
