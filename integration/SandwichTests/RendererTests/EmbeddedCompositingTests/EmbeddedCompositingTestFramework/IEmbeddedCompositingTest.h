//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IEMBEDDEDCOMPOSITINGTEST_H
#define RAMSES_IEMBEDDEDCOMPOSITINGTEST_H

#include "IRendererTest.h"
#include "EmbeddedCompositingTestsFramework.h"

namespace ramses_internal
{
    class IEmbeddedCompositingTest: public IRendererTest
    {
    public:
        ~IEmbeddedCompositingTest() override {}

        virtual void setUpEmbeddedCompositingTestCases(EmbeddedCompositingTestsFramework& testFramework) = 0;
        virtual bool runEmbeddedCompositingTestCase(EmbeddedCompositingTestsFramework& testFramework, const RenderingTestCase& testCase) = 0;

    private:
        void setUpTestCases(RendererTestsFramework& testFramework) final
        {
            setUpEmbeddedCompositingTestCases(static_cast<EmbeddedCompositingTestsFramework&>(testFramework));
        }

        bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final
        {
            return runEmbeddedCompositingTestCase(static_cast<EmbeddedCompositingTestsFramework&>(testFramework), testCase);
        }
    };
}

#endif
