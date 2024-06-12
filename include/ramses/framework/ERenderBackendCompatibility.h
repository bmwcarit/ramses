//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace ramses
{
    /**
     * @ingroup CoreAPI
     * Specifies compatibility of scene and its resources with the different renderer devices.
    */
    enum class ERenderBackendCompatibility : uint8_t
    {
        OpenGL,
        VulkanAndOpenGL
    };
}
