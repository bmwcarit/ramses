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
        MOCK_METHOD(void, resourceDestroyed, (), (override));
        MOCK_METHOD(void, surfaceWasDeleted, (), (override));
        MOCK_METHOD(void, bufferWasSetToSurface, (IWaylandBuffer* buffer), (override));
        MOCK_METHOD(WaylandIviSurfaceId, getIviId, (), (const, override));
    };
}

#endif
