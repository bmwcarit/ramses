//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/ISystemCompositorController.h"
#include "gmock/gmock.h"

namespace ramses::internal
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
        MOCK_METHOD(bool, setSurfaceDestinationRectangle, (WaylandIviSurfaceId, int32_t, int32_t, int32_t, int32_t), (override));
        MOCK_METHOD(bool, doScreenshot, (std::string_view fileName, int32_t screenIviId), (override));
        MOCK_METHOD(bool, addSurfaceToLayer, (WaylandIviSurfaceId, WaylandIviLayerId), (override));
        MOCK_METHOD(bool, removeSurfaceFromLayer, (WaylandIviSurfaceId, WaylandIviLayerId), (override));
        MOCK_METHOD(bool, destroySurface, (WaylandIviSurfaceId), (override));
        MOCK_METHOD(bool, setLayerVisibility, (WaylandIviLayerId, bool), (override));
        MOCK_METHOD(void, listIVISurfaces, (), (const, override));
    };
}
