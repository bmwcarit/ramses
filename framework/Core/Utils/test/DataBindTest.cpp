//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DataBindTest.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix44f.h"

using namespace testing;

namespace ramses_internal
{
    DATA_BIND_DEFINE_BEGIN(DataBindTestBool)
        DATA_BIND_DEFINE(0, DataBindTestBool, DataBindTestBool::getVal0, DataBindTestBool::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Boolean, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestBool, DataBindTestBool::getVal1, DataBindTestBool::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Boolean, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestBool, DataBindTestBool::getVal2, DataBindTestBool::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Boolean, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestInt32)
        DATA_BIND_DEFINE(0, DataBindTestInt32, DataBindTestInt32::getVal0, DataBindTestInt32::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Int32, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestInt32, DataBindTestInt32::getVal1, DataBindTestInt32::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Int32, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestInt32, DataBindTestInt32::getVal2, DataBindTestInt32::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Int32, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestFloat)
        DATA_BIND_DEFINE(0, DataBindTestFloat, DataBindTestFloat::getVal0, DataBindTestFloat::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Float, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestFloat, DataBindTestFloat::getVal1, DataBindTestFloat::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Float, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestFloat, DataBindTestFloat::getVal2, DataBindTestFloat::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Float, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestVector2)
        DATA_BIND_DEFINE(0, DataBindTestVector2, DataBindTestVector2::getVal0, DataBindTestVector2::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Vector2f, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestVector2, DataBindTestVector2::getVal1, DataBindTestVector2::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Vector2f, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestVector2, DataBindTestVector2::getVal2, DataBindTestVector2::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Vector2f, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestVector3)
        DATA_BIND_DEFINE(0, DataBindTestVector3, DataBindTestVector3::getVal0, DataBindTestVector3::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Vector3f, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestVector3, DataBindTestVector3::getVal1, DataBindTestVector3::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Vector3f, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestVector3, DataBindTestVector3::getVal2, DataBindTestVector3::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Vector3f, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestVector4)
        DATA_BIND_DEFINE(0, DataBindTestVector4, DataBindTestVector4::getVal0, DataBindTestVector4::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Vector4f, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestVector4, DataBindTestVector4::getVal1, DataBindTestVector4::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Vector4f, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestVector4, DataBindTestVector4::getVal2, DataBindTestVector4::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Vector4f, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestMatrix44f)
        DATA_BIND_DEFINE(0, DataBindTestMatrix44f, DataBindTestMatrix44f::getVal0, DataBindTestMatrix44f::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Matrix44f, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestMatrix44f, DataBindTestMatrix44f::getVal1, DataBindTestMatrix44f::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Matrix44f, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestMatrix44f, DataBindTestMatrix44f::getVal2, DataBindTestMatrix44f::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Matrix44f, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestVector2i)
        DATA_BIND_DEFINE(0, DataBindTestVector2i, DataBindTestVector2i::getVal0, DataBindTestVector2i::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Vector2i, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestVector2i, DataBindTestVector2i::getVal1, DataBindTestVector2i::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Vector2i, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestVector2i, DataBindTestVector2i::getVal2, DataBindTestVector2i::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Vector2i, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestVector3i)
        DATA_BIND_DEFINE(0, DataBindTestVector3i, DataBindTestVector3i::getVal0, DataBindTestVector3i::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Vector3i, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestVector3i, DataBindTestVector3i::getVal1, DataBindTestVector3i::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Vector3i, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestVector3i, DataBindTestVector3i::getVal2, DataBindTestVector3i::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Vector3i, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(DataBindTestVector4i)
        DATA_BIND_DEFINE(0, DataBindTestVector4i, DataBindTestVector4i::getVal0, DataBindTestVector4i::setVal0, DataBindTestBool::checkProperty0, EDataTypeID_Vector4i, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, DataBindTestVector4i, DataBindTestVector4i::getVal1, DataBindTestVector4i::setVal1, DataBindTestBool::checkProperty1, EDataTypeID_Vector4i, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, DataBindTestVector4i, DataBindTestVector4i::getVal2, DataBindTestVector4i::setVal2, DataBindTestBool::checkProperty2, EDataTypeID_Vector4i, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    TEST_F(DataBindTest, TypesTestNoHandle)
    {
        for (UInt32 handle = 0; handle < DataBindTestContainer<UInt>::NumHandles; ++handle)
        {
            DataBindTestHelper::testDataBindingTypes0(true, false);
            DataBindTestHelper::testDataBindingTypes0(-1, 999);
            DataBindTestHelper::testDataBindingTypes0(1e-6f, 999.999f);
            DataBindTestHelper::testDataBindingTypes0(Vector2(1, 2), Vector2(-1, -2));
            DataBindTestHelper::testDataBindingTypes0(Vector3(1, 2, 3), Vector3(-1, -2, -3));
            DataBindTestHelper::testDataBindingTypes0(Vector4(1, 2, 3, 4), Vector4(-1, -2, -3, -4));
            DataBindTestHelper::testDataBindingTypes0(Vector2i(1, 2), Vector2i(-1, -2));
            DataBindTestHelper::testDataBindingTypes0(Vector3i(1, 2, 3), Vector3i(-1, -2, -3));
            DataBindTestHelper::testDataBindingTypes0(Vector4i(1, 2, 3, 4), Vector4i(-1, -2, -3, -4));
            DataBindTestHelper::testDataBindingTypes0(Matrix44f(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f), Matrix44f(-1.f, -2.f, -3.f, -4.f, -5.f, -6.f, -7.f, -8.f, -9.f, -10.f, -11.f, -12.f, -13.f, -14.f, -15.f, -16.f));
        }
    }

    TEST_F(DataBindTest, TypesTestSingleHandle)
    {
        for (UInt32 handle = 0; handle < DataBindTestContainer<UInt>::NumHandles; ++handle)
        {
            DataBindTestHelper::testDataBindingTypes1(handle, true, false);
            DataBindTestHelper::testDataBindingTypes1(handle, -1, 999);
            DataBindTestHelper::testDataBindingTypes1(handle, 1e-6f, 999.999f);
            DataBindTestHelper::testDataBindingTypes1(handle, Vector2(1, 2), Vector2(-1, -2));
            DataBindTestHelper::testDataBindingTypes1(handle, Vector3(1, 2, 3), Vector3(-1, -2, -3));
            DataBindTestHelper::testDataBindingTypes1(handle, Vector4(1, 2, 3, 4), Vector4(-1, -2, -3, -4));
            DataBindTestHelper::testDataBindingTypes1(handle, Vector2i(1, 2), Vector2i(-1, -2));
            DataBindTestHelper::testDataBindingTypes1(handle, Vector3i(1, 2, 3), Vector3i(-1, -2, -3));
            DataBindTestHelper::testDataBindingTypes1(handle, Vector4i(1, 2, 3, 4), Vector4i(-1, -2, -3, -4));
            DataBindTestHelper::testDataBindingTypes1(handle, Matrix44f(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f), Matrix44f(-1.f, -2.f, -3.f, -4.f, -5.f, -6.f, -7.f, -8.f, -9.f, -10.f, -11.f, -12.f, -13.f, -14.f, -15.f, -16.f));
        }
    }

    TEST_F(DataBindTest, TypesTestTwoHandles)
    {
        for (UInt32 handle = 0; handle < DataBindTestContainer<UInt>::NumHandles; ++handle)
        {
            for (UInt32 handle2 = 0; handle2 < DataBindTestContainer<UInt>::NumHandles; ++handle2)
            {
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, true, false);
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, -1, 999);
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, 1e-6f, 999.999f);
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector2(1, 2), Vector2(-1, -2));
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector3(1, 2, 3), Vector3(-1, -2, -3));
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector4(1, 2, 3, 4), Vector4(-1, -2, -3, -4));
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector2i(1, 2), Vector2i(-1, -2));
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector3i(1, 2, 3), Vector3i(-1, -2, -3));
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector4i(1, 2, 3, 4), Vector4i(-1, -2, -3, -4));
                DataBindTestHelper::testDataBindingTypes2(handle, handle2, Matrix44f(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f), Matrix44f(-1.f, -2.f, -3.f, -4.f, -5.f, -6.f, -7.f, -8.f, -9.f, -10.f, -11.f, -12.f, -13.f, -14.f, -15.f, -16.f));
            }
        }
    }

    TEST_F(DataBindTest, GettersNoHandle)
    {
        DataBindTestFloat container;
        DataBind<DataBindTestFloat, Float> dataBind(container, EDataBindAccessorType_Handles_None);
        EXPECT_EQ(static_cast<TDataBindID>(EDataBindAccessorType_Handles_None), dataBind.getBindID());
    }

    TEST_F(DataBindTest, Getters1Handle)
    {
        DataBindTestFloat container;
        DataBind<DataBindTestFloat, Float, UInt32> dataBind(container, 11u, EDataBindAccessorType_Handles_1);
        EXPECT_EQ(11u, dataBind.getHandle());
        EXPECT_EQ(static_cast<TDataBindID>(EDataBindAccessorType_Handles_1), dataBind.getBindID());
    }

    TEST_F(DataBindTest, Getters2Handles)
    {
        DataBindTestFloat container;
        DataBind<DataBindTestFloat, Float, UInt32, UInt32> dataBind(container, 11u, 999u, EDataBindAccessorType_Handles_2);
        EXPECT_EQ(11u, dataBind.getHandle());
        EXPECT_EQ(999u, dataBind.getHandle2());
        EXPECT_EQ(static_cast<TDataBindID>(EDataBindAccessorType_Handles_2), dataBind.getBindID());
    }

    TEST_F(DataBindTest, canCheckPropertyValidity1Handle)
    {
        DataBindTestFloat container;
        DataBind<DataBindTestFloat, Float, UInt32> dataBind(container, DataBindTestFloat::NumHandles, EDataBindAccessorType_Handles_1);
        EXPECT_FALSE(dataBind.isPropertyValid());
    }

    TEST_F(DataBindTest, canCheckPropertyValidity2Handles)
    {
        DataBindTestFloat container;
        DataBind<DataBindTestFloat, Float, UInt32, UInt32> dataBind(container, DataBindTestFloat::NumHandles, 1u, EDataBindAccessorType_Handles_2);
        EXPECT_FALSE(dataBind.isPropertyValid());
    }
}
