//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AWindowWaylandWithEventHandling.h"
#include "TestCases.h"
#include "internal/Platform/Wayland/IVI/Window_Wayland_IVI.h"
#include "internal/Platform/Wayland/IVI/SystemCompositorController/SystemCompositorController_Wayland_IVI.h"
#include "PlatformMock.h"

namespace ramses::internal
{
    template <> void AWindowWaylandWithEventHandling<Window_Wayland_IVI>::makeWindowVisible()
    {
        SystemCompositorController_Wayland_IVI waylandSystemCompositorController;

        ASSERT_TRUE(waylandSystemCompositorController.init());
        waylandSystemCompositorController.addSurfaceToLayer(WaylandIviSurfaceId(m_config.getWaylandIviSurfaceID().getValue()),
                                                            WaylandIviLayerId(m_config.getWaylandIviLayerID().getValue()));
        waylandSystemCompositorController.setSurfaceVisibility(WaylandIviSurfaceId(m_config.getWaylandIviSurfaceID().getValue()), true);
        waylandSystemCompositorController.update();
    }

    INSTANTIATE_TYPED_TEST_SUITE_P(Window_Wayland_IVI_Test, AWindowWaylandWithEventHandling, Window_Wayland_IVI);
    INSTANTIATE_TYPED_TEST_SUITE_P(Window_Wayland_IVI_Test, AWindowWayland, Window_Wayland_IVI);
}
