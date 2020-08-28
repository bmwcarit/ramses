//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Wayland_Test/AWindowWaylandWithEventHandling.h"
#include "Window_Wayland_Test/TestCases.h"
#include "Window_Wayland_IVI/Window_Wayland_IVI.h"
#include "SystemCompositorController_Wayland_IVI/SystemCompositorController_Wayland_IVI.h"
#include "PlatformFactoryMock.h"
#include "Platform_Base/PlatformFactory_Base.h"

namespace ramses_internal
{
    NiceMock<PlatformFactoryNiceMock>* gPlatformFactoryMock = nullptr;

    ramses_internal::IPlatformFactory* ramses_internal::PlatformFactory_Base::CreatePlatformFactory(const ramses_internal::RendererConfig&)
    {
        gPlatformFactoryMock = new ::testing::NiceMock<PlatformFactoryNiceMock>();
        return gPlatformFactoryMock;
    }

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
