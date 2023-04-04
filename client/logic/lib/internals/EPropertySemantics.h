//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace rlogic::internal
{
    enum class EPropertySemantics : uint8_t
    {
        ScriptInput,
        BindingInput,
        ScriptOutput,
        AnimationInput,
        AnimationOutput,
        Interface,
    };
}
