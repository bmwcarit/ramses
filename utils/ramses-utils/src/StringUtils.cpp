//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StringUtils.h"
#include "Utils/StringUtils.h"

void StringUtils::GetLineTokens(const ramses_internal::String& line, char split, std::vector<ramses_internal::String>& tokens)
{
    std::vector<ramses_internal::String> allTokens;
    ramses_internal::StringUtils::Tokenize(line, allTokens, split);

    // due to the wired behavior of tokenize provided by capu, we have to filter out those empty tokens.
    tokens.clear();
    for(const auto& token : allTokens)
    {
        ramses_internal::String trimmedToken = ramses_internal::StringUtils::Trim(token.c_str());
        if (trimmedToken.getLength() > 0)
        {
            tokens.push_back(trimmedToken);
        }
    }
}
