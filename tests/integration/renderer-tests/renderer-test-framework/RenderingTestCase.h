//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/DisplayConfig.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

#include <string>
#include <string_view>

namespace ramses::internal
{
    class IRendererTest;

    using DisplayConfigVector = std::vector<ramses::DisplayConfig>;

    struct RenderingTestCase
    {
        RenderingTestCase(uint32_t id, IRendererTest& rendererTest, std::string_view name, bool defaultRendererRequired)
            : m_id(id)
            , m_name(name)
            , m_rendererTest(rendererTest)
            , m_defaultRendererRequired(defaultRendererRequired)
        {
        }

        void enableForAllFeatureLevels(EFeatureLevel startingFrom = EFeatureLevel_01)
        {
            m_featureLevelsToTest.clear();
            for (EFeatureLevel fl = startingFrom; fl <= EFeatureLevel_Latest; fl = EFeatureLevel{ fl + 1 })
                m_featureLevelsToTest.push_back(fl);
        }

        const uint32_t      m_id;
        const std::string   m_name;
        DisplayConfigVector m_displayConfigs;
        IRendererTest&      m_rendererTest;
        bool                m_defaultRendererRequired;
        std::vector<EFeatureLevel> m_featureLevelsToTest{ EFeatureLevel_Latest };
    };
}
