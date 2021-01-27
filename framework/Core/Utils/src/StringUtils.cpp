//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/StringUtils.h"
#include "PlatformAbstraction/PlatformStringUtils.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    String StringUtils::Trim(const Char* nativeString)
    {
        std::string result(nativeString);
        result.erase(result.begin(), std::find_if(result.begin(), result.end(),
                                                      [](int ch) { return std::isspace(ch) == 0; }));
        result.erase(std::find_if(result.rbegin(), result.rend(),
                                    [](int ch) { return std::isspace(ch) == 0; }).base(),
                       result.end());
        return String(std::move(result));
    }

    void StringUtils::Tokenize(const String& string, StringVector& tokens, const char splitChar)
    {
        const Int strLength = static_cast<Int>(string.size());
        Int nEnd = 0;
        Int nStart = 0;

        while (nStart < strLength)
        {
            nEnd = string.find(splitChar, nStart);

            if (nEnd < 0)
            {
                nEnd = string.size();
            }

            if (nEnd > nStart)
            {
                String token = string.substr(nStart, nEnd - nStart);
                tokens.push_back(token);
            }

            nStart = nEnd;

            while (nStart < strLength && string.at(nStart) == splitChar)
                ++nStart;
        }
    }

    void StringUtils::Tokenize(const String& string, StringSet& tokens, const char split)
    {
        StringVector vectorOfTokens;
        Tokenize(string, vectorOfTokens, split);
        for(const auto& token : vectorOfTokens)
        {
            tokens.put(token);
        }
    }

    void StringUtils::GetLineTokens(const ramses_internal::String& line, char split, std::vector<ramses_internal::String>& tokens)
    {
        std::vector<ramses_internal::String> allTokens;
        ramses_internal::StringUtils::Tokenize(line, allTokens, split);

        // we have to filter out empty tokens
        tokens.clear();
        for(const auto& token : allTokens)
        {
            ramses_internal::String trimmedToken = ramses_internal::StringUtils::Trim(token.c_str());
            if (trimmedToken.size() > 0)
            {
                tokens.push_back(trimmedToken);
            }
        }
    }
}
