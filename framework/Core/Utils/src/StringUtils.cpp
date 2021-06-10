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
#include "absl/strings/str_split.h"

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

    std::vector<String> StringUtils::Tokenize(const String& string, const char splitChar)
    {
        return absl::StrSplit(string.stdRef(), splitChar, absl::SkipWhitespace());
    }

    HashSet<String> StringUtils::TokenizeToSet(const String& string, const char split)
    {
        const StringVector vectorOfTokens = Tokenize(string, split);
        HashSet<String> result;
        for(const auto& token : vectorOfTokens)
        {
            result.put(token);
        }
        return result;
    }

    std::vector<String> StringUtils::TokenizeTrimmed(const ramses_internal::String& line, char split)
    {
        const std::vector<ramses_internal::String> allTokens = ramses_internal::StringUtils::Tokenize(line, split);

        // we have to filter out empty tokens
        std::vector<ramses_internal::String> result;
        for(const auto& token : allTokens)
        {
            ramses_internal::String trimmedToken = ramses_internal::StringUtils::Trim(token.c_str());
            if (trimmedToken.size() > 0)
            {
                result.push_back(trimmedToken);
            }
        }
        return result;
    }
}
