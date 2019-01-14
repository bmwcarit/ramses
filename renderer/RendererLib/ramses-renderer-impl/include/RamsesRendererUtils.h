//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESRENDERERUTILS_H
#define RAMSES_RAMSESRENDERERUTILS_H

#include "ramses-renderer-api/Types.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "RendererAPI/ELoopMode.h"
#include "RendererLib/EResourceStatus.h"
#include "RendererLib/EKeyCode.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EMouseEventType.h"
#include "RendererLib/WindowedRenderer.h"
#include "Utils/LogMacros.h"

#include <assert.h>

namespace ramses
{
    class RamsesRendererUtils
    {
    public:
        static ESceneResourceStatus GetResourceStatus(ramses_internal::EResourceStatus resourceStatus);
        static ramses::EMouseEvent  GetMouseEvent(    ramses_internal::EMouseEventType type);
        static ramses::ETouchEvent  GetTouchEvent(    ramses_internal::ETouchEventType type);
        static ramses::EKeyEvent    GetKeyEvent(      ramses_internal::EKeyEventType   type);
        static ramses::EKeyCode     GetKeyCode(       ramses_internal::EKeyCode        keyCode);

        static void DoOneLoop(ramses_internal::WindowedRenderer& renderer, ramses_internal::ELoopMode loopMode, std::chrono::microseconds sleepTime);
    };
}

#endif
