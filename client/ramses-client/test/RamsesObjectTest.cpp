//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/Spline.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimatedSetter.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepInt32.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/SplineStepVector2f.h"
#include "ramses-client-api/SplineStepVector3f.h"
#include "ramses-client-api/SplineStepVector4f.h"
#include "ramses-client-api/SplineStepVector2i.h"
#include "ramses-client-api/SplineStepVector3i.h"
#include "ramses-client-api/SplineStepVector4i.h"
#include "ramses-client-api/SplineLinearInt32.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearVector2f.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineLinearVector2i.h"
#include "ramses-client-api/SplineLinearVector3i.h"
#include "ramses-client-api/SplineLinearVector4i.h"
#include "ramses-client-api/SplineBezierInt32.h"
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/SplineBezierVector2f.h"
#include "ramses-client-api/SplineBezierVector3f.h"
#include "ramses-client-api/SplineBezierVector4f.h"
#include "ramses-client-api/SplineBezierVector2i.h"
#include "ramses-client-api/SplineBezierVector3i.h"
#include "ramses-client-api/SplineBezierVector4i.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/DataFloat.h"
#include "ramses-client-api/DataVector2f.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/DataMatrix22f.h"
#include "ramses-client-api/DataMatrix33f.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/DataVector2i.h"
#include "ramses-client-api/DataVector3i.h"
#include "ramses-client-api/DataVector4i.h"
#include "ramses-client-api/StreamTexture.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-utils.h"

#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"
#include "RamsesObjectTypeUtils.h"
#include "Collections/String.h"

using namespace testing;

namespace ramses
{
    template <typename ObjectType>
    class RamsesObjectTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
    RamsesObjectTest() : LocalTestClientWithSceneAndAnimationSystem()
    {
    }
    };

    TYPED_TEST_CASE_P(RamsesObjectTest);

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

    TYPED_TEST_P(RamsesObjectTest, getName)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");
        ASSERT_STREQ("object", obj.getName());
    }

    TYPED_TEST_P(RamsesObjectTest, setName)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        EXPECT_EQ(StatusOK, obj.setName("newName"));
        ASSERT_STREQ("newName", obj.getName());
    }

    TYPED_TEST_P(RamsesObjectTest, convertToConcreteType)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        EXPECT_TRUE(NULL != RamsesUtils::TryConvert<TypeParam>(obj));
        EXPECT_TRUE(NULL != RamsesUtils::TryConvert<TypeParam>(constObj));
    }

    TYPED_TEST_P(RamsesObjectTest, convertToRamsesObject)
    {
        TypeParam& obj = this->template createObject<TypeParam>("object");
        const TypeParam& constObj = obj;

        EXPECT_TRUE(NULL != RamsesUtils::TryConvert<RamsesObject>(obj));
        EXPECT_TRUE(NULL != RamsesUtils::TryConvert<RamsesObject>(constObj));
    }

    TYPED_TEST_P(RamsesObjectTest, convertToItsClosestBaseClass)
    {
        typedef typename CLASS_OF_RAMSES_OBJECT_TYPE<CLASS_OF_RAMSES_OBJECT_TYPE<TYPE_ID_OF_RAMSES_OBJECT<TypeParam>::ID>::BaseTypeID>::ClassType BaseClassType;

        RamsesObject& obj = this->template createObject<TypeParam>("object");
        const RamsesObject& constObj = obj;

        if (TYPE_ID_OF_RAMSES_OBJECT<BaseClassType>::ID != ERamsesObjectType_Invalid)
        {
            EXPECT_TRUE(NULL != RamsesUtils::TryConvert<BaseClassType>(obj));
            EXPECT_TRUE(NULL != RamsesUtils::TryConvert<BaseClassType>(constObj));
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

    TYPED_TEST_P(RamsesObjectTest, validationStringIsEmptyIfNotCalledValidate)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");

        EXPECT_STREQ("", obj.getValidationReport());
    }

    TYPED_TEST_P(RamsesObjectTest, validationStringIsNotNULL)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");

        obj.validate();
        EXPECT_TRUE(NULL != obj.getValidationReport());
    }

    TYPED_TEST_P(RamsesObjectTest, validationStringContainsObjectTypeAndName)
    {
        const RamsesObject& obj = this->template createObject<TypeParam>("object");

        obj.impl.addErrorEntry("dummy");
        obj.validate();
        const ramses_internal::String validationReport = obj.getValidationReport(EValidationSeverity_Info);

        EXPECT_TRUE(validationReport.find(RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj.getType())) >= 0);
        EXPECT_TRUE(validationReport.find(obj.getName()) >= 0);
    }

    TYPED_TEST_P(RamsesObjectTest, validationFailsIfThereWasAnyWrongAPIUsage)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");

        obj.impl.addErrorEntry("dummy error msg");
        EXPECT_NE(StatusOK, obj.validate());
    }

    TYPED_TEST_P(RamsesObjectTest, failedValidationAfterWrongAPIUsageResetAfterValidatedAgain)
    {
        RamsesObject& obj = this->template createObject<TypeParam>("object");

        // some objects are not valid initially
        const bool originalStatusOK = (obj.validate() == StatusOK);

        obj.impl.addErrorEntry("dummy error msg");
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
            EXPECT_TRUE(NULL == RamsesUtils::TryConvert<SplineStepBool>(obj));
            EXPECT_TRUE(NULL == RamsesUtils::TryConvert<SplineStepBool>(constObj));
        }
        else
        {
            EXPECT_TRUE(NULL == RamsesUtils::TryConvert<SplineStepInt32>(obj));
            EXPECT_TRUE(NULL == RamsesUtils::TryConvert<SplineStepInt32>(constObj));
        }
    }

    REGISTER_TYPED_TEST_CASE_P(
        RamsesObjectTest,
        getType,
        isOfTypeForAllDefinedBaseClasses,
        getName,
        setName,
        convertToConcreteType,
        convertToRamsesObject,
        convertToItsClosestBaseClass,
        convertToWrongType,
        getRamsesObjectFromImpl,
        validationStringIsEmptyIfNotCalledValidate,
        validationStringIsNotNULL,
        validationStringContainsObjectTypeAndName,
        validationFailsIfThereWasAnyWrongAPIUsage,
        failedValidationAfterWrongAPIUsageResetAfterValidatedAgain);

    INSTANTIATE_TYPED_TEST_CASE_P(RamsesObjectTest1, RamsesObjectTest, RamsesObjectTypes1);
    INSTANTIATE_TYPED_TEST_CASE_P(RamsesObjectTest2, RamsesObjectTest, RamsesObjectTypes2);
}
