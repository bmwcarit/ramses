//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/PerspectiveCamera.h"

// internal
#include "CameraNodeImpl.h"

namespace ramses
{
    PerspectiveCamera::PerspectiveCamera(std::unique_ptr<CameraNodeImpl> impl)
        : Camera{ std::move(impl) }
    {
    }

    status_t PerspectiveCamera::setFrustum(float fov, float aspectRatio, float nearPlane, float farPlane)
    {
        const status_t status = m_impl.setPerspectiveFrustum(fov, aspectRatio, nearPlane, farPlane);
        LOG_HL_CLIENT_API4(status, fov, aspectRatio, nearPlane, farPlane);
        return status;
    }

    float PerspectiveCamera::getVerticalFieldOfView() const
    {
        return m_impl.getVerticalFieldOfView();
    }

    float PerspectiveCamera::getAspectRatio() const
    {
        return m_impl.getAspectRatio();
    }
}
