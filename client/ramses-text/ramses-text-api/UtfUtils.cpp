//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text-api/UtfUtils.h"
#include "ramses-text/Logger.h"
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace ramses
{
    namespace UtfUtils
    {
        namespace
        {
            const uint32_t cUniMaxLegalUTF32 = 0x0010ffff;
            const uint32_t cUniSurHighStart = 0xd800;
            const uint32_t cUniSurLowStart = 0xdc00;
            const uint32_t cUniSurHighEnd = 0xdbff;
            const uint32_t cUniSurLowEnd = 0xdfff;
            const uint32_t cHalfBase = 0x0010000;

            const char TrailingBytesForUTF8[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5 };

            bool IsCodePointValid(uint32_t codePoint)
            {
                return codePoint < cUniSurHighStart || (codePoint > cUniSurLowEnd && codePoint <= cUniMaxLegalUTF32);
            }

            bool IsUTF8ContinuationByte(uint8_t byte)
            {
                return (byte & 0xC0) == 0x80;
            }

            bool IsCodePointOverlongUTF8Encoded(uint32_t codepoint, uint32_t numBytes)
            {
                switch (numBytes)
                {
                case 2: return codepoint < 0x0080;
                case 3: return codepoint < 0x0800;
                case 4: return codepoint < 0x10000;
                }
                return false;
            }

            bool IsValidUTF8Byte(uint8_t byte)
            {
                return byte < 254;
            }
            bool IsUTF16HighSurrogate(uint16_t word)
            {
                return cUniSurHighStart <= word && word <= cUniSurHighEnd;
            }

            bool IsUTF16LowSurrogate(uint16_t word)
            {
                return cUniSurLowStart <= word && word <= cUniSurLowEnd;
            }

            bool IsUTF16Surrogate(uint16_t word)
            {
                return IsUTF16HighSurrogate(word) || IsUTF16LowSurrogate(word);
            }
        }

        std::u32string ConvertUtf8ToUtf32(const std::string& utf8String)
        {
            std::u32string utf32String;
            utf32String.reserve(utf8String.size());

            auto strIt = utf8String.cbegin();
            while (strIt < utf8String.cend())
            {
                const ExtractedUnicodePoint extractedCodepoint = ExtractUnicodePointFromUTF8(strIt, utf8String.cend());
                if (extractedCodepoint.extractionSuccessful)
                    utf32String.push_back(extractedCodepoint.codePoint);

                strIt += extractedCodepoint.inputCharsConsumed;
            }

            return utf32String;
        }

        std::string ConvertCharUtf32ToUtf8(char32_t convertChar)
        {
            std::string utf8string;
            if (convertChar <= 127)
            {
                utf8string.push_back(static_cast<char>(convertChar));
            }
            else if (convertChar <= 2047)
            {
                const char32_t firstByte = (convertChar << 21 >> 27) + 192;
                const char32_t lastByte = (convertChar << 26 >> 26) + 128;
                utf8string.push_back(static_cast<char>(firstByte));
                utf8string.push_back(static_cast<char>(lastByte));
            }
            else if (convertChar <= 65535)
            {
                const char32_t firstByte = (convertChar << 16 >> 28) + 0xE0;
                const char32_t secondByte = (convertChar << 20 >> 26) + 0x80;
                const char32_t thirdByte = (convertChar << 26 >> 26) + 128;
                utf8string.push_back(static_cast<char>(firstByte));
                utf8string.push_back(static_cast<char>(secondByte));
                utf8string.push_back(static_cast<char>(thirdByte));
            }
            else if (convertChar <= 1114111)
            {
                const char32_t firstByte = (convertChar << 11 >> 29) + 0xF0;
                const char32_t secondByte = (convertChar << 14 >> 26) + 0x80;
                const char32_t thirdByte = (convertChar << 20 >> 26) + 0x80;
                const char32_t fourthByte = (convertChar << 26 >> 26) + 128;
                utf8string.push_back(static_cast<char>(firstByte));
                utf8string.push_back(static_cast<char>(secondByte));
                utf8string.push_back(static_cast<char>(thirdByte));
                utf8string.push_back(static_cast<char>(fourthByte));
            }
            else
            {
                LOG_TEXT_ERROR("UtfUtils: invalid Unicode " << convertChar << " Skipping Character\n" );
            }

            return utf8string;
        }

        std::string ConvertStrUtf32ToUtf8(const std::u32string& utf32String)
        {
            std::string utf8String;
            utf8String.reserve(2 * utf32String.size());
            for (char32_t character : utf32String)
            {
                utf8String.append(ConvertCharUtf32ToUtf8(character));
            }
            return utf8String;
        }

        ExtractedUnicodePoint ExtractUnicodePointFromUTF8(std::string::const_iterator strBegin, std::string::const_iterator strEnd)
        {
            if (strBegin >= strEnd)
                return { false, 0, 0 };

            ExtractedUnicodePoint result = { true, 0, 0 };
            const auto origBegin = strBegin;

            uint8_t currentCharacterNumBytes = 0;
            uint8_t trailingBytesRemaining = 0;
            while (strBegin < strEnd)
            {
                const uint8_t currentByte = *strBegin++;

                if (!UtfUtils::IsValidUTF8Byte(currentByte))
                {
                    //invalid byte
                    LOG_TEXT_ERROR("UtfUtils: invalid UTF-8 byte " << currentByte);
                    result.extractionSuccessful = false;
                    break;
                }

                if (!UtfUtils::IsUTF8ContinuationByte(currentByte))
                {
                    if (trailingBytesRemaining > 0)
                    {
                        //trailing byte expected, but lead byte found
                        //previous character did not finish correctly
                        LOG_TEXT_ERROR("UtfUtils: unexpected UTF-8 lead byte " << currentByte << "! Aborting previous code point and re-starting from this one");
                        result.extractionSuccessful = false;
                        // TODO Violin this is weird logic... We "unread" this unexpected lead byte and pretend we didn't see the wrong bytes before
                        // Check if there is a better way
                        --strBegin;
                        break;
                    }
                    trailingBytesRemaining = UtfUtils::TrailingBytesForUTF8[currentByte];
                    if (trailingBytesRemaining == 0)
                    {
                        //single byte character
                        result.codePoint = currentByte;
                        break;
                    }
                    else
                    {
                        //2 bytes -> last 5 bits are relevant; 3 bytes -> last 4 bits, etc.
                        const uint8_t mask = (1 << (7 - trailingBytesRemaining)) - 1;
                        result.codePoint = currentByte & mask;
                        currentCharacterNumBytes = trailingBytesRemaining + 1;
                    }
                }
                else
                {
                    if (trailingBytesRemaining > 0)
                    {
                        result.codePoint <<= 6;
                        result.codePoint += currentByte & 0x3F;
                        if (--trailingBytesRemaining == 0)
                        {
                            bool bSuccess = false;
                            if (UtfUtils::IsCodePointValid(result.codePoint))
                            {
                                if (!UtfUtils::IsCodePointOverlongUTF8Encoded(result.codePoint, currentCharacterNumBytes))
                                    bSuccess = true;
                                else
                                    LOG_TEXT_ERROR("UtfUtils::GetNextUTF8CharacterFromStream: overlong encoded UTF-8 character " << result.codePoint);
                            }
                            else
                                LOG_TEXT_ERROR("UtfUtils::GetNextUTF8CharacterFromStream invalid UTF-8 code point " << result.codePoint);

                            // TODO Violin this is wrong! If the codepoint is invalid, it should not be returned
                            // Otherwise this might result in further errors along the text processing stages
                            result.extractionSuccessful = bSuccess;
                            break;
                        }
                    }
                    else
                    {
                        //lead byte expected, but continuation byte, something is wrong
                        LOG_TEXT_ERROR("UtfUtils::GetNextUTF8CharacterFromStream: unexpected UTF-8 continuation byte " << currentByte);
                        result.extractionSuccessful = false;
                        break;
                    }
                }
            }
            result.inputCharsConsumed = std::distance(origBegin, strBegin);

            return result;
        }

        ExtractedUnicodePoint ExtractUnicodePointFromUTF16(std::u16string::const_iterator strBegin, std::u16string::const_iterator strEnd)
        {
            if (strBegin >= strEnd)
                return { false, 0, 0 };

            ExtractedUnicodePoint result = { true, 0, 0 };
            const auto origBegin = strBegin;

            bool lowSurrogateExpected = false;
            while (strBegin < strEnd)
            {
                // TODO Violin add check that this is not a null-terminator
                const uint16_t currentWord = *strBegin++;
                if (!UtfUtils::IsUTF16Surrogate(currentWord))
                {
                    if (lowSurrogateExpected)
                    {
                        LOG_TEXT_ERROR("UtfUtils::ExtractUnicodePointFromUTF16Array: unexpected UTF-16 word " << currentWord << ", low surrogate was expected");
                        // TODO Violin this looks quite weird, check if it is correct to "unread" one word
                        --strBegin;
                        result.extractionSuccessful = false;
                    }
                    else
                    {
                        result.codePoint = currentWord;
                    }
                    break;
                }
                else
                {
                    if (UtfUtils::IsUTF16HighSurrogate(currentWord))
                    {
                        if (lowSurrogateExpected)
                        {
                            LOG_TEXT_ERROR("UtfUtils::ExtractUnicodePointFromUTF16Array: unexpected UTF-16 high surrogate " << currentWord << " in string, low surrogate was expected");
                            // TODO Violin this looks quite weird, check if it is correct to "unread" one word
                            --strBegin;
                            result.extractionSuccessful = false;
                            break;
                        }
                        result.codePoint = (currentWord - UtfUtils::cUniSurHighStart) << 10;
                        lowSurrogateExpected = true;
                    }
                    else
                    {
                        if (!lowSurrogateExpected)
                        {
                            LOG_TEXT_ERROR("UtfUtils::ExtractUnicodePointFromUTF16Array: unexpected UTF-16 low surrogate " << currentWord << " in string, no high surrogate");
                            result.extractionSuccessful = false;
                            break;
                        }
                        else
                        {
                            result.codePoint += currentWord - UtfUtils::cUniSurLowStart + UtfUtils::cHalfBase;
                            break;
                        }
                    }
                }
            }
            result.inputCharsConsumed = std::distance(origBegin, strBegin);

            return result;
        }
    }
}
