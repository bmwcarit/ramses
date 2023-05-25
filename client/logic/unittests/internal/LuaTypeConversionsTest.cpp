//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "internals/LuaTypeConversions.h"

namespace ramses::internal
{
    class TheLuaTypeConversions : public ::testing::Test
    {
        protected:
            sol::state m_sol;
    };

    TEST_F(TheLuaTypeConversions, ProvidesCorrectIndexUpperBoundsForVecTypes)
    {
        EXPECT_EQ(LuaTypeConversions::GetMaxIndexForVectorType(EPropertyType::Vec2f), 2u);
        EXPECT_EQ(LuaTypeConversions::GetMaxIndexForVectorType(EPropertyType::Vec3f), 3u);
        EXPECT_EQ(LuaTypeConversions::GetMaxIndexForVectorType(EPropertyType::Vec4f), 4u);
        EXPECT_EQ(LuaTypeConversions::GetMaxIndexForVectorType(EPropertyType::Vec2i), 2u);
        EXPECT_EQ(LuaTypeConversions::GetMaxIndexForVectorType(EPropertyType::Vec3i), 3u);
        EXPECT_EQ(LuaTypeConversions::GetMaxIndexForVectorType(EPropertyType::Vec4i), 4u);
    }

    TEST_F(TheLuaTypeConversions, ExtractsStringViewFromSolObject)
    {
        m_sol["a_string"] = "string content";
        const sol::object solObject = m_sol["a_string"];

        const DataOrError<std::string_view> asStringView = LuaTypeConversions::ExtractSpecificType<std::string_view>(solObject);
        EXPECT_EQ("string content", asStringView.getData());
    }

    TEST_F(TheLuaTypeConversions, RecognizesNonStringDataWhenExtractingStringView)
    {
        m_sol["aNumber"] = 5;
        m_sol["aBool"] = true;
        m_sol["aTable"] = m_sol.create_table_with("a", 5);
        m_sol["aFunction"] = [](){};

        std::string errorMsg;

        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<std::string_view>(m_sol["aNumber"]).getError(), ::testing::HasSubstr("Expected a string but got object of type number instead!"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<std::string_view>(m_sol["aBool"]).getError(), ::testing::HasSubstr("Expected a string but got object of type bool instead!"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<std::string_view>(m_sol["aTable"]).getError(), ::testing::HasSubstr("Expected a string but got object of type table instead!"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<std::string_view>(m_sol["aFunction"]).getError(), ::testing::HasSubstr("Expected a string but got object of type function instead!"));
    }

    TEST_F(TheLuaTypeConversions, ExtractsSignedIntegers)
    {
        m_sol["positiveInt"] = 5;
        m_sol["negativeInt"] = -6;
        const sol::object positiveInt = m_sol["positiveInt"];
        const sol::object negativeInt = m_sol["negativeInt"];

        const DataOrError<int32_t> positiveIntOpt = LuaTypeConversions::ExtractSpecificType<int32_t>(positiveInt);
        const DataOrError<int32_t> negativeIntOpt = LuaTypeConversions::ExtractSpecificType<int32_t>(negativeInt);
        const DataOrError<int64_t> positiveInt64Opt = LuaTypeConversions::ExtractSpecificType<int64_t>(positiveInt);
        const DataOrError<int64_t> negativeInt64Opt = LuaTypeConversions::ExtractSpecificType<int64_t>(negativeInt);

        EXPECT_EQ(5, positiveIntOpt.getData());
        EXPECT_EQ(-6, negativeIntOpt.getData());
        EXPECT_EQ(5, positiveInt64Opt.getData());
        EXPECT_EQ(-6, negativeInt64Opt.getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsSignedIntegers_AllowsEpsilonRounding)
    {
        m_sol["positiveIntPlusEps"] = 5.0 + std::numeric_limits<double>::epsilon();
        m_sol["positiveIntMinusEps"] = 5.0 - std::numeric_limits<double>::epsilon();
        m_sol["negativeIntPlusEps"] = -6 + std::numeric_limits<double>::epsilon();
        m_sol["negativeIntMinusEps"] = -6 - std::numeric_limits<double>::epsilon();
        m_sol["zeroMinusEps"] = 0.0 - std::numeric_limits<double>::epsilon();
        const sol::object positiveIntPlusEps = m_sol["positiveIntPlusEps"];
        const sol::object positiveIntMinusEps = m_sol["positiveIntMinusEps"];
        const sol::object negativeIntPlusEps = m_sol["negativeIntPlusEps"];
        const sol::object negativeIntMinusEps = m_sol["negativeIntMinusEps"];
        const sol::object zeroMinusEps = m_sol["zeroMinusEps"];

        EXPECT_EQ(5, LuaTypeConversions::ExtractSpecificType<int32_t>(positiveIntPlusEps).getData());
        EXPECT_EQ(5, LuaTypeConversions::ExtractSpecificType<int32_t>(positiveIntMinusEps).getData());
        EXPECT_EQ(-6, LuaTypeConversions::ExtractSpecificType<int32_t>(negativeIntPlusEps).getData());
        EXPECT_EQ(-6, LuaTypeConversions::ExtractSpecificType<int32_t>(negativeIntMinusEps).getData());
        EXPECT_EQ(0, LuaTypeConversions::ExtractSpecificType<int32_t>(zeroMinusEps).getData());

        EXPECT_EQ(5, LuaTypeConversions::ExtractSpecificType<int64_t>(positiveIntPlusEps).getData());
        EXPECT_EQ(5, LuaTypeConversions::ExtractSpecificType<int64_t>(positiveIntMinusEps).getData());
        EXPECT_EQ(-6, LuaTypeConversions::ExtractSpecificType<int64_t>(negativeIntPlusEps).getData());
        EXPECT_EQ(-6, LuaTypeConversions::ExtractSpecificType<int64_t>(negativeIntMinusEps).getData());
        EXPECT_EQ(0, LuaTypeConversions::ExtractSpecificType<int64_t>(zeroMinusEps).getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsSignedIntegers_ForbidsLargerThanEpsilonRounding)
    {
        m_sol["positiveIntPlusEps"] = 5.0 + 5*std::numeric_limits<double>::epsilon();
        m_sol["positiveIntMinusEps"] = 5.0 - 5*std::numeric_limits<double>::epsilon();
        m_sol["negativeIntPlusEps"] = -6 + 5*std::numeric_limits<double>::epsilon();
        m_sol["negativeIntMinusEps"] = -6 - 5 * std::numeric_limits<double>::epsilon();
        m_sol["zeroMinusEps"] = 0.0 - 5 * std::numeric_limits<double>::epsilon();
        const sol::object positiveIntPlusEps = m_sol["positiveIntPlusEps"];
        const sol::object positiveIntMinusEps = m_sol["positiveIntMinusEps"];
        const sol::object negativeIntPlusEps = m_sol["negativeIntPlusEps"];
        const sol::object negativeIntMinusEps = m_sol["negativeIntMinusEps"];
        const sol::object zeroMinusEps = m_sol["zeroMinusEps"];

        const std::string errorSubstr = "Error while extracting integer: implicit rounding (fractional part";

        // 32 bit
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(positiveIntPlusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(positiveIntMinusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(negativeIntPlusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(negativeIntMinusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(zeroMinusEps).getError(), ::testing::HasSubstr(errorSubstr));

        // 64 bit
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(positiveIntPlusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(positiveIntMinusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(negativeIntPlusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(negativeIntMinusEps).getError(), ::testing::HasSubstr(errorSubstr));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(zeroMinusEps).getError(), ::testing::HasSubstr(errorSubstr));
    }

    TEST_F(TheLuaTypeConversions, ExtractsUnsignedIntegers_AcceptsUpToEpsilonRounding)
    {
        m_sol["okRoundingPos"] = 5.0 + std::numeric_limits<double>::epsilon();
        m_sol["okRoundingNeg"] = 5.0 - std::numeric_limits<double>::epsilon();
        m_sol["zeroMinusEps"] = 0.0 - std::numeric_limits<double>::epsilon();
        m_sol["tooMuchRoundingPos"] = 5.0 + 5 * std::numeric_limits<double>::epsilon();
        m_sol["tooMuchRoundingNeg"] = 5.0 - 5 * std::numeric_limits<double>::epsilon();
        m_sol["zeroRoundingError"] = 0.0 - 5 * std::numeric_limits<double>::epsilon();
        const sol::object okRoundingPos = m_sol["okRoundingPos"];
        const sol::object okRoundingNeg = m_sol["okRoundingNeg"];
        const sol::object zeroMinusEps = m_sol["zeroMinusEps"];

        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(m_sol["tooMuchRoundingPos"]).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(m_sol["tooMuchRoundingNeg"]).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(m_sol["zeroRoundingError"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected non-negative number, received"));

        EXPECT_EQ(5u, LuaTypeConversions::ExtractSpecificType<size_t>(okRoundingPos).getData());
        EXPECT_EQ(5u, LuaTypeConversions::ExtractSpecificType<size_t>(okRoundingNeg).getData());
        EXPECT_EQ(0u, LuaTypeConversions::ExtractSpecificType<size_t>(zeroMinusEps).getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsUnsignedIntegers)
    {
        m_sol["uint"] = 5;
        const sol::object uint = m_sol["uint"];

        EXPECT_EQ(5u, LuaTypeConversions::ExtractSpecificType<size_t>(uint).getData());
    }

    TEST_F(TheLuaTypeConversions, CatchesErrorWhenCastingNegativeNumberToUnsignedInteger)
    {
        m_sol["negative"] = -5;

        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(m_sol["negative"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected non-negative number, received"));
    }

    TEST_F(TheLuaTypeConversions, ExtractsUnsignedIntegers_AllowsEpsilonRounding)
    {
        m_sol["uint"] = 5.0 + std::numeric_limits<double>::epsilon();
        const sol::object uint = m_sol["uint"];

        EXPECT_EQ(5u, LuaTypeConversions::ExtractSpecificType<size_t>(uint).getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsFloats)
    {
        m_sol["float"] = 0.5f;
        m_sol["negFloat"] = -0.5f;
        m_sol["floatWithIntegralPart"] = 1.5f;
        const sol::object float_ = m_sol["float"];
        const sol::object negFloat = m_sol["negFloat"];
        const sol::object floatWithIntegralPart = m_sol["floatWithIntegralPart"];

        EXPECT_FLOAT_EQ(0.5f, LuaTypeConversions::ExtractSpecificType<float>(float_).getData());
        EXPECT_FLOAT_EQ(-0.5f, LuaTypeConversions::ExtractSpecificType<float>(negFloat).getData());
        EXPECT_FLOAT_EQ(1.5f, LuaTypeConversions::ExtractSpecificType<float>(floatWithIntegralPart).getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsExtremeSignedIntegers)
    {
        constexpr int32_t maxIntValue = std::numeric_limits<int32_t>::max();
        constexpr int32_t lowestIntValue = std::numeric_limits<int32_t>::lowest();
        m_sol["maxInt"] = maxIntValue;
        m_sol["lowInt"] = lowestIntValue;
        const sol::object maxInt = m_sol["maxInt"];
        const sol::object lowInt = m_sol["lowInt"];

        EXPECT_EQ(maxIntValue, LuaTypeConversions::ExtractSpecificType<int32_t>(maxInt).getData());
        EXPECT_EQ(lowestIntValue, LuaTypeConversions::ExtractSpecificType<int32_t>(lowInt).getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsExtremeUnsignedIntegers)
    {
        // Maximum size_t is too large for Lua (throws exception as expected), use uint32_t instead
        constexpr size_t maxUIntValue = std::numeric_limits<uint32_t>::max();
        constexpr size_t lowestUIntValue = std::numeric_limits<uint32_t>::lowest();
        m_sol["maxUInt"] = maxUIntValue;
        m_sol["lowUInt"] = lowestUIntValue;
        const sol::object maxUInt = m_sol["maxUInt"];
        const sol::object lowUInt = m_sol["lowUInt"];

        EXPECT_EQ(maxUIntValue, LuaTypeConversions::ExtractSpecificType<size_t>(maxUInt).getData());
        EXPECT_EQ(lowestUIntValue, LuaTypeConversions::ExtractSpecificType<size_t>(lowUInt).getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsExtremeFloats)
    {
        // Test numbers around the boundaries a) of the integral part and b) of the fractional part
        constexpr float maxFloatValue = std::numeric_limits<float>::max();
        constexpr float lowestFloatValue = std::numeric_limits<float>::lowest();
        constexpr float epsilonValue = std::numeric_limits<float>::epsilon();
        constexpr float negEpsilonValue = -std::numeric_limits<float>::epsilon();

        m_sol["maxFloat"] = maxFloatValue;
        m_sol["lowestFloat"] = lowestFloatValue;
        m_sol["epsilon"] = epsilonValue;
        m_sol["negEpsilon"] = negEpsilonValue;

        const sol::object maxFloat = m_sol["maxFloat"];
        const sol::object lowestFloat = m_sol["lowestFloat"];
        const sol::object epsilon = m_sol["epsilon"];
        const sol::object negEpsilon = m_sol["negEpsilon"];

        EXPECT_FLOAT_EQ(maxFloatValue,   LuaTypeConversions::ExtractSpecificType<float>(maxFloat).getData());
        EXPECT_FLOAT_EQ(lowestFloatValue,LuaTypeConversions::ExtractSpecificType<float>(lowestFloat).getData());
        EXPECT_FLOAT_EQ(epsilonValue,    LuaTypeConversions::ExtractSpecificType<float>(epsilon).getData());
        EXPECT_FLOAT_EQ(negEpsilonValue, LuaTypeConversions::ExtractSpecificType<float>(negEpsilon).getData());
    }

    TEST_F(TheLuaTypeConversions, RoundsDoublesToFloats)
    {
        // Test numbers around the boundaries a) of the integral part and b) of the fractional part
        constexpr double dblEpsilon = std::numeric_limits<double>::epsilon() * 10;

        m_sol["onePlusEpsilon"] = 1.0 + dblEpsilon;
        m_sol["oneMinusEpsilon"] = 1.0 - dblEpsilon;

        const sol::object onePlusEpsilon = m_sol["onePlusEpsilon"];
        const sol::object oneMinusEpsilon = m_sol["oneMinusEpsilon"];

        EXPECT_FLOAT_EQ(1.0f, LuaTypeConversions::ExtractSpecificType<float>(onePlusEpsilon).getData());
        EXPECT_FLOAT_EQ(1.0f, LuaTypeConversions::ExtractSpecificType<float>(oneMinusEpsilon).getData());
    }

    TEST_F(TheLuaTypeConversions, ExtractsTableOfFloatsToFloatArray)
    {
        m_sol.script(R"(
            floats = {0.1, 10000.42}
        )");

        const DataOrError<std::array<float, 2>> floatArray = LuaTypeConversions::ExtractArray<float, 2>(m_sol["floats"]);

        EXPECT_FLOAT_EQ(0.1f, floatArray.getData()[0]);
        EXPECT_FLOAT_EQ(10000.42f, floatArray.getData()[1]);
    }

    TEST_F(TheLuaTypeConversions, ExtractsTableOfIntegersToSignedIntegerArray)
    {
        m_sol.script(R"(
            ints = {11, -12, (1.5 - 2.5)}
        )");

        const DataOrError<std::array<int32_t, 3>> intsArray = LuaTypeConversions::ExtractArray<int32_t, 3>(m_sol["ints"]);

        EXPECT_EQ(11, intsArray.getData()[0]);
        EXPECT_EQ(-12, intsArray.getData()[1]);
        EXPECT_EQ(-1, intsArray.getData()[2]);
    }

    TEST_F(TheLuaTypeConversions, FailsValueExtractionWhenSymbolDoesNotExist)
    {
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(m_sol["noSuchSymbol"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected a number, received 'nil'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(m_sol["noSuchSymbol"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected a number, received 'nil'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<float>(m_sol["noSuchSymbol"]).getError(), ::testing::HasSubstr("Error while extracting floating point number: expected a number, received 'nil'"));
    }

    TEST_F(TheLuaTypeConversions, FailsValueExtractionWhenTypesDontMatch)
    {
        m_sol.script(R"(
            integer = 5
            aString = "string"
            aNil = nil
        )");

        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(m_sol["aString"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected a number, received 'string'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(m_sol["aString"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected a number, received 'string'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<float>(m_sol["aString"]).getError(), ::testing::HasSubstr("Error while extracting floating point number: expected a number, received 'string'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(m_sol["aNil"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected a number, received 'nil'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(m_sol["aNil"]).getError(), ::testing::HasSubstr("Error while extracting integer: expected a number, received 'nil'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<float>(m_sol["aNil"]).getError(), ::testing::HasSubstr("Error while extracting floating point number: expected a number, received 'nil'"));
    }

    TEST_F(TheLuaTypeConversions, ReportsErrorWhenTableAndArraySizeDontMatch)
    {
        m_sol.script(R"(
            ints = {11, 12, 13, 14, 15}
        )");

        auto error = LuaTypeConversions::ExtractArray<int32_t, 3>(m_sol["ints"]).getError();
        EXPECT_THAT(error, ::testing::HasSubstr("Error while extracting array: expected 3 array components in table but got 5 instead!"));
    }

    class TheLuaTypeConversions_CatchNumericErrors : public TheLuaTypeConversions
    {
    };

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, WhenNarrowingToSignedIntegers)
    {
        constexpr double largerThanMaxInt32Value = std::numeric_limits<int32_t>::max() + double(1.0);
        constexpr double smallerThanLowestInt32Value = std::numeric_limits<double>::lowest() - double(1.0);
        m_sol["largerThanMaxInt32"] = largerThanMaxInt32Value;
        m_sol["smallerThanLowestInt32"] = smallerThanLowestInt32Value;

        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(m_sol["largerThanMaxInt32"]).getError(),
            ::testing::HasSubstr("Error while extracting integer: integral part too large to fit in a signed 32-bit integer ('2147483648')"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(m_sol["smallerThanLowestInt32"]).getError(),
            ::testing::HasSubstr("Error while extracting integer: integral part too large to fit in a signed 32-bit integer"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, WhenNarrowingFloats)
    {
        // Adding is not enough, have to multiply to get out of float range
        constexpr double largerThanMaxFloatValue = std::numeric_limits<float>::max() * double(2.0);
        constexpr double smallerThanLowestFloatValue = std::numeric_limits<float>::lowest() * double(2.0);
        m_sol["largerThanMaxFloat"] = largerThanMaxFloatValue;
        m_sol["smallerThanLowestFloat"] = smallerThanLowestFloatValue;

        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<float>(m_sol["largerThanMaxFloat"]).getError(),
            ::testing::HasSubstr("Error while extracting floating point number: value would cause overflow in float"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<float>(m_sol["smallerThanLowestFloat"]).getError(),
            ::testing::HasSubstr("Error while extracting floating point number: value would cause overflow in float"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, WhenNarrowingUnsignedIntegers)
    {
        // Adding is not enough, have to multiply to get out of range
        constexpr double largerThanMaxUIntValue = double(std::numeric_limits<size_t>::max()) * 2.0;
        m_sol["largerThanMaxUInt"] = largerThanMaxUIntValue;
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(m_sol["largerThanMaxUInt"]).getError(), ::testing::HasSubstr("Error while extracting integer: integral part too large to fit in 64-bit unsigned integer"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, WhenImplicitlyRoundingFloats)
    {
        // Combinations: positive and negative (X) with and without integral parts
        m_sol["float"] = 0.5f;
        m_sol["negFloat"] = -0.5f;
        m_sol["largerThanOneFloat"] = 1.5f;
        m_sol["smallerThanMinusOne"] = -1.5f;
        const sol::object float_ = m_sol["float"];
        const sol::object negFloat = m_sol["negFloat"];
        const sol::object largerThanOneFloat = m_sol["largerThanOneFloat"];
        const sol::object smallerThanMinusOne = m_sol["smallerThanMinusOne"];

        // Check signed and unsigned types alike, both should fail
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(float_).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(negFloat).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(largerThanOneFloat).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(smallerThanMinusOne).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(float_).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(negFloat).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(largerThanOneFloat).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(smallerThanMinusOne).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(float_).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(negFloat).getError(), ::testing::HasSubstr("Error while extracting integer: expected non-negative number, received '-0.5'"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(largerThanOneFloat).getError(), ::testing::HasSubstr("Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(smallerThanMinusOne).getError(), ::testing::HasSubstr("Error while extracting integer: expected non-negative number, received '-1.5'"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, WhenImplicitlyRoundingFloats_RoundingErrorLargerThanEpsilon)
    {
        // Test numbers around the boundaries a) of the integral part and b) of the fractional part
        constexpr double dblEpsilon = std::numeric_limits<double>::epsilon() * 2;

        m_sol["onePlusEpsilon"] = 1.0 + dblEpsilon;
        m_sol["oneMinusEpsilon"] = 1.0 - dblEpsilon;

        const sol::object onePlusEpsilon = m_sol["onePlusEpsilon"];
        const sol::object oneMinusEpsilon = m_sol["oneMinusEpsilon"];

        const std::string errorMsg = "Error while extracting integer: implicit rounding (fractional part";
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(onePlusEpsilon).getError(), ::testing::HasSubstr(errorMsg));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int32_t>(oneMinusEpsilon).getError(), ::testing::HasSubstr(errorMsg));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(onePlusEpsilon).getError(), ::testing::HasSubstr(errorMsg));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(oneMinusEpsilon).getError(), ::testing::HasSubstr(errorMsg));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(onePlusEpsilon).getError(), ::testing::HasSubstr(errorMsg));
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<size_t>(oneMinusEpsilon).getError(), ::testing::HasSubstr(errorMsg));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, WhenImplicitlyRoundingLargeNumbersToInt64)
    {
        m_sol["dblMax"] = std::numeric_limits<double>::max();
        EXPECT_THAT(LuaTypeConversions::ExtractSpecificType<int64_t>(m_sol["dblMax"]).getError(),
            ::testing::HasSubstr("Error while extracting integer: integral part too large to fit in a signed 64-bit integer ('1.7976931348623157e+308')"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, CatchesError_WhenNarrowing_WhileExtractingIntegerArray)
    {
        m_sol["oneAboveLargestSignedInt"] = double(std::numeric_limits<int32_t>::max()) + 1;
        m_sol.script(R"(
            notOnlyInts = {11, 12, oneAboveLargestSignedInt}
        )");

        const std::string errorMsg = LuaTypeConversions::ExtractArray<int32_t, 3>(m_sol["notOnlyInts"]).getError();
        EXPECT_THAT(errorMsg,
            ::testing::HasSubstr("Error while extracting array: unexpected value (type: 'number') at array element # 3! Reason: Error while extracting integer: integral part too large to fit in a signed 32-bit integer"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, CatchesError_WhenImplicitlyRoundingFloats_WhileExtractingIntegerArray)
    {
        m_sol.script(R"(
            notOnlyInts = {11, 12, 0.5}
        )");

        const std::string errorMsg = LuaTypeConversions::ExtractArray<int32_t, 3>(m_sol["notOnlyInts"]).getError();
        EXPECT_THAT(errorMsg,
            ::testing::HasSubstr("Error while extracting array: unexpected value (type: 'number') at array element # 3! Reason: Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, CatchesError_WhenNegativeFloatFound_WhileExtractingIntegerArray)
    {
        m_sol.script(R"(
            notOnlyInts = {11, 12, -1.5}
        )");

        const std::string errorMsg = LuaTypeConversions::ExtractArray<int32_t, 3>(m_sol["notOnlyInts"]).getError();
        EXPECT_THAT(errorMsg,
            ::testing::HasSubstr(
                "Error while extracting array: unexpected value (type: 'number') at array element # 3! "
                "Reason: Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
    }

    TEST_F(TheLuaTypeConversions_CatchNumericErrors, CatchesError_WhenNarrowing_WhileExtractingFloatArray)
    {
        constexpr double largerThanMaxFloatValue = std::numeric_limits<float>::max() * double(2.0);
        m_sol["largerThanMaxFloat"] = largerThanMaxFloatValue;
        m_sol.script(R"(
            tooLarge = {11, 12, largerThanMaxFloat}
        )");

        const std::string errorMsg = LuaTypeConversions::ExtractArray<float, 3>(m_sol["tooLarge"]).getError();
        EXPECT_THAT(errorMsg,
            ::testing::HasSubstr("Error while extracting array: unexpected value (type: 'number') at array element # 3! Reason: Error while extracting floating point number: value would cause overflow in float"));
    }
}
