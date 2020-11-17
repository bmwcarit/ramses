//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STREAMTEXTURERENDEREREVENTTESTS_H
#define RAMSES_STREAMTEXTURERENDEREREVENTTESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class StreamTextureRendererEventTests : public IEmbeddedCompositingTest
    {
    public:
        virtual void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) override final;
        virtual bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) override final;

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

#endif
