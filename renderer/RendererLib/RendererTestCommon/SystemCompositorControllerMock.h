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

        MOCK_METHOD(bool, init, ());
        MOCK_METHOD(void, update, (), (override));
        MOCK_METHOD(bool, setSurfaceVisibility, (WaylandIviSurfaceId, bool), (override));
        MOCK_METHOD(bool, setSurfaceOpacity, (WaylandIviSurfaceId, float), (override));
        MOCK_METHOD(bool, setSurfaceDestinationRectangle, (WaylandIviSurfaceId, Int32, Int32, Int32, Int32), (override));
        MOCK_METHOD(bool, doScreenshot, (const String& fileName, int32_t screenIviId), (override));
        MOCK_METHOD(bool, addSurfaceToLayer, (WaylandIviSurfaceId, WaylandIviLayerId), (override));
        MOCK_METHOD(bool, removeSurfaceFromLayer, (WaylandIviSurfaceId, WaylandIviLayerId), (override));
        MOCK_METHOD(bool, destroySurface, (WaylandIviSurfaceId), (override));
        MOCK_METHOD(bool, setLayerVisibility, (WaylandIviLayerId, bool), (override));
        MOCK_METHOD(void, listIVISurfaces, (), (const, override));
    };
}

#endif
