#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from tests.embedded_compositor_base import embedded_compositor_base


# The test starts a RAMSES client, that shows several quads with different stream textures configurations
class TestEmbeddedCompositorMultiSourcesMultiFallback(embedded_compositor_base.EmbeddedCompositorBase):

    def __init__(self, methodName ='runTest'):
        embedded_compositor_base.EmbeddedCompositorBase.__init__(self, methodName, 3)

    def impl_test(self):
        # show the scene
        self.renderer.showScene(34)
        # in the beginning all surfaces show fallback textures
        self.validateScreenshot(self.renderer, "testClient_compositing_allSurfacesFallbackTextures.png")

        # ivi gears should be composited
        self._startIviGears(iviID = 2, alternateColors = False)
        self.validateScreenshot(self.renderer, "testClient_compositing_threeSurfacesCompositing.png")

        # gears is killed, fallback is shown again
        self._killIviGears()
        self.validateScreenshot(self.renderer, "testClient_compositing_allSurfacesFallbackTextures.png")

        # the other ivi gears is composited
        self._startIviGears(iviID = 4, alternateColors = True)
        self.validateScreenshot(self.renderer, "testClient_compositing_twoSurfacesCompositing.png")

        # the first ivi gears is composited again
        self._startIviGears(iviID = 2, alternateColors = False)
        self.validateScreenshot(self.renderer, "testClient_compositing_fiveSurfacesCompositing.png")

        # gears is killed (this kills both gears applications), fallback is shown again
        self._killIviGears()
        self.validateScreenshot(self.renderer, "testClient_compositing_allSurfacesFallbackTextures.png")
