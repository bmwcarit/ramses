//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTTESTCOMMON_H
#define RAMSES_RAMSESOBJECTTESTCOMMON_H

#include "ClientTestUtils.h"
#include "RamsesObjectTypeUtils.h"
#include "Collections/String.h"

using namespace testing;

namespace ramses
{
    template <typename ObjectType>
    class RamsesObjectTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    };

    TYPED_TEST_SUITE_P(RamsesObjectTest);

    TYPED_TEST_P(RamsesObjectTest, getType)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");
        const ERamsesObjectType type = TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID;
        EXPECT_EQ(type, obj.getType());
        EXPECT_TRUE(obj.isOfType(ERamsesObjectType_RamsesObject));
    }

    TYPED_TEST_P(RamsesObjectTest, isOfTypeForAllDefinedBaseClasses)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");
        ERamsesObjectType type = TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID;

        while (type != ERamsesObjectType_Invalid)
        {
            EXPECT_TRUE(obj.isOfType(type));
            type = RamsesObjectTraits[type].baseClassTypeID;
        }
    }

    TYPED_TEST_P(RamsesObjectTest, getSetName)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        ASSERT_STREQ("object", obj.getName());
        EXPECT_EQ(StatusOK, obj.setName("newName"));
        ASSERT_STREQ("newName", obj.getName());
    }

    TYPED_TEST_P(RamsesObjectTest, convertToTypes)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<TypeParam>(obj));
        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<TypeParam>(constObj));

        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<RamsesObject>(obj));
        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<RamsesObject>(constObj));
    }

    TYPED_TEST_P(RamsesObjectTest, convertToItsClosestBaseClass)
    {
        typedef typename CLASS_OF_RAMSES_OBJECT_TYPE<CLASS_OF_RAMSES_OBJECT_TYPE<TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID>::BaseTypeID>::ClassType BaseClassType;

        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        if (TYPE_ID_OF_RAMSES_OBJECT<BaseClassType>::ID != ERamsesObjectType_Invalid)
        {
            EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<BaseClassType>(obj));
            EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<BaseClassType>(constObj));
        }
    }

    TYPED_TEST_P(RamsesObjectTest, getRamsesObjectFromImpl)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        RamsesObjectImpl& objImpl = obj.impl;
        const RamsesObjectImpl& constObjImpl = constObj.impl;

        EXPECT_EQ(&obj, &objImpl.getRamsesObject());
        EXPECT_EQ(&constObj, &constObjImpl.getRamsesObject());
    }

    TYPED_TEST_P(RamsesObjectTest, validationStringIsNonEmptyAfterCall)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");

        EXPECT_STREQ("", obj.getValidationReport());
        obj.validate();
        EXPECT_STRNE("", obj.getValidationReport());
    }

    TYPED_TEST_P(RamsesObjectTest, validationStringContainsObjectTypeAndName)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");

        EXPECT_NE(StatusOK, obj.impl.addErrorEntry("dummy"));
        obj.validate();
        const ramses_internal::String validationReport = obj.getValidationReport(EValidationSeverity_Info);

        EXPECT_TRUE(validationReport.find(RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj.getType())) >= 0);
        EXPECT_TRUE(validationReport.find(obj.getName()) >= 0);
    }

    TYPED_TEST_P(RamsesObjectTest, validationFailsIfThereWasAnyWrongAPIUsage)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");

        EXPECT_NE(StatusOK, obj.impl.addErrorEntry("dummy error msg"));
        EXPECT_NE(StatusOK, obj.validate());
    }

    TYPED_TEST_P(RamsesObjectTest, failedValidationAfterWrongAPIUsageResetAfterValidatedAgain)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");

        // some objects are not valid initially
        const bool originalStatusOK = (obj.validate() == StatusOK);

        EXPECT_NE(StatusOK, obj.impl.addErrorEntry("dummy error msg"));
        EXPECT_NE(StatusOK, obj.validate());

        // subsequent validation expected to return to original state
        EXPECT_EQ(originalStatusOK, obj.validate() == StatusOK);
    }

    TYPED_TEST_P(RamsesObjectTest, convertToWrongType)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        if (constObj.getType() != ERamsesObjectType_SplineStepBool)
        {
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<SplineStepBool>(obj));
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<SplineStepBool>(constObj));
        }
        else
        {
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<SplineStepInt32>(obj));
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<SplineStepInt32>(constObj));
        }
    }

    REGISTER_TYPED_TEST_SUITE_P(RamsesObjectTest,
                               getType,
                               isOfTypeForAllDefinedBaseClasses,
                               getSetName,
                               convertToTypes,
                               convertToItsClosestBaseClass,
                               convertToWrongType,
                               getRamsesObjectFromImpl,
                               validationStringIsNonEmptyAfterCall,
                               validationStringContainsObjectTypeAndName,
                               validationFailsIfThereWasAnyWrongAPIUsage,
                               failedValidationAfterWrongAPIUsageResetAfterValidatedAgain);
}

#endif
