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
        virtual ~SystemCompositorControllerMock() override;

        MOCK_METHOD0(init, Bool());
        MOCK_METHOD0(update, void());
        MOCK_METHOD0(destroy, Bool());
        MOCK_METHOD2(setSurfaceVisibility, Bool(WaylandIviSurfaceId, Bool));
        MOCK_METHOD2(setSurfaceOpacity, Bool(WaylandIviSurfaceId, Float));
        MOCK_METHOD5(setSurfaceDestinationRectangle, Bool(WaylandIviSurfaceId, Int32, Int32, Int32, Int32));
        MOCK_METHOD2(doScreenshot, Bool(const String& fileName, int32_t screenIviId));
        MOCK_METHOD2(addSurfaceToLayer, Bool(WaylandIviSurfaceId, WaylandIviLayerId));
        MOCK_METHOD2(removeSurfaceFromLayer, Bool(WaylandIviSurfaceId, WaylandIviLayerId));
        MOCK_METHOD1(destroySurface, Bool(WaylandIviSurfaceId));
        MOCK_METHOD1(createLayer, Bool(WaylandIviLayerId));
        MOCK_METHOD2(setLayerVisibility, Bool(WaylandIviLayerId, Bool));
        MOCK_CONST_METHOD0(listIVISurfaces, void());
    };

    class SystemCompositorControllerMockWithDestructor : public SystemCompositorControllerMock
    {
    public:
        SystemCompositorControllerMockWithDestructor();
        virtual ~SystemCompositorControllerMockWithDestructor() override;

        MOCK_METHOD0(Die, void());
    };
}

#endif
