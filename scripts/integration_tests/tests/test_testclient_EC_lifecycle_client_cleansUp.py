#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from tests.embedded_compositor_base import embedded_compositor_base
from ramses_test_framework import log


# The test starts a RAMSES client, that shows a cube with six stream textures. Initially, the stream textures
# show their fallback texture. Then a wayland client (ivi-gears) is started, that maps on stream texture 2.
# Finally, the gears client is stopped by SIGINT and cleans-up everything, so the fallback texture is
# shown again.
class TestEmbeddedCompositorLifecycleClientCleansUp(embedded_compositor_base.EmbeddedCompositorBase):

    def __init__(self, methodName ='runTest'):
        embedded_compositor_base.EmbeddedCompositorBase.__init__(self, methodName, 0)

    def impl_test(self):
        # show the scene
        self.renderer.showScene(34)
        # in the beginning all fallback textures are shown, each side has its own
        self.validateScreenshot(self.renderer, "testClient_compositing_fallbacktexture.png")

        # ivi gears should be composited
        self._startIviGears(iviID = 2, alternateColors = False)
        self.validateScreenshot(self.renderer, "testClient_streamtexture_1.png")

        # gears stops and properly cleans up everything, fallbacks are shown again
        self._stopIviGears()
        self.validateScreenshot(self.renderer, "testClient_compositing_fallbacktexture.png")

        # check that given permissions of wayland socket were set correctly
        (stdoutdata, stderrdata, returncode) = self.target.execute_on_target("stat -c '%G' /run/wayland/wayland-10", env=self.target.defaultEnvironment)
        self.assertTrue('mgu_wayland\n' in stdoutdata, "permission group not set correctly on embedded compositing socket")
