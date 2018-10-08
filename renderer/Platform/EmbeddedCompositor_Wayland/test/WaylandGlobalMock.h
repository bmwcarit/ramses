//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDGLOBALMOCK_H
#define RAMSES_WAYLANDGLOBALMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandGlobal.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"

namespace ramses_internal
{
    class WaylandGlobalMock : public IWaylandGlobal
    {
    public:
    };
}

#endif
