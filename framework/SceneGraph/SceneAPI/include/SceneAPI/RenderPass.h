//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_RENDERPASS_H
#define RAMSES_INTERNAL_RENDERPASS_H

#include "SceneAPI/SceneTypes.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{
    struct RenderPass
    {
        bool                   isEnabled = true;
        CameraHandle           camera;
        RenderTargetHandle     renderTarget;
        Int32                  renderOrder = 0;
        Vector4                clearColor{ 0.f, 0.f, 0.f, 1.f };
        UInt32                 clearFlags = EClearFlags_All;
        bool                   isRenderOnce = false;

        RenderGroupOrderVector renderGroups;
    };

    static_assert(std::is_nothrow_move_constructible<RenderPass>::value, "RenderPass must be movable");
    static_assert(std::is_nothrow_move_assignable<RenderPass>::value, "RenderPass must be movable");
}

#endif
