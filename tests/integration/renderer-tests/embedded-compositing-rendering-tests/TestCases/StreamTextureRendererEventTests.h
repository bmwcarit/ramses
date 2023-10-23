//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IEmbeddedCompositingTest.h"

namespace ramses::internal
{
    class StreamTextureRendererEventTests : public IEmbeddedCompositingTest
    {
    public:
        void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            SurfaceAvailableEventGeneratedWhenBufferAttached,
            SurfaceUnavailableEventGeneratedWhenBufferDetached,
            SurfaceUnavailableEventGeneratedWhenSurfaceDestroyed,
            SurfaceUnavailableEventGeneratedWhenIviSurfaceDestroyed,
            SurfaceUnavailableEventGeneratedWhenClientIsKilled,
            NoSurfaceEventGeneratedWhenBufferAttachedAndDetachedInSameLoop,
            SurfaceAvailableAndUnavailableEventsGeneratedWhenBufferAttachedAndDetachedInDifferentLoops,
            SurfaceAvailableEventsGeneratedTwiceWhenBufferReattached,
            SurfaceAvailableEventGeneratedWhenBufferAttached_NoSceneAvailable,
            SurfaceUnavailableEventGeneratedWhenBufferDetached_NoSceneAvailable,
        };

        bool runTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase);

        bool renderAndExpectNoStreamSurfaceAvailabilityChanged(EmbeddedCompositingTestsFramework& testFramework, WaylandIviSurfaceId streamSourceId);
        BoolVector renderAndGetSurfaceAvailabilityChangeEvents(EmbeddedCompositingTestsFramework& testFramework, WaylandIviSurfaceId streamSourceId);
    };
}
