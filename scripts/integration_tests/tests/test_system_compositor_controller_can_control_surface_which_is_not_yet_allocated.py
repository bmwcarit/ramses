#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from tests.system_compositor_controller_base import system_compositor_controller_base
from ramses_test_framework.ramses_test_extensions import ensureSystemCompositorRoundTrip, ensureHasContentOnSurface


# Tests if the system compositor controller can control a new surface, which has the same id than a destroyed one before.
class TestSystemCompositorController(system_compositor_controller_base.SystemCompositorControllerBase):

    def impl_test(self):
        # Precondition: renderer visible
        self.validateScreenshot(self.renderer, "scc_only_cube.png", useSystemCompositorForScreenshot=True)

        # Surface is controlled by scc before the application creates the surface
        self.renderer.send_ramsh_command("scastl {0} {1}".format(self.testSurfaceIVIIds["wlClient3"], self.testLayer), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient3"]), waitForRendererConfirmation=True)
        self.expectedSurfaceIds.add("{0}".format(self.testSurfaceIVIIds["wlClient3"]))
        self.assertTrue(self.wait_on_surfaces_beeing_registered_in_scc())
        self.wlClient3 = self.target.start_application(
            "ivi-gears", "-I {0} --still -g 480x480".format(self.testSurfaceIVIIds["wlClient3"]), binaryDirectoryOnTarget=self.target.baseWorkingDirectory)
        self.addCleanup(self.target.kill_application, self.wlClient3)

        # wait for content available on wlClient3 surface before doing screenshot
        ensureHasContentOnSurface(self.renderer, self.testSurfaceIVIIds["wlClient3"])

        ensureSystemCompositorRoundTrip(self.renderer, self.testSurfaceIVIIds["wlClient3"])
        # Postcondition: renderer and gears No. 3 visible
        self.validateScreenshot(self.renderer, "scc_big_red_gear_left_and_cube.png", useSystemCompositorForScreenshot=True)
