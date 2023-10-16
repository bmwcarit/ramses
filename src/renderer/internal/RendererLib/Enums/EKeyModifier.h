//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Common/BitForgeMacro.h"
#include "ramses/renderer/Types.h"

#include <string>

namespace ramses::internal
{
    using ramses::EKeyModifier;
    using ramses::KeyModifiers;

    static inline std::string KeyModifierToString(KeyModifiers flags)
    {
        std::string s = "(";
        if (flags == EKeyModifier::NoModifier)
        {
            s.append("EKeyModifier::NoModifier");
        }
        else
        {
            if (flags.isSet(EKeyModifier::Ctrl))
            {
                s.append("EKeyModifier::Ctrl | ");
            }
            if (flags.isSet(EKeyModifier::Shift))
            {
                s.append("EKeyModifier::Shift | ");
            }
            if (flags.isSet(EKeyModifier::Alt))
            {
                s.append("EKeyModifier::Alt | ");
            }
            if (flags.isSet(EKeyModifier::Function))
            {
                s.append("EKeyModifier::Function | ");
            }
            if (flags.isSet(EKeyModifier::Numpad))
            {
                s.append("EKeyModifier::Numpad | ");
            }
        }
        s.append(")");

        return s;
    };

}

