#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from tests.embedded_compositor_base import embedded_compositor_base


# The test starts a RAMSES client, that shows several quads with different stream textures configurations
class TestEmbeddedCompositorDmaBufConfidence(embedded_compositor_base.EmbeddedCompositorBase):

    def __init__(self, methodName='runTest'):
        embedded_compositor_base.EmbeddedCompositorBase.__init__(self, methodName, 3)

    def impl_test(self):
        # show the scene
        self.renderer.showScene(34)
        # in the beginning all surfaces show fallback textures
        self.validateScreenshot(self.renderer, "testClient_compositing_allSurfacesFallbackTextures.png")

        # dma buf example should be composited
        self._startDmaBufExample(iviID=2, alternateColors=False)
        self.validateScreenshot(self.renderer, "testClient_compositing_threeSurfacesCompositingDmaBuf.png")

        # dma buf example is killed, fallback is shown again
        self._killDmaBufExample()
        self.validateScreenshot(self.renderer, "testClient_compositing_allSurfacesFallbackTextures.png")

        # the other dma buf example is composited
        self._startDmaBufExample(iviID=4, alternateColors=True)
        self.validateScreenshot(self.renderer, "testClient_compositing_twoSurfacesCompositingDmaBuf.png")

        # the ivi gears is composited
        self._startIviGears(iviID=6, alternateColors=False)
        self.validateScreenshot(self.renderer, "testClient_compositing_threeSurfacesCompositingDmaBufAndIviGears.png")

        # the first dma buf example is composited again
        self._startDmaBufExample(iviID=2, alternateColors=False)
        self.validateScreenshot(self.renderer, "testClient_compositing_fiveSurfacesCompositingDmaBufAndIviGears.png")

        # gears and dma buf are killed (this kills both dma buf applications), fallback is shown again
        self._killIviGears()
        self._killDmaBufExample()
        self.validateScreenshot(self.renderer, "testClient_compositing_allSurfacesFallbackTextures.png")
