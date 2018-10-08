//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDDISPLAYMOCK_H
#define RAMSES_WAYLANDDISPLAYMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandDisplay.h"

namespace ramses_internal
{
    class WaylandDisplayMock : public IWaylandDisplay
    {
    public:
        MOCK_METHOD3(init, bool(const String& socketName, const String& socketGroupName, int socketFD));
        MOCK_METHOD4(createGlobal, IWaylandGlobal*(const wl_interface* interface, int version, void* data, wl_global_bind_func_t bind));
        MOCK_METHOD0(dispatchEventLoop, void());
        MOCK_METHOD0(flushClients, void());
    };
}

#endif
