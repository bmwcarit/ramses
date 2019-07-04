#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
from tests.system_compositor_controller_base import system_compositor_controller_base
from ramses_test_framework.ramses_test_extensions import ensureSystemCompositorRoundTrip

# Tests if the system compositor controller can control a new surface, which has the same id than a destroyed one before.
class TestSystemCompositorController(system_compositor_controller_base.SystemCompositorControllerBase):

    def impl_test(self):
        # Precondition: renderer and ivi-gears No. 1 and 2 visible
        self.renderer.send_ramsh_command("screct {0} 900 0 384 384".format(self.testSurfaceIVIIds["wlClient1"]), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient1"]), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient2"]), waitForRendererConfirmation=True)
        ensureSystemCompositorRoundTrip(self.renderer, self.testSurfaceIVIIds["wlClient2"])
        self.validateScreenshot(self.renderer, "scc_white_gear_left_red_gear_right_and_cube.png", useSystemCompositorForScreenshot=True)

        # Destroy surface of ivi-gears No. 1
        self.target.ivi_control.destroySurface(self.testSurfaceIVIIds["wlClient1"])
        self.target.ivi_control.flush()
        self.expectedSurfaceIds.remove("{0}".format(self.testSurfaceIVIIds["wlClient1"]))
        self.assertTrue(self.wait_on_surfaces_beeing_registered_in_scc())
        self.validateScreenshot(self.renderer, "scc_white_gear_left_and_cube.png", useSystemCompositorForScreenshot=True)

        # New surface registers with same ivi-id like ivi-gears No. 1 had before
        self.wlClient4 = self.target.start_application(
            "ivi-gears", "-I {0} --still -g 480x480".format(self.testSurfaceIVIIds["wlClient4"]), binaryDirectoryOnTarget=self.target.baseWorkingDirectory)
        self.addCleanup(self.target.kill_application, self.wlClient4)
        self.wlClient4.initialisation_message_to_look_for("time since startup until first eglSwapBuffers done")
        self.expectedSurfaceIds.add("{0}".format(self.testSurfaceIVIIds["wlClient4"]))
        self.assertTrue(self.wait_on_surfaces_beeing_registered_in_scc())
        self.renderer.send_ramsh_command("screct {0} 600 0 480 480".format(self.testSurfaceIVIIds["wlClient4"]), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scastl {0} {1}".format(self.testSurfaceIVIIds["wlClient4"], self.testLayer), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient4"]), waitForRendererConfirmation=True)

        ensureSystemCompositorRoundTrip(self.renderer, self.testSurfaceIVIIds["wlClient4"])
        # Postcondition: renderer and gears No. 2 and 4 visible
        self.validateScreenshot(self.renderer, "scc_white_gear_left_big_red_gear_right_and_cube.png", useSystemCompositorForScreenshot=True)

