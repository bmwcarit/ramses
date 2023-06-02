//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERINGTESTCASE_H
#define RAMSES_RENDERINGTESTCASE_H

#include "ramses-renderer-api/DisplayConfig.h"
#include "Collections/Vector.h"

#include <string>
#include <string_view>

class IRendererTest;

using DisplayConfigVector = std::vector<ramses::DisplayConfig>;

struct RenderingTestCase
{
    RenderingTestCase(ramses_internal::UInt32 id, IRendererTest& rendererTest, std::string_view name, bool defaultRendererRequired)
        : m_id(id)
        , m_name(name)
        , m_rendererTest(rendererTest)
        , m_defaultRendererRequired(defaultRendererRequired)
    {
    }

    const ramses_internal::UInt32   m_id;
    const std::string               m_name;
    DisplayConfigVector             m_displayConfigs;
    IRendererTest&                  m_rendererTest;
    bool                            m_defaultRendererRequired;
};

#endif
