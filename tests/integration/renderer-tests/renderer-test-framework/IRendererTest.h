//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "RendererTestsFramework.h"

namespace ramses::internal
{
    class IRendererTest
    {
    public:
        virtual ~IRendererTest() = default;

        virtual void setUpTestCases(RendererTestsFramework& testFramework) = 0;
        virtual bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) = 0;
    };
}
