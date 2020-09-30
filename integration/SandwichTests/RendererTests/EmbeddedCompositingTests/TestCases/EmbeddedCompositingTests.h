//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITINGTESTS_H
#define RAMSES_EMBEDDEDCOMPOSITINGTESTS_H

#include "EmbeddedCompositingTestsFramework.h"
#include "SingleStreamTextureTests.h"
#include "MultiStreamTextureTests.h"
#include "MultiSceneStreamTextureTests.h"
#include "StreamTextureRendererEventTests.h"
#include "OffscreenBuffersWithStreamTexturesTests.h"
#include "WaylandApplicationWithRamsesRendererTests.h"
#include "EmbeddedCompositingTestsWithFD.h"
#include "WaylandOutputTests.h"
#include "SharedMemoryBufferTests.h"

namespace ramses_internal
{
    class TestForkingController;

    class EmbeddedCompositingTests
    {
    public:
        EmbeddedCompositingTests(TestForkingController& testForkingController, const String& waylandSocket, const StringVector& filterIn, const StringVector& filterOut, bool generateScreenshots, const ramses::RamsesFrameworkConfig& config)
            : m_testFramework(generateScreenshots, testForkingController, config)
            , m_embeddedCompositingTestsWithFD(waylandSocket)
        {
            m_singleStreamTextureTests.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_multiStreamTextureTests.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_multiSceneStreamTextureTests.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_streamTextureRendererEventTests.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_offscreenBuffersWithStreamTexturesTests.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_waylandApplicationWithRamsesRendererTests.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_embeddedCompositingTestsWithFD.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_waylandOutputTests.setUpEmbeddedCompositingTestCases(m_testFramework);
            m_sharedMemoryBufferTests.setUpEmbeddedCompositingTestCases(m_testFramework);

            m_testFramework.filterTestCases(filterIn, filterOut);
        }

        bool runTests()
        {
            return m_testFramework.runAllTests();
        }

        void logReport()
        {
            const String reportStr = m_testFramework.generateReport();
            printf("%s\n", reportStr.c_str());
        }

    protected:
        EmbeddedCompositingTestsFramework           m_testFramework;

        SingleStreamTextureTests                    m_singleStreamTextureTests;
        MultiStreamTextureTests                     m_multiStreamTextureTests;
        MultiSceneStreamTextureTests                m_multiSceneStreamTextureTests;
        StreamTextureRendererEventTests             m_streamTextureRendererEventTests;
        OffscreenBuffersWithStreamTexturesTests     m_offscreenBuffersWithStreamTexturesTests;
        WaylandApplicationWithRamsesRendererTests   m_waylandApplicationWithRamsesRendererTests;
        EmbeddedCompositingTestsWithFD              m_embeddedCompositingTestsWithFD;
        WaylandOutputTests                          m_waylandOutputTests;
        SharedMemoryBufferTests                     m_sharedMemoryBufferTests;
    };
}

#endif
