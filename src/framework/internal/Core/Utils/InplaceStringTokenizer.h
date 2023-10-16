//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

namespace ramses::internal
{
    namespace InplaceStringTokenizer
    {
        template <typename F>
        void TokenizeToCStrings(std::string& s, size_t maxStringLength, char splitToken, F&& fun)
        {
            const char* tokenStart = s.c_str();
            size_t curLen = 0;
            for (size_t idx = 0; idx < s.size(); ++idx)
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

        template <typename F>
        void TokenizeToMultilineCStrings(std::string& s, size_t maxBlockSize, char splitToken, F&& fun)
        {
            size_t currentStart = 0;
            size_t nextEnd = 0;
            do {
                nextEnd = currentStart + maxBlockSize;
                if (nextEnd >= s.size())
                {
                    if (currentStart != s.size())
                        fun(s.c_str() + currentStart);
                }
                else
                {
                    const size_t splitPos = s.rfind(splitToken, nextEnd);
                    size_t nextStart = 0;
                    if (splitPos != std::string::npos && splitPos > currentStart)
                    {
                        nextEnd = splitPos;
                        nextStart = nextEnd + 1;
                    }
                    else
                    {
                        nextStart = nextEnd;
                    }

                    if (currentStart != nextEnd)
                    {
                        const char c = s[nextEnd];
                        s[nextEnd] = '\0';
                        fun(s.c_str() + currentStart);
                        s[nextEnd] = c;
                    }

                    currentStart = nextStart;
                }
            } while (nextEnd < s.size());
        }
    }
}
