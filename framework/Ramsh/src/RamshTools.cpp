//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshTools.h"
#include <PlatformAbstraction/PlatformTypes.h>

namespace ramses_internal
{
    size_t RamshTools::delimiterPosition(const std::string& msg, const std::string& delimiter)
    {
        const size_t inputLength = msg.size();

        size_t delimPos = 0;
        size_t pos = inputLength >= delimiter.size() ?
            inputLength - delimiter.size()
            : 0;

        for(; pos != inputLength; pos++)
        {
            if(delimiter.at(delimPos) == msg.at(pos))
                delimPos++;
            else
                delimPos = 0;
        }

        return delimPos;
    }

    size_t RamshTools::trailingSpacesPosition(const std::string& msg, size_t offset)
    {
        size_t pos = offset;

        for(;
            pos > 0
            && ' ' == msg.at(pos);
            pos--)
        {
        }

        return pos;
    }

    size_t RamshTools::leadingSpacesPosition(const std::string& msg, size_t offset)
    {
        size_t pos = offset;

        for(;
            pos < msg.size()
            && ' ' == msg.at(pos);
            pos++)
        {
        }

        return pos;
    }

    std::vector<std::string> RamshTools::parseCommandString(const std::string& msg)
    {
        std::vector<std::string> result;

        const std::string delim       = "\r\n";
        const size_t      delimPos    = msg.size() - delimiterPosition(msg, delim);
        const size_t      inputLength = delimPos > 0 ? trailingSpacesPosition(msg, delimPos - 1) + 1 : 0;

        std::string message{msg.substr(0, inputLength)};

        for (size_t pos = 0; pos < inputLength;)
        {
            auto endpos  = std::string::npos;
            pos          = leadingSpacesPosition(msg, pos);
            const auto c = message.at(pos);

            switch (c)
            {
            case '"':
            case '\'':
                endpos = message.find(std::string{c}.append(" "), pos + 1); // look for next special separator inside the string
                if (std::string::npos == endpos)
                {
                    endpos = message.rfind(c); // if no separator found inside, look for special separator on message end
                    if (inputLength - 1 != endpos)
                    {
                        endpos = message.find(' ', pos + 1); // if no ending separator found, treat as normal
                        pos--;
                    }
                }
                pos++;
                break;
            default:
                endpos = message.find(' ', pos + 1); // look for next separator
            }

            if (std::string::npos == endpos)
            {
                endpos = inputLength; // if no separator found, take all the rest
            }

            result.push_back(message.substr(pos, endpos - pos)); // append command part

            pos = endpos + 1;
        }

        return result;
    }
} // namespace ramses_internal
