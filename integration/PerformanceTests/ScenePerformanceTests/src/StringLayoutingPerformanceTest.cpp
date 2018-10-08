//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StringLayoutingPerformanceTest.h"
#include "ramses-text-api/UtfUtils.h"
#include "ramses-text-api/LayoutUtils.h"
#include "ramses-text-api/TextCache.h"
#include <fstream>

StringLayoutingPerformanceTest::StringLayoutingPerformanceTest(ramses_internal::String testName, uint32_t testState)
    : PerformanceTestBase(testName, testState)
{
};

void StringLayoutingPerformanceTest::initTest(ramses::RamsesClient&, ramses::Scene& scene)
{
    std::ifstream input("res/BigString.txt", std::ios::binary);
    const std::string fileContents{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
    assert(!fileContents.empty());

    const size_t numCopies = 100u; // copy file contents multiple times to increase total text size
    m_string = ramses::UtfUtils::ConvertUtf8ToUtf32(std::string{ fileContents, 0u, numCopies });

    const auto fontId = m_fontRegistry.createFreetype2Font("res/ramses-test-client-Roboto-Regular.ttf");
    const auto fontInstId = m_fontRegistry.createFreetype2FontInstance(fontId, 24u);

    ramses::TextCache textCache{ scene, m_fontRegistry, 16u, 16u };
    m_glyphs = textCache.getPositionedGlyphs(m_string, fontInstId);
    assert(!m_glyphs.empty());
    m_currentIter = m_glyphs.cbegin();
}

void StringLayoutingPerformanceTest::update()
{
    m_currentIter = ramses::LayoutUtils::FindFittingSubstring(m_currentIter, m_glyphs.cend(), 800u);
    if (m_currentIter == m_glyphs.cend())
        m_currentIter = m_glyphs.cbegin();
}
