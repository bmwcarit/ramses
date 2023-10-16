//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/CameraBinding.h"
#include "impl/logic/CameraBindingImpl.h"

namespace ramses
{
    CameraBinding::CameraBinding(std::unique_ptr<internal::CameraBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_cameraBinding{ static_cast<internal::CameraBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    ramses::Camera& CameraBinding::getRamsesCamera() const
    {
        return m_cameraBinding.getRamsesCamera();
    }

    internal::CameraBindingImpl& CameraBinding::impl()
    {
        return m_cameraBinding;
    }

    const internal::CameraBindingImpl& CameraBinding::impl() const
    {
        return m_cameraBinding;
    }
}
