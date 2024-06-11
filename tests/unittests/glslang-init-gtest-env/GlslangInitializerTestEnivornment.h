//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    // This makes sure that glslang init/deinit is called maximum once per test run to reduce execution time
    class GlslangInitializerTestEnivornmentSetter
    {
    public:
        GlslangInitializerTestEnivornmentSetter();
    };
}
