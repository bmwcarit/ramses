//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/glslEffectBlock/GlslangInitializer.h"
#include "GlslangInitializerTestEnivornment.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    // gtest's Environment gets setup before RUN_ALL_TESTS, and gets torn down right after
    // this should be a safe way to init and deinit glslang once per test run
    class GlslangInitializerTestEnivornment : public ::testing::Environment {
    public:
        void SetUp() override
        {
            m_glslangInitializer = std::make_unique<GlslangInitializer>();
        }

        void TearDown() override
        {
            m_glslangInitializer.reset();
        }

    private:
        std::unique_ptr<GlslangInitializer> m_glslangInitializer;
    };

    GlslangInitializerTestEnivornmentSetter::GlslangInitializerTestEnivornmentSetter()
    {
        // gtest takes ownership of the passed Environment, i.e., delete should not be called
        AddGlobalTestEnvironment(new GlslangInitializerTestEnivornment);
    }

    // This static var only has a constructor (no destructor), since it relies on gtest
    // Environment doing proper cleanup
    static GlslangInitializerTestEnivornmentSetter GlslangInitializerTestEnivornmentSetterInstance;
}
