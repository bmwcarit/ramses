//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/DataObject.h"

// internal
#include "CameraNodeImpl.h"

namespace ramses
{
    Camera::Camera(std::unique_ptr<CameraNodeImpl> impl)
        : Node{ std::move(impl) }
        , m_impl{ static_cast<CameraNodeImpl&>(Node::m_impl) }
    {
    }

    status_t Camera::setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        const status_t status = m_impl.setFrustum(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        LOG_HL_CLIENT_API6(status, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        return status;
    }

    status_t Camera::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = m_impl.setViewport(x, y, width, height);
        LOG_HL_CLIENT_API4(status, x, y, width, height);
        return status;
    }

    int32_t Camera::getViewportX() const
    {
        return m_impl.getViewportX();
    }

    int32_t Camera::getViewportY() const
    {
        return m_impl.getViewportY();
    }

    uint32_t Camera::getViewportWidth() const
    {
        return m_impl.getViewportWidth();
    }

    uint32_t Camera::getViewportHeight() const
    {
        return m_impl.getViewportHeight();
    }

    float Camera::getLeftPlane() const
    {
        return m_impl.getLeftPlane();
    }

    float Camera::getRightPlane() const
    {
        return m_impl.getRightPlane();
    }

    float Camera::getBottomPlane() const
    {
        return m_impl.getBottomPlane();
    }

    float Camera::getTopPlane() const
    {
        return m_impl.getTopPlane();
    }

    float Camera::getNearPlane() const
    {
        return m_impl.getNearPlane();
    }

    float Camera::getFarPlane() const
    {
        return m_impl.getFarPlane();
    }

    status_t Camera::getProjectionMatrix(matrix44f& projectionMatrix) const
    {
        return m_impl.getProjectionMatrix(projectionMatrix);
    }

    status_t Camera::bindViewportOffset(const DataObject& offsetData)
    {
        const status_t status = m_impl.bindViewportOffset(offsetData);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(offsetData));
        return status;
    }

    status_t Camera::bindViewportSize(const DataObject& sizeData)
    {
        const status_t status = m_impl.bindViewportSize(sizeData);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(sizeData));
        return status;
    }

    status_t Camera::bindFrustumPlanes(const DataObject& frustumPlanesData, const DataObject& nearFarPlanesData)
    {
        const status_t status = m_impl.bindFrustumPlanes(frustumPlanesData, nearFarPlanesData);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(frustumPlanesData), LOG_API_RAMSESOBJECT_STRING(nearFarPlanesData));
        return status;
    }

    status_t Camera::unbindViewportOffset()
    {
        const status_t status = m_impl.unbindViewportOffset();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t Camera::unbindViewportSize()
    {
        const status_t status = m_impl.unbindViewportSize();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t Camera::unbindFrustumPlanes()
    {
        const status_t status = m_impl.unbindFrustumPlanes();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool Camera::isViewportOffsetBound() const
    {
        return m_impl.isViewportOffsetBound();
    }

    bool Camera::isViewportSizeBound() const
    {
        return m_impl.isViewportSizeBound();
    }

    bool Camera::isFrustumPlanesBound() const
    {
        return m_impl.isFrustumPlanesBound();
    }
}
