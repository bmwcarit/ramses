#  -------------------------------------------------------------------------
#  Copyright (C) 2015 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

from tests.embedded_compositor_base import embedded_compositor_base


class TestEmbeddedCompositorMultiFallbackSameSource(embedded_compositor_base.EmbeddedCompositorBase):

    def __init__(self, methodName='runTest'):
        embedded_compositor_base.EmbeddedCompositorBase.__init__(self, methodName, 2)

    def impl_test(self):
        self.percentageOfRGBDifferenceAllowedPerPixel = 0.008  # allows +/- 2 for rgb values (needed e.g. for ufo driver)
        self.percentageOfWrongPixelsAllowed = 0.001  # allow some wrong pixels due to distortion of gears

        # show the scene
        self.renderer.showScene(34)
        # in the beginning all stream textures show different fallbacks
        self.validateScreenshot(self.renderer, "testClient_compositing_fallbacktexture.png")

        # ivi gears is composited
        self._startIviGears(iviID=2, alternateColors=False)
        self.validateScreenshot(self.renderer, "testClient_compositing_sameSourceMultiFallback.png")

        # gears is killed, fallbacks are shown again
        self._killIviGears()
        self.validateScreenshot(self.renderer, "testClient_compositing_fallbacktexture.png")
