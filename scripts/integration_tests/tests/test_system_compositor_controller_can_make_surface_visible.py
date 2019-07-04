#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from tests.system_compositor_controller_base import system_compositor_controller_base
from ramses_test_framework.ramses_test_extensions import ensureSystemCompositorRoundTrip

# Tests if the system compositor controller can make a surface visible.
class TestSystemCompositorController(system_compositor_controller_base.SystemCompositorControllerBase):

    def impl_test(self):
        # Precondition: Only renderer visible
        self.validateScreenshot(self.renderer, "scc_only_cube.png", useSystemCompositorForScreenshot=True)

        # Make ivi-gears No. 1 visible
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient1"]), waitForRendererConfirmation=True)

        ensureSystemCompositorRoundTrip(self.renderer, self.testSurfaceIVIIds["wlClient1"])
        # Postcondition: renderer and gears visible
        self.validateScreenshot(self.renderer, "scc_red_gear_left_and_cube.png", useSystemCompositorForScreenshot=True)

