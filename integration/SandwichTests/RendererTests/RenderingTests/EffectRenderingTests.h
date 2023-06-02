//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTRENDERINGTESTS_H
#define RAMSES_EFFECTRENDERINGTESTS_H

#include "IRendererTest.h"

class EffectRenderingTests : public IRendererTest
{
public:
    void setUpTestCases(RendererTestsFramework& testFramework) final;
    bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

private:
    template <typename INTEGRATION_SCENE>
    bool runBasicTest(RendererTestsFramework& testFramework, ramses_internal::UInt32 sceneState, const std::string& expectedImageName);

    enum
    {
        EffectTest_Shaders = 0,
        EffectTest_Discard_Shaders,
        EffectTest_OptimizedInput,
        EffectTest_UniformWithSameNameInBothStages,
        EffectTest_ExplicitAttributeLocation,
        EffectTest_ExplicitAttributeLocationSwapped,
        EffectTest_StructUniform,
        EffectTest_BoolUniform,
        EffectTest_TextureSize,
    };
};

#endif
