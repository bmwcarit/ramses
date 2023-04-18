//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/RamsesCameraBinding.h"
#include "impl/RamsesCameraBindingImpl.h"

namespace rlogic
{
    RamsesCameraBinding::RamsesCameraBinding(std::unique_ptr<internal::RamsesCameraBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_cameraBinding{ static_cast<internal::RamsesCameraBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    RamsesCameraBinding::~RamsesCameraBinding() noexcept = default;

    ramses::Camera& RamsesCameraBinding::getRamsesCamera() const
    {
        return m_cameraBinding.getRamsesCamera();
    }
}
