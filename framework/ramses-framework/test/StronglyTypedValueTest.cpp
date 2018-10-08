//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-framework-api/StronglyTypedValue.h"
#include "stdint.h"

using namespace ramses;

class AClassWithDataAndOperators
{
public:
    explicit AClassWithDataAndOperators(uint32_t data)
        : m_data(data)
    {
    }

    AClassWithDataAndOperators(const AClassWithDataAndOperators& other)
        : m_data(other.m_data)
    {
    }

    AClassWithDataAndOperators& operator=(const AClassWithDataAndOperators& other)
    {
        m_data = other.m_data;
        return *this;
    }

    uint32_t getData()
    {
        return m_data;
    }

    void setData(uint32_t data)
    {
        m_data = data;
    }

    MOCK_CONST_METHOD1(equalsCalled, void(uint32_t));
    bool operator==(const AClassWithDataAndOperators& other) const
    {
        equalsCalled(other.m_data);
        return m_data == other.m_data;
    }

    MOCK_CONST_METHOD1(notEqualsCalled, void(uint32_t));
    bool operator!=(const AClassWithDataAndOperators& other) const
    {
        notEqualsCalled(other.m_data);
        return m_data != other.m_data;
    }

private:
    uint32_t m_data;
};

struct UInt32Tag {};
typedef StronglyTypedValue<uint32_t, UInt32Tag> StronglyTypedUInt32;

struct AClassWithDataAndOperatorsTag {};
typedef StronglyTypedValue<AClassWithDataAndOperators, AClassWithDataAndOperatorsTag> StronglyTypedClass;

TEST(AStronglyTypedValue, CanBeCreatedWithUInt32)
{
    StronglyTypedUInt32 stronglyTypedUInt(12u);
}

TEST(AStronglyTypedValue, CanBeCreatedWithClass)
{
    StronglyTypedClass stronglyTypedClass(AClassWithDataAndOperators(12u));
}

TEST(AStronglyTypedValue, ReturnsCorrectUInt32Value)
{
    StronglyTypedUInt32 stronglyTypedUInt(12u);
    EXPECT_EQ(12u, stronglyTypedUInt.getValue());
}

TEST(AStronglyTypedValue, ReturnsCorrectClassValue)
{
    StronglyTypedClass stronglyTypedClass(AClassWithDataAndOperators(12u));
    EXPECT_EQ(12u, stronglyTypedClass.getValue().getData());
}

TEST(AStronglyTypedValue, ReturnsCorrectUInt32Reference)
{
    StronglyTypedUInt32 stronglyTypedUInt(12u);
    EXPECT_EQ(12u, stronglyTypedUInt.getReference());
}

TEST(AStronglyTypedValue, ReturnsCorrectClassReference)
{
    StronglyTypedClass stronglyTypedClass(AClassWithDataAndOperators(12u));
    EXPECT_EQ(12u, stronglyTypedClass.getReference().getData());
}

TEST(AStronglyTypedValue, ReturnedReferenceChangeEffectsValue_UInt32)
{
    StronglyTypedUInt32 stronglyTypedUInt(12u);
    stronglyTypedUInt.getReference() = 34u;
    EXPECT_EQ(34u, stronglyTypedUInt.getReference());
}

TEST(AStronglyTypedValue, ReturnedReferenceChangeEffectsValue_Class)
{
    StronglyTypedClass stronglyTypedClass(AClassWithDataAndOperators(12u));
    stronglyTypedClass.getReference().setData(34u);
    EXPECT_EQ(34u, stronglyTypedClass.getReference().getData());
}

TEST(AStronglyTypedValue, CallsEqualsOperator)
{
    StronglyTypedClass stronglyTypedClass1(AClassWithDataAndOperators(12u));
    StronglyTypedClass stronglyTypedClass2(AClassWithDataAndOperators(34u));
    EXPECT_CALL(stronglyTypedClass1.getReference(), equalsCalled(34u)).Times(1);
    EXPECT_FALSE(stronglyTypedClass1 == stronglyTypedClass2);
}

TEST(AStronglyTypedValue, CallsNotEqualsOperator)
{
    StronglyTypedClass stronglyTypedClass1(AClassWithDataAndOperators(12u));
    StronglyTypedClass stronglyTypedClass2(AClassWithDataAndOperators(34u));
    EXPECT_CALL(stronglyTypedClass1.getReference(), notEqualsCalled(34u)).Times(1);
    EXPECT_TRUE(stronglyTypedClass1 != stronglyTypedClass2);
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

TEST(AStronglyTypedValue, EqualsToOtherIfSameClassValue)
{
    StronglyTypedClass stronglyTypedClass1(AClassWithDataAndOperators(12u));
    StronglyTypedClass stronglyTypedClass2(AClassWithDataAndOperators(12u));
    EXPECT_CALL(stronglyTypedClass1.getReference(), equalsCalled(12u)).Times(1);
    EXPECT_CALL(stronglyTypedClass1.getReference(), notEqualsCalled(12u)).Times(1);
    EXPECT_TRUE(stronglyTypedClass1 == stronglyTypedClass2);
    EXPECT_FALSE(stronglyTypedClass1 != stronglyTypedClass2);
}

TEST(AStronglyTypedValue, DoesNotEqualToOtherIfDifferentClassValue)
{
    StronglyTypedClass stronglyTypedClass1(AClassWithDataAndOperators(12u));
    StronglyTypedClass stronglyTypedClass2(AClassWithDataAndOperators(34u));
    EXPECT_CALL(stronglyTypedClass1.getReference(), equalsCalled(34u)).Times(1);
    EXPECT_CALL(stronglyTypedClass1.getReference(), notEqualsCalled(34u)).Times(1);
    EXPECT_FALSE(stronglyTypedClass1 == stronglyTypedClass2);
    EXPECT_TRUE(stronglyTypedClass1 != stronglyTypedClass2);
}

TEST(AStronglyTypedValue, CanBeCopiedWithUInt32AsValue)
{
    StronglyTypedUInt32 stronglyTypedUInt1(12u);
    StronglyTypedUInt32 stronglyTypedUInt2(stronglyTypedUInt1);
    EXPECT_EQ(stronglyTypedUInt1, stronglyTypedUInt2);
}

TEST(AStronglyTypedValue, CanBeCopiedWithClassAsValue)
{
    StronglyTypedClass stronglyTypedClass1(AClassWithDataAndOperators(12u));
    StronglyTypedClass stronglyTypedClass2(stronglyTypedClass1);
    EXPECT_EQ(stronglyTypedClass1.getValue().getData(), stronglyTypedClass2.getValue().getData());
}

TEST(AStronglyTypedValue, CanBeCopyAssignedWithUInt32AsValue)
{
    StronglyTypedUInt32 stronglyTypedUInt1(12u);
    StronglyTypedUInt32 stronglyTypedUInt2(34u);
    stronglyTypedUInt2 = stronglyTypedUInt1;
    EXPECT_EQ(stronglyTypedUInt1, stronglyTypedUInt2);
}

TEST(AStronglyTypedValue, CanBeCopyAssignedWithClassAsValue)
{
    StronglyTypedClass stronglyTypedClass1(AClassWithDataAndOperators(12u));
    StronglyTypedClass stronglyTypedClass2(AClassWithDataAndOperators(34u));
    stronglyTypedClass2 = stronglyTypedClass1;
    EXPECT_EQ(stronglyTypedClass1.getValue().getData(), stronglyTypedClass2.getValue().getData());
}
