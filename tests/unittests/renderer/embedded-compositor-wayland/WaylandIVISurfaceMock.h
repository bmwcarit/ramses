//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandIVISurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandBuffer.h"

namespace ramses::internal
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
