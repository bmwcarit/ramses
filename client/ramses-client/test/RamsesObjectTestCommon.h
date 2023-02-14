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

namespace ramses
{
    template <typename ObjectType>
    class RamsesObjectTest : public LocalTestClientWithScene, public testing::Test
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
        using BaseClassType = typename CLASS_OF_RAMSES_OBJECT_TYPE<CLASS_OF_RAMSES_OBJECT_TYPE<TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID>::BaseTypeID>::ClassType;

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

        EXPECT_THAT(validationReport.stdRef(), ::testing::HasSubstr(RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj.getType())));
        EXPECT_THAT(validationReport.stdRef(), ::testing::HasSubstr(obj.getName()));
    }

    TYPED_TEST_P(RamsesObjectTest, validationIgnoresWrongAPIUsage)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");

        // simulate some API usage error
        EXPECT_NE(StatusOK, obj.impl.addErrorEntry("dummy error msg"));

        obj.validate();
        EXPECT_THAT(obj.getValidationReport(EValidationSeverity::EValidationSeverity_Warning), ::testing::Not(::testing::HasSubstr("dummy error msg")));

        // will not appear in validation of scene either
        this->m_scene.validate();
        EXPECT_THAT(this->m_scene.getValidationReport(EValidationSeverity::EValidationSeverity_Warning), ::testing::Not(::testing::HasSubstr("dummy error msg")));
    }

    TYPED_TEST_P(RamsesObjectTest, convertToWrongType)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        if (constObj.getType() != ERamsesObjectType_PerspectiveCamera)
        {
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<PerspectiveCamera>(obj));
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<PerspectiveCamera>(constObj));
        }
        else
        {
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<OrthographicCamera>(obj));
            EXPECT_TRUE(nullptr == RamsesUtils::TryConvert<OrthographicCamera>(constObj));
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
                               validationIgnoresWrongAPIUsage);
}

#endif
