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
#include "PlatformMock.h"

namespace ramses_internal
{
    INSTANTIATE_TYPED_TEST_SUITE_P(Window_Wayland_Shell_Test, AWindowWayland, Window_Wayland_Shell);
}
