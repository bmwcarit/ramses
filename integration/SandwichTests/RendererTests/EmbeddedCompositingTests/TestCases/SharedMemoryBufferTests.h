//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHAREDMEMORYBUFFERTESTS_H
#define RAMSES_SHAREDMEMORYBUFFERTESTS_H

#include "IEmbeddedCompositingTest.h"

namespace ramses_internal
{
    class SharedMemoryBufferTests : public IEmbeddedCompositingTest
    {
    public:
        virtual void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) final;
        virtual bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            //single stream
            ShowSharedMemoryStreamTexture,
            ShowFallbackTextureWhenBufferIsDetachedFromSurface,
            ShowFallbackTextureWhenBufferIsDetachedFromSurfaceAndLastFrameNotUsedForRendering,
            ClientAttachesAndDestroysBufferWithoutCommit,

            //switchting between EGL and shared memory buffers
            SwitchBetweenBufferTypes_ShmThenDestroyShmThenEgl,
            SwitchBetweenBufferTypes_ShmThenEgl,
            SwitchBetweenBufferTypes_EglThenShmThenEgl,
            SwitchBetweenBufferTypes_EglThenShmThenDestroyShmThenEgl,
            SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenEgl,
            SwitchBetweenBufferTypes_ShmAttachedWithoutCommitThenDestroyedThenEgl,
            SwitchBetweenBufferTypes_ConfidenceTest,
            SwitchBetweenBufferTypes_ConfidenceTest_TwoStreams,
            SwitchBetweenBufferTypes_ConfidenceTest_SwizzledFallbackTextures,

            //multiple streams
            ShowSameBufferOnTwoStreamTextures,
            TestCorrectBufferRelease,
        };

        bool renderAndCheckOneSharedMemoryFrame(EmbeddedCompositingTestsFramework& testFramework, TestApplicationSurfaceId testSurfaceId, ETriangleColor color, StreamTextureSourceId streamTextureSourceId, UInt32& frameCount, const String& expectedImageName);
        bool checkFreeBufferState(EmbeddedCompositingTestsFramework& testFramework, const ramses_internal::String& bufferFreeState);
    };
}

#endif
