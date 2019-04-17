//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Window_Wayland_Test/AWindowWaylandWithEventHandling.h"
#include "Window_Wayland_Test/TestCases.h"
#include "Window_Wayland_Shell/Window_Wayland_Shell.h"
#include "PlatformFactoryMock.h"
#include "Platform_Base/PlatformFactory_Base.h"

namespace ramses_internal
{
    NiceMock<PlatformFactoryNiceMock>* gPlatformFactoryMock = NULL;

    ramses_internal::IPlatformFactory* ramses_internal::PlatformFactory_Base::CreatePlatformFactory(const ramses_internal::RendererConfig&)
    {
        gPlatformFactoryMock = new ::testing::NiceMock<PlatformFactoryNiceMock>();
        return gPlatformFactoryMock;
    }

    INSTANTIATE_TYPED_TEST_CASE_P(Window_Wayland_Shell_Test, AWindowWayland, Window_Wayland_Shell);
}
