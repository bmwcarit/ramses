//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text-api/IFontAccessor.h"
#include "ramses-text-api/IFontInstance.h"
#include "ramses-text-api/FontCascade.h"
#include <algorithm>

namespace ramses
{
    std::u32string FontCascade::FilterAndFindFontInstancesForString(const FontCascade& fontCascade, const std::u32string& str, FontInstanceOffsets& fontOffsets)
    {
        std::u32string filteredStr;
        filteredStr.reserve(str.size());
        fontOffsets.clear();

        for (char32_t character : str)
        {
            if (0 == character)
                break;

            if (fontCascade.charsToRemove.find(character) != std::u32string::npos)
                continue;

            FontInstanceId fontInstanceToUse;
            const auto it = std::find_if(fontCascade.fontPriorityList.cbegin(), fontCascade.fontPriorityList.cend(), [&fontCascade, character](FontInstanceId fid)
            {
                const auto fontInstance = fontCascade.fontAccessor.getFontInstance(fid);
                return fontInstance != nullptr && fontInstance->supportsCharacter(character);
            });
            if (it != fontCascade.fontPriorityList.cend())
            {
                fontInstanceToUse = *it;
            }
            else
            {
                character = fontCascade.fallbackChar;
                fontInstanceToUse = fontCascade.fallbackFont;
            }

            if (fontOffsets.empty() || fontOffsets.back().fontInstance != fontInstanceToUse)
                fontOffsets.push_back({ fontInstanceToUse, filteredStr.size() });
            filteredStr.push_back(character);
        }

        return filteredStr;
    }
}
