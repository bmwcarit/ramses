//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/StringUtils.h"

namespace
{
    template <typename F>
    void tokenizeTo(std::string_view string, const char splitChar, F f)
    {
        while (!string.empty())
        {
            const auto pos = string.find(splitChar);
            if (pos == string.npos)
            {
                f(string);
                break;
            }
            if (const auto s = string.substr(0, pos); s.size() != 0 && s != " ")
            {
                f(s);
            }
            string.remove_prefix(pos + 1);
        }
    }
} // namespace

namespace ramses_internal
{
    String StringUtils::Trim(std::string_view string)
    {
        return String{TrimView(string)};
    }

    std::string_view StringUtils::TrimView(std::string_view string)
    {
        if (const auto pos = string.find_first_not_of("\t\n\v\f\r "); pos != string.npos)
        {
            string.remove_prefix(pos);
        }
        else
        {
            return {};
        }

        if (const auto pos = string.find_last_not_of("\t\n\v\f\r "); pos != string.npos)
        {
            string.remove_suffix(string.size() - pos - 1);
        }

        return string;
    }

    std::vector<String> StringUtils::Tokenize(std::string_view string, const char splitChar)
    {
        std::vector<String> result{};
        auto f = [&result](const auto token) { result.emplace_back(token); };
        tokenizeTo(string, splitChar, f);
        return result;
    }

    HashSet<String> StringUtils::TokenizeToSet(std::string_view string, const char split)
    {
        HashSet<String> result{};
        auto f = [&result](const auto token) { result.put(String{token}); };
        tokenizeTo(string, split, f);
        return result;
    }
} // namespace ramses_internal
