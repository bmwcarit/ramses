//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/LocalCamera.h"

// internal
#include "CameraNodeImpl.h"

namespace ramses
{
    LocalCamera::LocalCamera(CameraNodeImpl& pimpl)
        : Camera(pimpl)
    {
    }

    LocalCamera::~LocalCamera()
    {
    }

    status_t LocalCamera::setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        const status_t status = impl.setFrustum(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        LOG_HL_CLIENT_API6(status, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        return status;
    }

    status_t LocalCamera::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = impl.setViewport(x, y, width, height);
        LOG_HL_CLIENT_API4(status, x, y, width, height)
        return status;
    }

    uint32_t LocalCamera::getViewportX() const
    {
        return impl.getViewportX();
    }

    uint32_t LocalCamera::getViewportY() const
    {
        return impl.getViewportY();
    }

    uint32_t LocalCamera::getViewportWidth() const
    {
        return impl.getViewportWidth();
    }

    uint32_t LocalCamera::getViewportHeight() const
    {
        return impl.getViewportHeight();
    }

    float LocalCamera::getLeftPlane() const
    {
        return impl.getLeftPlane();
    }

    float LocalCamera::getRightPlane() const
    {
        return impl.getRightPlane();
    }

    float LocalCamera::getBottomPlane() const
    {
        return impl.getBottomPlane();
    }

    float LocalCamera::getTopPlane() const
    {
        return impl.getTopPlane();
    }

    float LocalCamera::getNearPlane() const
    {
        return impl.getNearPlane();
    }

    float LocalCamera::getFarPlane() const
    {
        return impl.getFarPlane();
    }

    status_t LocalCamera::getProjectionMatrix(float(&projectionMatrix)[16]) const
    {
        return impl.getProjectionMatrix(projectionMatrix);
    }
}
