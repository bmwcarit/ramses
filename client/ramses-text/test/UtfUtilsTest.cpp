//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-text-api/UtfUtils.h"

namespace ramses
{
    static std::string toUTF8String(std::initializer_list<uint8_t> codes)
    {
        return{ codes.begin(), codes.end() };
    }

    TEST(UtfUtils, CanConvertUtf32ToUtf16)
    {
        EXPECT_EQ(0xdc00d800u, UtfUtils::Utf32_to_Utf16(0x10000));
        EXPECT_EQ(0xdfffdbbfu, UtfUtils::Utf32_to_Utf16(0xfffff));

        EXPECT_EQ(0x54u, UtfUtils::Utf32_to_Utf16(0x54));
        EXPECT_EQ(0x65u, UtfUtils::Utf32_to_Utf16(0x65));
        EXPECT_EQ(0x73u, UtfUtils::Utf32_to_Utf16(0x73));
        EXPECT_EQ(0x74u, UtfUtils::Utf32_to_Utf16(0x74));
    }

    TEST(UtfUtils, CanReadDifferentLengthUTF8Char)
    {
        const uint32_t numChars = 4;
        const std::string utf8String{ toUTF8String({ 65, 194, 162, 224, 176, 139, 240, 144, 144, 165 }) }; // u8"A¢ఋ𐐥"
        const uint32_t utf32CodePoints[] = { 0x41, 0xa2, 0xc0b, 0x10425 };

        size_t nextOffset = 0;
        for (uint32_t i = 0; i < numChars; ++i)
        {
            const auto result = UtfUtils::ExtractUnicodePointFromUTF8(utf8String.cbegin() + nextOffset, utf8String.cend());
            ASSERT_TRUE(result.extractionSuccessful);
            EXPECT_EQ(result.codePoint, utf32CodePoints[i]);
            //byte length is 1, 2, 3, 4, respectively
            ASSERT_EQ(result.inputCharsConsumed, i+1);
            nextOffset += result.inputCharsConsumed;
        }
    }

    TEST(UtfUtils, HandlesTwoBytesUTF8Errors)
    {
        const std::string utf8_2BytesChar_UnexpectedLeadByte{ toUTF8String({ 0xC2, 0xC2 }) };
        const std::string utf8_2BytesChar_OverLong{ toUTF8String({ 0xC0, 0x81 }) };
        const std::string utf8_2BytesChar_UnexpectedContinuation{ toUTF8String({ 0xA2 }) };

        auto result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_2BytesChar_UnexpectedLeadByte.cbegin(), utf8_2BytesChar_UnexpectedLeadByte.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(2u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_2BytesChar_OverLong.cbegin(), utf8_2BytesChar_OverLong.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(2u, result.inputCharsConsumed);
        EXPECT_EQ(1u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_2BytesChar_UnexpectedContinuation.cbegin(), utf8_2BytesChar_UnexpectedContinuation.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);
    }

    TEST(UtfUtils, HandlesThreeBytesUTF8Errors)
    {
        const std::string utf8_3BytesChar_UnexpectedLeadByte{ toUTF8String({ 0xE2, 0x82, 0xE2 }) };
        const std::string utf8_3BytesChar_OverLong{ toUTF8String({ 0xE0, 0x80, 0x81 }) };
        const std::string utf8_3BytesChar_InvalidChar_UTF16{ toUTF8String({ 0xED, 0xBF, 0xBF, 0x00 }) };

        auto result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_3BytesChar_UnexpectedLeadByte.cbegin(), utf8_3BytesChar_UnexpectedLeadByte.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(2u, result.inputCharsConsumed);
        EXPECT_EQ(130u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_3BytesChar_OverLong.cbegin(), utf8_3BytesChar_OverLong.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(3u, result.inputCharsConsumed);
        EXPECT_EQ(1u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_3BytesChar_InvalidChar_UTF16.cbegin(), utf8_3BytesChar_InvalidChar_UTF16.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(3u, result.inputCharsConsumed);
        EXPECT_EQ(57343u, result.codePoint);
    }

    TEST(UtfUtils, HandlesFourBytesUTF8Errors)
    {
        const std::string utf8_4BytesChar_UnexpectedLeadByte{ toUTF8String({ 0xF0, 0x90, 0x8D, 0xF0 }) };
        const std::string utf8_4BytesChar_OverLong{ toUTF8String({ 0xF0, 0x80, 0x80, 0x81 }) };
        const std::string utf8_4BytesChar_InvalidChar_TooLarge{ toUTF8String({ 0xF4, 0x90, 0x80, 0x80 }) };

        auto result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_4BytesChar_UnexpectedLeadByte.cbegin(), utf8_4BytesChar_UnexpectedLeadByte.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(3u, result.inputCharsConsumed);
        EXPECT_EQ(1037u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_4BytesChar_OverLong.cbegin(), utf8_4BytesChar_OverLong.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(4u, result.inputCharsConsumed);
        EXPECT_EQ(1u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_4BytesChar_InvalidChar_TooLarge.cbegin(), utf8_4BytesChar_InvalidChar_TooLarge.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(4u, result.inputCharsConsumed);
        EXPECT_EQ(1114112u, result.codePoint);
    }

    TEST(UtfUtils, HandlesInvalidUTF8Byte)
    {
        const std::string utf8_InvalidByte{ toUTF8String({ 0xFF }) };

        auto result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_InvalidByte.cbegin(), utf8_InvalidByte.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);
    }

    TEST(UtfUtils, ReadsZeroUTF8Byte)
    {
        const std::string utf8_ZeroByte{ toUTF8String({ 0x00 }) };

        auto result = UtfUtils::ExtractUnicodePointFromUTF8(utf8_ZeroByte.cbegin(), utf8_ZeroByte.cend());
        EXPECT_TRUE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);
    }

    TEST(UtfUtils, HandlesUTF16Errors)
    {
        const std::u16string utf16_NotClosedSurrogate_NewCharacter{ 0xD800, 0x0041 };
        const std::u16string utf16_NotClosedSurrogate_NewSurrogate{ 0xD800, 0xD800 };
        const std::u16string utf16_UnexpectedLowSurrogate{ 0xDC00 };

        auto result = UtfUtils::ExtractUnicodePointFromUTF16(utf16_NotClosedSurrogate_NewCharacter.cbegin(), utf16_NotClosedSurrogate_NewCharacter.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF16(utf16_NotClosedSurrogate_NewSurrogate.cbegin(), utf16_NotClosedSurrogate_NewSurrogate.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF16(utf16_UnexpectedLowSurrogate.cbegin(), utf16_UnexpectedLowSurrogate.cend());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);
    }

    TEST(UtfUtils, ReadsZeroUTF16Word)
    {
        const std::u16string utf16_ZeroWord{ 0x0000 };

        auto result = UtfUtils::ExtractUnicodePointFromUTF16(utf16_ZeroWord.cbegin(), utf16_ZeroWord.cend());
        EXPECT_TRUE(result.extractionSuccessful);
        EXPECT_EQ(1u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);
    }

    TEST(UtfUtils, FailsExtractionFromEmptyOrInvalidRange_UTF16)
    {
        const std::u16string utf16{ 0x0000 };

        auto result = UtfUtils::ExtractUnicodePointFromUTF16(utf16.cbegin(), utf16.cbegin());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(0u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF16(utf16.cend(), utf16.cbegin());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(0u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);
    }

    TEST(UtfUtils, FailsExtractionFromEmptyOrInvalidRange_UTF8)
    {
        const std::string utf8{ 0x00 };

        auto result = UtfUtils::ExtractUnicodePointFromUTF8(utf8.cbegin(), utf8.cbegin());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(0u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);

        result = UtfUtils::ExtractUnicodePointFromUTF8(utf8.cend(), utf8.cbegin());
        EXPECT_FALSE(result.extractionSuccessful);
        EXPECT_EQ(0u, result.inputCharsConsumed);
        EXPECT_EQ(0u, result.codePoint);
    }
}
