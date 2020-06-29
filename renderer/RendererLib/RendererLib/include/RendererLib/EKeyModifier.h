//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EKEYMODIFIER_H
#define RAMSES_EKEYMODIFIER_H

#include "Common/BitForgeMacro.h"
#include "Collections/String.h"

namespace ramses_internal
{

    enum EKeyModifier : uint32_t
    {
        EKeyModifier_NoModifier = 0,
        EKeyModifier_Ctrl       = BIT(0u),
        EKeyModifier_Shift      = BIT(1u),
        EKeyModifier_Alt        = BIT(2u),
        EKeyModifier_Function   = BIT(3u),
        EKeyModifier_Numpad     = BIT(4u)
    };

    static inline String KeyModifierToString(UInt32 flags)
    {
        String s = "(";
        if (flags == EKeyModifier_NoModifier)
        {
            s.append("EKeyModifier_NoModifier");
        }
        else
        {
            if (flags & EKeyModifier_Ctrl)
            {
                s.append("EKeyModifier_Ctrl | ");
            }
            if (flags & EKeyModifier_Shift)
            {
                s.append("EKeyModifier_Shift | ");
            }
            if (flags & EKeyModifier_Alt)
            {
                s.append("EKeyModifier_Alt | ");
            }
            if (flags & EKeyModifier_Function)
            {
                s.append("EKeyModifier_Function | ");
            }
            if (flags & EKeyModifier_Numpad)
            {
                s.append("EKeyModifier_Numpad | ");
            }
        }
        s.append(")");

        return s;
    };

}
#endif
