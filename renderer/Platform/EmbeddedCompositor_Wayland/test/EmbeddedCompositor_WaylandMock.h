//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITOR_WAYLANDMOCK_H
#define RAMSES_EMBEDDEDCOMPOSITOR_WAYLANDMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IEmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/IWaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/IWaylandBuffer.h"
#include "EmbeddedCompositor_Wayland/IWaylandSurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandRegion.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"

namespace ramses_internal
{
    class EmbeddedCompositor_WaylandMock : public IEmbeddedCompositor_Wayland
    {
    public:
        MOCK_METHOD1(handleBufferDestroyed, void(IWaylandBuffer& buffer));
        MOCK_METHOD1(addWaylandSurface, void(IWaylandSurface& waylandSurface));
        MOCK_METHOD1(removeWaylandSurface, void(IWaylandSurface& waylandSurface));
        MOCK_METHOD1(getOrCreateBuffer, IWaylandBuffer&(WaylandBufferResource& bufferResource));
        MOCK_METHOD1(addWaylandCompositorConnection, void(IWaylandCompositorConnection& waylandCompositorConnection));
        MOCK_METHOD1(removeWaylandCompositorConnection, void(IWaylandCompositorConnection& waylandCompositorConnection));
        MOCK_METHOD1(addWaylandRegion, void(IWaylandRegion& waylandRegion));
        MOCK_METHOD1(removeWaylandRegion, void(IWaylandRegion& waylandRegion));
    };
}

#endif
