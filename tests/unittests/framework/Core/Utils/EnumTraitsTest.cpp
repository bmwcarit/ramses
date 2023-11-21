//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/EnumTraits.h"
#include <gtest/gtest.h>

#if defined(RAMSES_HAS_ENUMTRAITS)

namespace ramses::internal
{
    enum class EnumClass
    {
        Value1, Value2
    };

    enum class EnumClassU8 : uint8_t
    {
        Value1, Value2
    };

    enum EFoo
    {
        EFoo_Value1, EFoo_Value2
    };

    enum EFooU16 : uint16_t
    {
        EFooU16_Value1, EFooU16_Value2
    };

    TEST(EnumTraits, isSupported)
    {
        static_assert(EnumTraits::IsSupported());
    }

    TEST(EnumTraits, isValidEnumClass)
    {
        static_assert(EnumTraits::internal::IsValid<EnumClass, EnumClass::Value1>());
        static_assert(EnumTraits::internal::IsValid<EnumClass, EnumClass::Value2>());
        static_assert(EnumTraits::internal::IsValid<EnumClass, static_cast<EnumClass>(0)>());
        static_assert(EnumTraits::internal::IsValid<EnumClass, static_cast<EnumClass>(1)>());
        static_assert(!EnumTraits::internal::IsValid<EnumClass, static_cast<EnumClass>(2)>());
        static_assert(!EnumTraits::internal::IsValid<EnumClass, static_cast<EnumClass>(-1)>());
        static_assert(!EnumTraits::internal::IsValid<EnumClass, static_cast<EnumClass>(15)>());
    }

    TEST(EnumTraits, isValidEnumClassU8)
    {
        static_assert(EnumTraits::internal::IsValid<EnumClassU8, EnumClassU8::Value1>());
        static_assert(EnumTraits::internal::IsValid<EnumClassU8, EnumClassU8::Value2>());
        static_assert(EnumTraits::internal::IsValid<EnumClassU8, static_cast<EnumClassU8>(0)>());
        static_assert(EnumTraits::internal::IsValid<EnumClassU8, static_cast<EnumClassU8>(1)>());
        static_assert(!EnumTraits::internal::IsValid<EnumClassU8, static_cast<EnumClassU8>(2)>());
        static_assert(!EnumTraits::internal::IsValid<EnumClassU8, static_cast<EnumClassU8>(-1)>());
        static_assert(!EnumTraits::internal::IsValid<EnumClassU8, static_cast<EnumClassU8>(15)>());
    }

    TEST(EnumTraits, isValidEnum)
    {
        static_assert(EnumTraits::internal::IsValid<EFoo, EFoo_Value1>());
        static_assert(EnumTraits::internal::IsValid<EFoo, EFoo_Value2>());
        static_assert(EnumTraits::internal::IsValid<EFoo, static_cast<EFoo>(0)>());
        static_assert(EnumTraits::internal::IsValid<EFoo, static_cast<EFoo>(1)>());
        static_assert(!EnumTraits::internal::IsValid<EFoo, static_cast<EFoo>(2)>());
        static_assert(!EnumTraits::internal::IsValid<EFoo, static_cast<EFoo>(-1)>());
        static_assert(!EnumTraits::internal::IsValid<EFoo, static_cast<EFoo>(15)>());
    }

    TEST(EnumTraits, isValidEnumU16)
    {
        static_assert(EnumTraits::internal::IsValid<EFooU16, EFooU16_Value1>());
        static_assert(EnumTraits::internal::IsValid<EFooU16, EFooU16_Value2>());
        static_assert(EnumTraits::internal::IsValid<EFooU16, static_cast<EFooU16>(0)>());
        static_assert(EnumTraits::internal::IsValid<EFooU16, static_cast<EFooU16>(1)>());
        static_assert(!EnumTraits::internal::IsValid<EFooU16, static_cast<EFooU16>(2)>());
        static_assert(!EnumTraits::internal::IsValid<EFooU16, static_cast<EFooU16>(-1)>());
        static_assert(!EnumTraits::internal::IsValid<EFooU16, static_cast<EFooU16>(15)>());
    }

    TEST(EnumTraits, ElementCount0)
    {
        enum class Enum
        {
        };
        static_assert(0 == EnumTraits::internal::ElementCount<Enum>::value);
        static_assert(EnumTraits::VerifyElementCountIfSupported<Enum>(0));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(1));
    }


    TEST(EnumTraits, ElementCount1)
    {
        enum class Enum
        {
            A
        };
        static_assert(1 == EnumTraits::internal::ElementCount<Enum>::value);
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(0));
        static_assert(EnumTraits::VerifyElementCountIfSupported<Enum>(1));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(2));
    }

    TEST(EnumTraits, ElementCount1Offset)
    {
        enum class Enum
        {
            A = 25
        };
        static_assert(1 == EnumTraits::internal::ElementCount<Enum>::value);
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(0));
        static_assert(EnumTraits::VerifyElementCountIfSupported<Enum>(1));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(2));
    }

    TEST(EnumTraits, ElementCount3Offset)
    {
        enum class Enum
        {
            A, B = 2, C
        };
        static_assert(3 == EnumTraits::internal::ElementCount<Enum>::value);
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(0));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(1));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(2));
        static_assert(EnumTraits::VerifyElementCountIfSupported<Enum>(3));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(4));
    }

    TEST(EnumTraits, ElementCount3)
    {
        enum class Enum
        {
            A, B, C
        };
        static_assert(3 == EnumTraits::internal::ElementCount<Enum>::value);
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(0));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(1));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(2));
        static_assert(EnumTraits::VerifyElementCountIfSupported<Enum>(3));
        static_assert(!EnumTraits::VerifyElementCountIfSupported<Enum>(4));
    }

    TEST(EnumTraits, ElementCount127)
    {
        enum class Enum
        {
            E00, E01, E02, E03, E04, E05, E06, E07, E08, E09,
            E10, E11, E12, E13, E14, E15, E16, E17, E18, E19,
            E20, E21, E22, E23, E24, E25, E26, E27, E28, E29,
            E30, E31, E32, E33, E34, E35, E36, E37, E38, E39,
            E40, E41, E42, E43, E44, E45, E46, E47, E48, E49,
            E50, E51, E52, E53, E54, E55, E56, E57, E58, E59,
            E60, E61, E62, E63, E64, E65, E66, E67, E68, E69,
            E70, E71, E72, E73, E74, E75, E76, E77, E78, E79,
            E80, E81, E82, E83, E84, E85, E86, E87, E88, E89,
            E90, E91, E92, E93, E94, E95, E96, E97, E98, E99,
            E100, E101, E102, E103, E104, E105, E106, E107, E108, E109,
            E110, E111, E112, E113, E114, E115, E116, E117, E118, E119,
            E120, E121, E122, E123, E124, E125, E126,
        };
        static_assert(127 == EnumTraits::internal::ElementCount<Enum>::value);
        static_assert(EnumTraits::VerifyElementCountIfSupported<Enum>(127));
    }

    TEST(EnumTraits, ElementCount128)
    {
        enum class Enum
        {
            E00, E01, E02, E03, E04, E05, E06, E07, E08, E09,
            E10, E11, E12, E13, E14, E15, E16, E17, E18, E19,
            E20, E21, E22, E23, E24, E25, E26, E27, E28, E29,
            E30, E31, E32, E33, E34, E35, E36, E37, E38, E39,
            E40, E41, E42, E43, E44, E45, E46, E47, E48, E49,
            E50, E51, E52, E53, E54, E55, E56, E57, E58, E59,
            E60, E61, E62, E63, E64, E65, E66, E67, E68, E69,
            E70, E71, E72, E73, E74, E75, E76, E77, E78, E79,
            E80, E81, E82, E83, E84, E85, E86, E87, E88, E89,
            E90, E91, E92, E93, E94, E95, E96, E97, E98, E99,
            E100, E101, E102, E103, E104, E105, E106, E107, E108, E109,
            E110, E111, E112, E113, E114, E115, E116, E117, E118, E119,
            E120, E121, E122, E123, E124, E125, E126, E127,
        };
        static_assert(128 == EnumTraits::internal::ElementCount<Enum>::value);
    }
}

#endif
