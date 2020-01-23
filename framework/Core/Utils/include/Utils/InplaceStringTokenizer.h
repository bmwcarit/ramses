//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_INPLACESTRINGTOKENIZER_H
#define RAMSES_UTILS_INPLACESTRINGTOKENIZER_H

#include "Collections/String.h"

namespace ramses_internal
{
    namespace InplaceStringTokenizer
    {
        template <typename F>
        void TokenizeToCStrings(String& s, UInt maxStringLength, Char splitToken, F&& fun)
        {
            const char* tokenStart = s.c_str();
            UInt curLen = 0;
            for (UInt idx = 0; idx < s.size(); ++idx)
            {
                if (s[idx] == splitToken || curLen == maxStringLength)
                {
                    const char c = s[idx];
                    s[idx] = '\0';
                    fun(tokenStart);
                    s[idx] = c;

                    tokenStart = s.c_str() + idx;
                    if (c == splitToken)
                    {
                        ++tokenStart;
                        curLen = 0;
                    }
                    else
                    {
                        curLen = 1;
                    }
                }
                else
                {
                    ++curLen;
                }
            }
            if (curLen > 0)
            {
                fun(tokenStart);
            }
        }
    }
}

#endif
