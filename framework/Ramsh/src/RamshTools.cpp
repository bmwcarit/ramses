//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshTools.h"
#include "Collections/String.h"

namespace ramses_internal
{
    UInt RamshTools::delimiterPosition(const String& msg, const String& delimiter)
    {
        const UInt inputLength = msg.getLength();

        UInt delimPos = 0;
        UInt pos = inputLength >= delimiter.getLength() ?
            inputLength - delimiter.getLength()
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

    UInt RamshTools::trailingSpacesPosition(const String& msg, UInt offset)
    {
        UInt pos = offset;

        for(;
            pos > 0
            && ' ' == msg.at(pos);
            pos--)
        {
        }

        return pos;
    }

    UInt RamshTools::leadingSpacesPosition(const String& msg, UInt offset)
    {
        UInt pos = offset;

        for(;
            pos < msg.getLength()
            && ' ' == msg.at(pos);
            pos++)
        {
        }

        return pos;
    }

    RamshInput RamshTools::parseCommandString(const String& msg)
    {
        RamshInput input;

        const String delim = "\r\n";
        const UInt delimPos = msg.getLength() - delimiterPosition(msg, delim);
        const UInt inputLength = delimPos > 0 ? trailingSpacesPosition(msg, delimPos - 1) + 1 : 0;

        String message(msg.c_str(), 0, inputLength - 1);

        for(UInt pos = 0; pos < inputLength;)
        {
            Int endpos = -1;
            pos = leadingSpacesPosition(msg, pos);
            Char c = message.at(pos);

            switch(c)
            {
            case '"':
            case '\'':
                endpos = message.find(String(1,c).append(" "), pos+1); // look for next special separator inside the string
                if(-1 == endpos)
                {
                    endpos = message.lastIndexOf(c); // if no separator found inside, look for special separator on message end
                    if(static_cast<Int>(inputLength) - 1 != endpos)
                    {
                        endpos = message.find(String(1,' '), pos+1); // if no ending separator found, treat as normal
                        pos--;
                    }
                }
                pos++;
                break;
            default:
                endpos = message.find(String(1,' '), pos+1); // look for next separator
            }

            if(-1 == endpos)
            {
                endpos = inputLength; // if no separator found, take all the rest
            }

            input.append(String(message.c_str(), pos, endpos-1)); // append command part

            pos = endpos + 1;
        }

        return input;
    }
}// namespace ramses_internal
