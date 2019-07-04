#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
from tests.system_compositor_controller_base import system_compositor_controller_base
from ramses_test_framework.ramses_test_extensions import ensureSystemCompositorRoundTrip

# Tests if the system compositor controller can control a new surface, which registers while the renderer is already running.
class TestSystemCompositorController(system_compositor_controller_base.SystemCompositorControllerBase):

    def impl_test(self):
        # Precondition: renderer visible
        self.validateScreenshot(self.renderer, "scc_only_cube.png", useSystemCompositorForScreenshot=True)

        # New surface registers during runtime of renderer
        self.wlClient3 = self.target.start_application(
            "ivi-gears", "-I {0} --still -g 480x480".format(self.testSurfaceIVIIds["wlClient3"]), binaryDirectoryOnTarget=self.target.baseWorkingDirectory)
        self.addCleanup(self.target.kill_application, self.wlClient3)
        self.wlClient3.initialisation_message_to_look_for("time since startup until first eglSwapBuffers done")
        self.expectedSurfaceIds.add("{0}".format(self.testSurfaceIVIIds["wlClient3"]))
        self.assertTrue(self.wait_on_surfaces_beeing_registered_in_scc())
        self.renderer.send_ramsh_command("scastl {0} {1}".format(self.testSurfaceIVIIds["wlClient3"], self.testLayer), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient3"]), waitForRendererConfirmation=True)

        ensureSystemCompositorRoundTrip(self.renderer, self.testSurfaceIVIIds["wlClient3"])
        # Postcondition: renderer and gears No. 3 visible
        self.validateScreenshot(self.renderer, "scc_big_red_gear_left_and_cube.png", useSystemCompositorForScreenshot=True)

