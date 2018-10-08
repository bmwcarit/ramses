//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTSTRESSTESTS_H
#define RAMSES_TEXTSTRESSTESTS_H

#include "StressTest.h"
#include "ramses-text-api/FontInstanceId.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/TextLine.h"
#include <memory>

namespace ramses
{
    class Appearance;
    class Effect;
    class MeshNode;
    class TextCache;
}

namespace ramses_internal
{
    class TextStressTestBase : public StressTest
    {
    public:
        TextStressTestBase(int32_t argc, const char* argv[], const ramses_internal::String& name);
        virtual ~TextStressTestBase() override;

    protected:
        virtual void createSceneElements() override;
        virtual void destroyScene() override;

        void doUpdateLoop();
        ramses::TextLineId createText();

        ramses::Effect*          m_textEffect     = nullptr;

        ramses::FontRegistry     m_fontRegistry;
        std::unique_ptr<ramses::TextCache> m_textCache;
        ramses::FontId           m_font           = ramses::InvalidFontId;
        ramses::FontInstanceId   m_fontInst       = ramses::InvalidFontInstanceId;
        ramses::GlyphMetricsVector m_glyphMetrics;
    };

    class CreateDestroyTextNodes : public TextStressTestBase
    {
    public:
        CreateDestroyTextNodes(int32_t argc, const char* argv[]);
        virtual ~CreateDestroyTextNodes();
        int32_t run_loop() override;
    };

    class AddRemoveTextNodes : public TextStressTestBase
    {
    public:
        AddRemoveTextNodes(int32_t argc, const char* argv[]);
        int32_t run_loop() override;
    };
}

#endif
