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

    TEST(UtfUtils, CanConvertUTF32ToUTF8differentOuputByteCounts)
    {
        const std::u32string utf32String{ 0x41, 0xa2, 0xc0b, 0x10425 };
        const std::string utf8String{ toUTF8String({ 65, 194, 162, 224, 176, 139, 240, 144, 144, 165 }) }; // u8"A¢ఋ𐐥"
        EXPECT_EQ(utf8String, UtfUtils::ConvertStrUtf32ToUtf8(utf32String));
    }


    TEST(UtfUtils, CanConvertCharUTF32ToUTF8differentOuputByteCounts)
    {
        // u8"A¢ఋ𐐥"
        EXPECT_EQ(toUTF8String({ 65 }), UtfUtils::ConvertCharUtf32ToUtf8(0x41));
        EXPECT_EQ(toUTF8String({ 194, 162 }), UtfUtils::ConvertCharUtf32ToUtf8(0xa2));
        EXPECT_EQ(toUTF8String({ 224, 176, 139 }), UtfUtils::ConvertCharUtf32ToUtf8(0xc0b));
        EXPECT_EQ(toUTF8String({ 240, 144, 144, 165 }), UtfUtils::ConvertCharUtf32ToUtf8(0x10425));
    }

    TEST(UtfUtils, CanConvertUTF32ToUTF8emptyStr)
    {
        const std::string emptyStr = "";
        const std::u32string emptyU32String;
        EXPECT_EQ(emptyStr, UtfUtils::ConvertStrUtf32ToUtf8(emptyU32String));
    }

    TEST(UtfUtils, CanConvertUTF32ToUTF8ByteBounds)
    {
        const std::u32string utf32String{ 0x7F, 0x80, 0x7FF, 0x800, 0xFFFF, 0x10000};
        const std::string utf8String{ toUTF8String({ 127, 194, 128, 223, 191, 224, 160, 128, 239, 191, 191, 240, 144, 128, 128}) };
        EXPECT_EQ(utf8String, UtfUtils::ConvertStrUtf32ToUtf8(utf32String));
    }

    TEST(UtfUtils, CanConvertUTF32ToUTF8UpperBound)
    {
        const std::u32string utf32String{ 0x10FFFF };
        const std::string utf8String{ toUTF8String({ 0xF4, 0x8F, 0xBF, 0xBF}) };
        EXPECT_EQ(utf8String, UtfUtils::ConvertStrUtf32ToUtf8(utf32String));
    }

    TEST(UtfUtils, CanConvertUTF32ToUTF8zeroInput)
    {
        const std::u32string utf32String{ 0x0 };
        const std::string utf8String{ toUTF8String({ 0x0}) };
        EXPECT_EQ(utf8String, UtfUtils::ConvertStrUtf32ToUtf8(utf32String));
    }

    TEST(UtfUtils, CanConvertUTF32ToUTF8outOfUpperUnicodeRange)
    {
        const std::u32string utf32String{ 0x110000};
        std::string failStr;
        EXPECT_EQ(failStr, UtfUtils::ConvertStrUtf32ToUtf8(utf32String));
    }

    TEST(UtfUtils, CanConvertUTF32ToUTF8containingInvalidChars)
    {
        const std::u32string utf32StringWithInvalidChars{ 0x41, 0x110000, 0x42, 0x110000, 0x43 }; //"A invalChar B invalChar C"
        const std::string utf8String{ toUTF8String({0x41, 0x42, 0x43}) }; //"ABC"
        EXPECT_EQ(utf8String, UtfUtils::ConvertStrUtf32ToUtf8(utf32StringWithInvalidChars));
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
