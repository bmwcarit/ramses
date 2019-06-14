//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/StronglyTypedValue.h"
#include "gmock/gmock.h"
#include <cstdint>
#include <unordered_set>

namespace ramses
{
    namespace
    {
        struct UInt32Tag {};
        typedef StronglyTypedValue<uint32_t, UInt32Tag> StronglyTypedUInt32;

        typedef StronglyTypedValue<void*, struct VoidPtrTag> StronglyTypedPtr;
    }

    TEST(AStronglyTypedValue, CanBeCreatedWithUInt32)
    {
        StronglyTypedUInt32 stronglyTypedUInt(12u);
    }

    TEST(AStronglyTypedValue, ReturnsCorrectUInt32Value)
    {
        StronglyTypedUInt32 stronglyTypedUInt(12u);
        EXPECT_EQ(12u, stronglyTypedUInt.getValue());
    }

    TEST(AStronglyTypedValue, ReturnsCorrectUInt32Reference)
    {
        StronglyTypedUInt32 stronglyTypedUInt(12u);
        EXPECT_EQ(12u, stronglyTypedUInt.getReference());
    }

    TEST(AStronglyTypedValue, ReturnedReferenceChangeEffectsValue_UInt32)
    {
        StronglyTypedUInt32 stronglyTypedUInt(12u);
        stronglyTypedUInt.getReference() = 34u;
        EXPECT_EQ(34u, stronglyTypedUInt.getReference());
    }

    TEST(AStronglyTypedValue, EqualsToOtherIfSameUInt32Value)
    {
        StronglyTypedUInt32 stronglyTypedUInt1(12u);
        StronglyTypedUInt32 stronglyTypedUInt2(12u);
        EXPECT_TRUE(stronglyTypedUInt1 == stronglyTypedUInt2);
        EXPECT_FALSE(stronglyTypedUInt1 != stronglyTypedUInt2);
    }

    TEST(AStronglyTypedValue, DoesNotEqualToOtherIfDifferentUInt32Value)
    {
        StronglyTypedUInt32 stronglyTypedUInt1(12u);
        StronglyTypedUInt32 stronglyTypedUInt2(34u);
        EXPECT_FALSE(stronglyTypedUInt1 == stronglyTypedUInt2);
        EXPECT_TRUE(stronglyTypedUInt1 != stronglyTypedUInt2);
    }

    TEST(AStronglyTypedValue, CanBeCopiedWithUInt32AsValue)
    {
        StronglyTypedUInt32 stronglyTypedUInt1(12u);
        StronglyTypedUInt32 stronglyTypedUInt2(stronglyTypedUInt1);
        EXPECT_EQ(stronglyTypedUInt1, stronglyTypedUInt2);
    }

    TEST(AStronglyTypedValue, CanBeCopyAssignedWithUInt32AsValue)
    {
        StronglyTypedUInt32 stronglyTypedUInt1(12u);
        StronglyTypedUInt32 stronglyTypedUInt2(34u);
        stronglyTypedUInt2 = stronglyTypedUInt1;
        EXPECT_EQ(stronglyTypedUInt1, stronglyTypedUInt2);
    }

    TEST(AStronglyTypedValue, canBeUsedWithPointer)
    {
        int i = 1;
        int j = 2;
        StronglyTypedPtr p1(&i);
        StronglyTypedPtr p2(&i);
        StronglyTypedPtr p3(&j);

        EXPECT_TRUE(p1 == p2);
        EXPECT_TRUE(p1 != p3);
        EXPECT_FALSE(p1 == p3);
        EXPECT_TRUE(p1.getValue() == &i);

        p3 = p1;
        EXPECT_TRUE(p1 == p3);

        StronglyTypedPtr p4(nullptr);
        EXPECT_TRUE(p4.getValue() == nullptr);

        p4.getReference() = &j;
        EXPECT_TRUE(p4.getValue() == &j);
    }

    TEST(AStronglyTypedValue, canBeUsedInStdHashContainerWithIntegral)
    {
        StronglyTypedUInt32 i1(1);
        StronglyTypedUInt32 i2(2);
        StronglyTypedUInt32 i3(3);

        std::unordered_set<StronglyTypedUInt32> si;
        si.insert(i1);
        si.insert(i2);
        EXPECT_TRUE(si.find(i1) != si.end());
        EXPECT_TRUE(si.find(i2) != si.end());
        EXPECT_FALSE(si.find(i3) != si.end());
    }

    TEST(AStronglyTypedValue, canBeUsedInStdHashContainerWithPointer)
    {
        int i = 1;
        int j = 2;
        StronglyTypedPtr p1(&i);
        StronglyTypedPtr p2(nullptr);
        StronglyTypedPtr p3(&j);

        std::unordered_set<StronglyTypedPtr> sp;
        sp.insert(p1);
        sp.insert(p2);
        EXPECT_TRUE(sp.find(p1) != sp.end());
        EXPECT_TRUE(sp.find(p2) != sp.end());
        EXPECT_FALSE(sp.find(p3) != sp.end());
    }
}
