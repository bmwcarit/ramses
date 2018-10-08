//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDIVISURFACEMOCK_H
#define RAMSES_WAYLANDIVISURFACEMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandIVISurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandBuffer.h"

namespace ramses_internal
{
    class WaylandIVISurfaceMock : public IWaylandIVISurface
    {
    public:
        MOCK_METHOD0(resourceDestroyed, void());
        MOCK_METHOD1(iviSurfaceDestroy, void(IWaylandClient& client));
        MOCK_METHOD0(surfaceWasDeleted, void());
        MOCK_METHOD1(bufferWasSetToSurface, void(IWaylandBuffer* buffer));
        MOCK_CONST_METHOD0(getIviId, WaylandIviSurfaceId());
    };
}

#endif
