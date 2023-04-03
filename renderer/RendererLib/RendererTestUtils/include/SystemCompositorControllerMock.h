//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLERMOCK_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/ISystemCompositorController.h"


namespace ramses_internal
{
    class SystemCompositorControllerMock : public ISystemCompositorController
    {
    public:
        SystemCompositorControllerMock();
        ~SystemCompositorControllerMock() override;

        MOCK_METHOD(Bool, init, ());
        MOCK_METHOD(void, update, (), (override));
        MOCK_METHOD(Bool, setSurfaceVisibility, (WaylandIviSurfaceId, Bool), (override));
        MOCK_METHOD(Bool, setSurfaceOpacity, (WaylandIviSurfaceId, Float), (override));
        MOCK_METHOD(Bool, setSurfaceDestinationRectangle, (WaylandIviSurfaceId, Int32, Int32, Int32, Int32), (override));
        MOCK_METHOD(Bool, doScreenshot, (const String& fileName, int32_t screenIviId), (override));
        MOCK_METHOD(Bool, addSurfaceToLayer, (WaylandIviSurfaceId, WaylandIviLayerId), (override));
        MOCK_METHOD(Bool, removeSurfaceFromLayer, (WaylandIviSurfaceId, WaylandIviLayerId), (override));
        MOCK_METHOD(Bool, destroySurface, (WaylandIviSurfaceId), (override));
        MOCK_METHOD(Bool, setLayerVisibility, (WaylandIviLayerId, Bool), (override));
        MOCK_METHOD(void, listIVISurfaces, (), (const, override));
    };
}

#endif
