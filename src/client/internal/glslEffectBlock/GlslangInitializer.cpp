//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "GlslangInitializer.h"
#include "GLSlang.h"
#include "glslang/build_info.h"

namespace ramses::internal
{
    GlslangInitializer::GlslangInitializer()
    {
        // The function internally has ref counting, which means calling it several times
        // is actually safe, desptite being very inefficient.
        // Unfortunately this behavior relies on internal implementation of glslang, so it must be
        // checked with any glslang update that this behavior can still be relied on, and act accordingly
        static_assert(GLSLANG_VERSION_MAJOR == 14 && GLSLANG_VERSION_MINOR == 1 && GLSLANG_VERSION_PATCH == 0,
            "Check glslang::InitializeProcess() has ref-counting of connected clients for any glslang update");
        glslang::InitializeProcess();
    }

    GlslangInitializer::~GlslangInitializer()
    {
        glslang::FinalizeProcess();
    }
}
