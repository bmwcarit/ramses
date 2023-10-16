//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/text/LayoutUtils.h"
#include "ramses/client/text/TextCache.h"
#include "ramses/client/text/FontCascade.h"
#include "ramses/client/text/FontRegistry.h"
#include "ramses/client/RamsesClient.h"
#include <gtest/gtest.h>
#include <algorithm>

namespace ramses::internal
{
    class ALayoutUtilsHMI : public testing::Test
    {
    public:
        ALayoutUtilsHMI()
            : m_textCache(*SScene, *FRegistry, 64u, 64u)
        {
        }

        static void SetUpTestSuite()
        {
            RamsesFrameworkConfig config{EFeatureLevel_Latest};
            Framework = new RamsesFramework{config};
            Client = ALayoutUtilsHMI::Framework->createClient("test");
            SScene = Client->createScene(sceneId_t(1u));

            FRegistry = new FontRegistry;

            const FontId fontLatinId = FRegistry->createFreetype2Font("./res/ramses-text-Roboto-Regular.ttf");
            Latin24 = FRegistry->createFreetype2FontInstance(fontLatinId, 24u);
        }

        static void TearDownTestSuite()
        {
            delete FRegistry;
            Framework->destroyClient(*Client);
            delete Framework;
        }

    protected:
        TextCache m_textCache;

        const std::u32string latinStringWithKerning = U"Test";
        const uint32_t substringOffset = 18u;

        static RamsesFramework* Framework;
        static RamsesClient* Client;
        static Scene* SScene;
        static FontRegistry* FRegistry;
        static FontInstanceId Latin24;
    };

    RamsesFramework* ALayoutUtilsHMI::Framework = nullptr;
    RamsesClient* ALayoutUtilsHMI::Client= nullptr;
    Scene* ALayoutUtilsHMI::SScene = nullptr;

    FontRegistry* ALayoutUtilsHMI::FRegistry(nullptr);
    FontInstanceId ALayoutUtilsHMI::Latin24(0u);

    TEST_F(ALayoutUtilsHMI, CalculatesBoundingBoxCorrectly)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(0, bbox.offsetX);
        EXPECT_EQ(0, bbox.offsetY);
        EXPECT_EQ(47u, bbox.width);
        EXPECT_EQ(17u, bbox.height);
        EXPECT_EQ(47, bbox.combinedAdvance);
    }
}
