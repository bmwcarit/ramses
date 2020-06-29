//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENESTATE_H
#define RAMSES_RENDERERSCENESTATE_H

#include <stdint.h>

namespace ramses
{
    /// State of a scene on renderer, used with #ramses::RendererSceneControl, #ramses::IRendererSceneControlEventHandler
    /// or #ramses::SceneReference
    enum class RendererSceneState : uint8_t
    {
        Unavailable,  ///< Scene is unavailable (renderer is not subscribed to scene data), scene can be either unpublished or published
        Available,    ///< Scene is available and can be requested to be ready or rendered
        Ready,        ///< Scene is ready to start rendering (its resources are uploaded)
        Rendered      ///< Scene is being rendered
    };
}

#endif
