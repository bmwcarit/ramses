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
        const UInt32 strLen = PlatformStringUtils::StrLen(nativeString);
        if (0 == strLen)
        {
            return String();
        }

        UInt32 firstNonSpace = 0;
        while (firstNonSpace <= strLen - 1 && nativeString[firstNonSpace] == ' ')
            ++firstNonSpace;

        UInt32 lastNonSpace = strLen - 1;
        while (lastNonSpace > firstNonSpace && nativeString[lastNonSpace] == ' ')
            --lastNonSpace;

        if (lastNonSpace <= firstNonSpace)
        {
            return String();
        }

        return String(nativeString, firstNonSpace, lastNonSpace);
    }

    void StringUtils::Tokenize(const String& string, StringVector& tokens, const char split)
    {
        if (string.getLength() > 0)
        {
            Int nEnd = 0;
            Int nStart = 0;
            const char splitString[2] = { split, 0 };

            const Int strLength = static_cast<Int>(string.getLength());
            while (nStart < strLength)
            {
                nEnd = string.find(splitString, nStart);

                if (nEnd < 0)
                {
                    nEnd = string.getLength();
                }

                if (nEnd > nStart)
                {
                    String token = string.substr(nStart, nEnd - nStart);
                    tokens.push_back(token);
                }

                nStart = nEnd;

                while (nStart < strLength && string.at(nStart) == split)
                    ++nStart;
            }
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

    String StringUtils::HexFromResourceContentHash( const ResourceContentHash& n )
    {
        // 2 concatenated uint64_t numbers yield 32 hex chars, prepended by '0x' and a NULL stop byte => 35 chars
        String str(34u, '\0');
        ramses_capu::StringUtils::Sprintf(const_cast<char*>(str.c_str()), 35u, "0x%016llx%016llx", n.highPart, n.lowPart);
        return str;
    }

    String StringUtils::HexFromNumber( uint64_t n )
    {
        // uint64_t yields 16 hex chars, prepended by '0x' and a NULL stop byte => 19 chars
        String str(18u, '\0');
        ramses_capu::StringUtils::Sprintf(const_cast<char*>(str.c_str()), 19u, "0x%016llx", n);
        return str;
    }

    String StringUtils::HexFromNumber( uint32_t n )
    {
        // uint32_t yields 8 hex chars, prepended by '0x' and a NULL stop byte => 11 chars
        String str(10u, '\0');
        ramses_capu::StringUtils::Sprintf(const_cast<char*>(str.c_str()), 11u, "0x%08x", n);
        return str;
    }

    String StringUtils::HexFromNumber( uint8_t n )
    {
        // uint8_t yields 2 hex chars, prepended by '0x' and a NULL stop byte => 5 chars
        String str(4u, '\0');
        ramses_capu::StringUtils::Sprintf(const_cast<char*>(str.c_str()), 5u, "0x%02x", n);
        return str;
    }

    String StringUtils::IToA( int64_t n )
    {
        StringOutputStream sStream;
        sStream << n;
        return sStream.release();
    }

    String StringUtils::IToA( int32_t n )
    {
        return IToA(static_cast<int64_t>(n));
    }

    String StringUtils::IToA( int8_t n )
    {
        return IToA(static_cast<int64_t>(n));
    }

    String StringUtils::IToA( uint64_t n )
    {
        StringOutputStream sStream;
        sStream << n;
        return sStream.release();
    }

    String StringUtils::IToA( uint32_t n )
    {
        return IToA(static_cast<uint64_t>(n));
    }

    String StringUtils::IToA( uint8_t n )
    {
        return IToA(static_cast<uint64_t>(n));
    }


    const String StringUtils::ConvertUTF32ArrayIntoHexString(const std::vector<uint32_t>& data)
    {
        ramses_internal::StringOutputStream sStream;
        sStream.setHexadecimalOutputFormat(ramses_internal::StringOutputStream::EHexadecimalType_HexLeadingZeros);
        sStream << "utf32-str [ ";

        for(auto charCode : data)
        {
            if(charCode != 0)
            {
                sStream << " ; ";
            }

            sStream << charCode;
        }
        sStream << " ]";

        return sStream.release();
    }
}
