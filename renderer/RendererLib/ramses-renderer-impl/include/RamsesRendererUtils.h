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
#include "RendererLib/EResourceStatus.h"
#include "RendererLib/EKeyCode.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EMouseEventType.h"

namespace ramses
{
    class RamsesRendererUtils
    {
    public:
        static EMouseEvent          GetMouseEvent(    ramses_internal::EMouseEventType type);
        static EKeyEvent            GetKeyEvent(      ramses_internal::EKeyEventType   type);
        static EKeyCode             GetKeyCode(       ramses_internal::EKeyCode        keyCode);
    };
}

#endif
