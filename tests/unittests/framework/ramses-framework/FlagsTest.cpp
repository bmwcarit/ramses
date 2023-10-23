//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/framework/Flags.h"
#include "gmock/gmock.h"

namespace ramses
{
    namespace internal
    {
        enum class ETestFlag : uint32_t;
    }
    template <> struct is_flag<internal::ETestFlag> : std::true_type {};
}

namespace ramses::internal
{
    enum class ETestFlag : uint32_t
    {
        None = 0, Foo = 1, Bar = 2, Baz = 4, All = Foo|Bar|Baz
    };

    enum EUnrelated
    {
        EUnrelated_A = 1, EUnrelated_B = 2, EUnrelated_C = 4
    };

    using TestFlags = Flags<ETestFlag>;

    TEST(AFlags, DefaultConstruct)
    {
        TestFlags flags;

        EXPECT_FALSE(flags.isSet(ETestFlag::Foo));
        EXPECT_FALSE(flags.isSet(ETestFlag::Bar));
        EXPECT_FALSE(flags.isSet(ETestFlag::Baz));
        EXPECT_FALSE(flags.isSet(ETestFlag::All));
        EXPECT_EQ(0u, flags.value());
    }

    TEST(AFlags, ConstructFromEnum)
    {
        TestFlags flags = ETestFlag::Foo;

        EXPECT_TRUE(flags.isSet(ETestFlag::Foo));
        EXPECT_FALSE(flags.isSet(ETestFlag::Bar));
        EXPECT_FALSE(flags.isSet(ETestFlag::Baz));
        EXPECT_FALSE(flags.isSet(ETestFlag::Baz | ETestFlag::Foo));
        EXPECT_FALSE(flags.isSet(ETestFlag::All));
        EXPECT_EQ(1u, flags.value());
    }

    TEST(AFlags, ConstructFromEnum2)
    {
        TestFlags flags = ETestFlag::Foo | ETestFlag::Bar;

        EXPECT_TRUE(flags.isSet(ETestFlag::None));
        EXPECT_TRUE(flags.isSet(ETestFlag::Foo));
        EXPECT_TRUE(flags.isSet(ETestFlag::Bar));
        EXPECT_TRUE(flags.isSet(ETestFlag::Bar | ETestFlag::Foo));
        EXPECT_FALSE(flags.isSet(ETestFlag::Baz));
        EXPECT_FALSE(flags.isSet(ETestFlag::All));
        EXPECT_EQ(3u, flags.value());
    }

    TEST(AFlags, ConstructFromEnum3)
    {
        TestFlags flags = ETestFlag::Foo | ETestFlag::Bar | ETestFlag::Baz;

        EXPECT_TRUE(flags.isSet(ETestFlag::None));
        EXPECT_TRUE(flags.isSet(ETestFlag::Foo));
        EXPECT_TRUE(flags.isSet(ETestFlag::Bar));
        EXPECT_TRUE(flags.isSet(ETestFlag::Baz));
        EXPECT_TRUE(flags.isSet(ETestFlag::Baz | ETestFlag::Foo | ETestFlag::Bar));
        EXPECT_TRUE(flags.isSet(ETestFlag::All));
        EXPECT_EQ(7u, flags.value());
    }

    TEST(AFlags, operators)
    {
        EXPECT_TRUE(ETestFlag::Foo == TestFlags(1));
        EXPECT_TRUE(ETestFlag::Foo != TestFlags(2));
        EXPECT_EQ(TestFlags(3), ETestFlag::Foo | ETestFlag::Bar);
        EXPECT_EQ(TestFlags(3), ETestFlag::Foo | TestFlags(2));
    }

    TEST(AFlags, setFlag)
    {
        TestFlags flags;
        flags.setFlag(ETestFlag::Foo, true);
        EXPECT_EQ(TestFlags(ETestFlag::Foo), flags);
        flags.setFlag(ETestFlag::Bar, true);
        EXPECT_EQ(TestFlags(ETestFlag::Foo | ETestFlag::Bar), flags);
        flags.setFlag(ETestFlag::Foo, false);
        EXPECT_EQ(TestFlags(ETestFlag::Bar), flags);
        flags.setFlag(ETestFlag::Bar, false);
        EXPECT_EQ(TestFlags(), flags);
    }

    TEST(AFlags, UnrelatedEnumDoesNotMatchOperators)
    {
        // verifies that unrelated enums do not instantiate global flag operators:
        // Flags<EUnrelated> operator|(EUnrelated, EUnrelated)
        const auto unrelated = EUnrelated(EUnrelated_A | EUnrelated_B);
        EXPECT_EQ(unrelated, 3);
        EXPECT_EQ(EUnrelated_A, EUnrelated_A);
        EXPECT_NE(EUnrelated_B, EUnrelated_A);
    }

    TEST(AFlags, ConvertFromInt)
    {
        TestFlags flags(1);
        EXPECT_EQ(ETestFlag::Foo, flags);
    }

    // enforce performance guarantees
    static_assert(std::is_trivially_copyable<TestFlags>::value, "expectation failed");
    static_assert(std::is_default_constructible<TestFlags>::value, "expectation failed");
    static_assert(std::is_trivially_destructible<TestFlags>::value, "expectation failed");

    static_assert((ETestFlag::Foo | ETestFlag::Bar).value() == 3);
    static_assert((ETestFlag::Foo | ETestFlag::Bar).isSet(ETestFlag::Foo));
    static_assert(!(ETestFlag::Baz | ETestFlag::Bar).isSet(ETestFlag::Foo));
    static_assert(sizeof(TestFlags) == sizeof(ETestFlag));
    static_assert(sizeof(TestFlags) == sizeof(std::underlying_type_t<ETestFlag>));
}
