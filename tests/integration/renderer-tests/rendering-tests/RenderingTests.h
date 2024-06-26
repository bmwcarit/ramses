//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "RendererTestsFramework.h"
#include "SceneRenderingTests.h"
#include "RenderTargetRenderingTests.h"
#include "RenderPassRenderingTests.h"
#include "EffectRenderingTests.h"
#include "TextureRenderingTests.h"
#include "DisplayRenderingTests.h"
#include "DataLinkingTests.h"
#include "OffscreenBufferLinkTests.h"
#include "internal/Core/Utils/LogMacros.h"
#include "InterruptibleOffscreenBufferLinkTests.h"

#include <vector>
#include <string>

namespace ramses::internal
{
    class RenderingTests
    {
    public:
        RenderingTests(const std::vector<std::string>& filterIn, const std::vector<std::string>& filterOut, bool generateScreenshots, const ramses::RamsesFrameworkConfig& config)
            : m_testFramework(generateScreenshots, config)
            , m_offscreenBufferLinkTests(false)
            , m_offscreenBufferLinkTestsUsingInterruptible(true)
        {
            m_sceneRenderingTests.setUpTestCases(m_testFramework);
            m_renderTargetRenderingTests.setUpTestCases(m_testFramework);
            m_renderPassRenderingTests.setUpTestCases(m_testFramework);
            m_effectRendererTests.setUpTestCases(m_testFramework);
            m_textureRenderingTests.setUpTestCases(m_testFramework);
            m_displayRenderingTests.setUpTestCases(m_testFramework);
            m_dataLinkingTests.setUpTestCases(m_testFramework);
            m_offscreenBufferLinkTests.setUpTestCases(m_testFramework);
            m_offscreenBufferLinkTestsUsingInterruptible.setUpTestCases(m_testFramework);
            m_interruptibleOffscreenBufferLinkTests.setUpTestCases(m_testFramework);

            m_testFramework.filterTestCases(filterIn, filterOut);
        }

        [[nodiscard]] bool runTests()
        {
            return m_testFramework.runAllTests();
        }

        [[nodiscard]] std::string generateReport() const
        {
            return m_testFramework.generateReport();
        }

    protected:
        RendererTestsFramework m_testFramework;

        SceneRenderingTests         m_sceneRenderingTests;
        RenderTargetRenderingTests  m_renderTargetRenderingTests;
        RenderPassRenderingTests    m_renderPassRenderingTests;
        EffectRenderingTests        m_effectRendererTests;
        TextureRenderingTests       m_textureRenderingTests;
        DisplayRenderingTests       m_displayRenderingTests;
        DataLinkingTests            m_dataLinkingTests;
        OffscreenBufferLinkTests    m_offscreenBufferLinkTests;
        OffscreenBufferLinkTests    m_offscreenBufferLinkTestsUsingInterruptible;
        InterruptibleOffscreenBufferLinkTests m_interruptibleOffscreenBufferLinkTests;
    };
}
