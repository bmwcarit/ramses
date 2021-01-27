//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimationDataBindTest.h"
// In order to explicitly instantiate AnimationProcessDataDispatch for custom types defined here
#include "../src/AnimationProcessDataDispatch.cpp" // NOLINT(bugprone-suspicious-include) will likely cause ODR issues but not worth/easy to fix
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "AnimationTestUtils.h"
#include "SceneAPI/Handles.h"

using namespace testing;

namespace ramses_internal
{
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestBool,     bool     >(const AnimationDataBind< AnimationDataBindTestBool,     bool   >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestInt32,    Int32    >(const AnimationDataBind< AnimationDataBindTestInt32,    Int32  >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestFloat,    Float    >(const AnimationDataBind< AnimationDataBindTestFloat,    Float  >&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector2,  Vector2  >(const AnimationDataBind< AnimationDataBindTestVector2,  Vector2>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector3,  Vector3  >(const AnimationDataBind< AnimationDataBindTestVector3,  Vector3>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector4,  Vector4  >(const AnimationDataBind< AnimationDataBindTestVector4,  Vector4>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector2i, Vector2i >(const AnimationDataBind< AnimationDataBindTestVector2i, Vector2i>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector3i, Vector3i >(const AnimationDataBind< AnimationDataBindTestVector3i, Vector3i>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector4i, Vector4i >(const AnimationDataBind< AnimationDataBindTestVector4i, Vector4i>&) const;

    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestBool,     bool,     MemoryHandle >(const AnimationDataBind< AnimationDataBindTestBool,     bool,     MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestInt32,    Int32,    MemoryHandle >(const AnimationDataBind< AnimationDataBindTestInt32,    Int32,    MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestFloat,    Float,    MemoryHandle >(const AnimationDataBind< AnimationDataBindTestFloat,    Float,    MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector2,  Vector2,  MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector2,  Vector2,  MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector3,  Vector3,  MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector3,  Vector3,  MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector4,  Vector4,  MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector4,  Vector4,  MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector2i, Vector2i, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector2i, Vector2i, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector3i, Vector3i, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector3i, Vector3i, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector4i, Vector4i, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector4i, Vector4i, MemoryHandle>&) const;

    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestBool,     bool,     MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestBool,     bool,     MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestInt32,    Int32,    MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestInt32,    Int32,    MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestFloat,    Float,    MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestFloat,    Float,    MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector2,  Vector2,  MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector2,  Vector2,  MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector3,  Vector3,  MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector3,  Vector3,  MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector4,  Vector4,  MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector4,  Vector4,  MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector2i, Vector2i, MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector2i, Vector2i, MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector3i, Vector3i, MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector3i, Vector3i, MemoryHandle, MemoryHandle>&) const;
    template void AnimationProcessDataDispatch::dispatchDataBind< AnimationDataBindTestVector4i, Vector4i, MemoryHandle, MemoryHandle >(const AnimationDataBind< AnimationDataBindTestVector4i, Vector4i, MemoryHandle, MemoryHandle>&) const;

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestBool)
        DATA_BIND_DEFINE(0, AnimationDataBindTestBool, AnimationDataBindTestBool::getVal0, AnimationDataBindTestBool::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Boolean, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestBool, AnimationDataBindTestBool::getVal1, AnimationDataBindTestBool::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Boolean, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestBool, AnimationDataBindTestBool::getVal2, AnimationDataBindTestBool::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Boolean, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestInt32)
        DATA_BIND_DEFINE(0, AnimationDataBindTestInt32, AnimationDataBindTestInt32::getVal0, AnimationDataBindTestInt32::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Int32, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestInt32, AnimationDataBindTestInt32::getVal1, AnimationDataBindTestInt32::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Int32, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestInt32, AnimationDataBindTestInt32::getVal2, AnimationDataBindTestInt32::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Int32, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestFloat)
        DATA_BIND_DEFINE(0, AnimationDataBindTestFloat, AnimationDataBindTestFloat::getVal0, AnimationDataBindTestFloat::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Float, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestFloat, AnimationDataBindTestFloat::getVal1, AnimationDataBindTestFloat::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Float, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestFloat, AnimationDataBindTestFloat::getVal2, AnimationDataBindTestFloat::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Float, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestVector2)
        DATA_BIND_DEFINE(0, AnimationDataBindTestVector2, AnimationDataBindTestVector2::getVal0, AnimationDataBindTestVector2::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Vector2f, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestVector2, AnimationDataBindTestVector2::getVal1, AnimationDataBindTestVector2::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Vector2f, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestVector2, AnimationDataBindTestVector2::getVal2, AnimationDataBindTestVector2::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Vector2f, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestVector3)
        DATA_BIND_DEFINE(0, AnimationDataBindTestVector3, AnimationDataBindTestVector3::getVal0, AnimationDataBindTestVector3::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Vector3f, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestVector3, AnimationDataBindTestVector3::getVal1, AnimationDataBindTestVector3::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Vector3f, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestVector3, AnimationDataBindTestVector3::getVal2, AnimationDataBindTestVector3::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Vector3f, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestVector4)
        DATA_BIND_DEFINE(0, AnimationDataBindTestVector4, AnimationDataBindTestVector4::getVal0, AnimationDataBindTestVector4::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Vector4f, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestVector4, AnimationDataBindTestVector4::getVal1, AnimationDataBindTestVector4::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Vector4f, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestVector4, AnimationDataBindTestVector4::getVal2, AnimationDataBindTestVector4::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Vector4f, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestVector2i)
        DATA_BIND_DEFINE(0, AnimationDataBindTestVector2i, AnimationDataBindTestVector2i::getVal0, AnimationDataBindTestVector2i::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Vector2i, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestVector2i, AnimationDataBindTestVector2i::getVal1, AnimationDataBindTestVector2i::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Vector2i, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestVector2i, AnimationDataBindTestVector2i::getVal2, AnimationDataBindTestVector2i::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Vector2i, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestVector3i)
        DATA_BIND_DEFINE(0, AnimationDataBindTestVector3i, AnimationDataBindTestVector3i::getVal0, AnimationDataBindTestVector3i::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Vector3i, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestVector3i, AnimationDataBindTestVector3i::getVal1, AnimationDataBindTestVector3i::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Vector3i, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestVector3i, AnimationDataBindTestVector3i::getVal2, AnimationDataBindTestVector3i::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Vector3i, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    DATA_BIND_DEFINE_BEGIN(AnimationDataBindTestVector4i)
        DATA_BIND_DEFINE(0, AnimationDataBindTestVector4i, AnimationDataBindTestVector4i::getVal0, AnimationDataBindTestVector4i::setVal0, AnimationDataBindTestBool::isPropertyValid0, EDataTypeID_Vector4i, EDataBindAccessorType_Handles_None)
        DATA_BIND_DEFINE(1, AnimationDataBindTestVector4i, AnimationDataBindTestVector4i::getVal1, AnimationDataBindTestVector4i::setVal1, AnimationDataBindTestBool::isPropertyValid1, EDataTypeID_Vector4i, EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE(2, AnimationDataBindTestVector4i, AnimationDataBindTestVector4i::getVal2, AnimationDataBindTestVector4i::setVal2, AnimationDataBindTestBool::isPropertyValid2, EDataTypeID_Vector4i, EDataBindAccessorType_Handles_2)
    DATA_BIND_DEFINE_END()

    TEST_F(AnimationDataBindTest, TypesTestNoHandle)
    {
        for (MemoryHandle handle = 0; handle < AnimationDataBindTestContainer<UInt>::NumHandles; ++handle)
        {
            AnimationDataBindTestHelper::testDataBindingTypes0(true, false);
            AnimationDataBindTestHelper::testDataBindingTypes0(-1, 999);
            AnimationDataBindTestHelper::testDataBindingTypes0(1e-6f, 999.999f);
            AnimationDataBindTestHelper::testDataBindingTypes0(Vector2(1, 2), Vector2(-1, -2));
            AnimationDataBindTestHelper::testDataBindingTypes0(Vector3(1, 2, 3), Vector3(-1, -2, -3));
            AnimationDataBindTestHelper::testDataBindingTypes0(Vector4(1, 2, 3, 4), Vector4(-1, -2, -3, -4));
            AnimationDataBindTestHelper::testDataBindingTypes0(Vector2i(1, 2), Vector2i(-1, -2));
            AnimationDataBindTestHelper::testDataBindingTypes0(Vector3i(1, 2, 3), Vector3i(-1, -2, -3));
            AnimationDataBindTestHelper::testDataBindingTypes0(Vector4i(1, 2, 3, 4), Vector4i(-1, -2, -3, -4));
        }
    }

    TEST_F(AnimationDataBindTest, TypesTestSingleHandle)
    {
        for (MemoryHandle handle = 0; handle < AnimationDataBindTestContainer<UInt>::NumHandles; ++handle)
        {
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, true, false);
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, -1, 999);
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, 1e-6f, 999.999f);
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, Vector2(1, 2), Vector2(-1, -2));
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, Vector3(1, 2, 3), Vector3(-1, -2, -3));
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, Vector4(1, 2, 3, 4), Vector4(-1, -2, -3, -4));
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, Vector2i(1, 2), Vector2i(-1, -2));
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, Vector3i(1, 2, 3), Vector3i(-1, -2, -3));
            AnimationDataBindTestHelper::testDataBindingTypes1(handle, Vector4i(1, 2, 3, 4), Vector4i(-1, -2, -3, -4));
        }
    }

    TEST_F(AnimationDataBindTest, TypesTestTwoHandles)
    {
        for (MemoryHandle handle = 0; handle < AnimationDataBindTestContainer<UInt>::NumHandles; ++handle)
        {
            for (MemoryHandle handle2 = 0; handle2 < AnimationDataBindTestContainer<UInt>::NumHandles; ++handle2)
            {
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, true, false);
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, -1, 999);
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, 1e-6f, 999.999f);
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector2(1, 2), Vector2(-1, -2));
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector3(1, 2, 3), Vector3(-1, -2, -3));
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector4(1, 2, 3, 4), Vector4(-1, -2, -3, -4));
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector2i(1, 2), Vector2i(-1, -2));
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector3i(1, 2, 3), Vector3i(-1, -2, -3));
                AnimationDataBindTestHelper::testDataBindingTypes2(handle, handle2, Vector4i(1, 2, 3, 4), Vector4i(-1, -2, -3, -4));
            }
        }
    }

    TEST_F(AnimationDataBindTest, InitialValue)
    {
        const Vector3 initVal(1, 2, 3);
        AnimationDataBindTestVector3 container;
        container.setVal0(initVal);
        AnimationDataBind< AnimationDataBindTestVector3, Vector3 > dataBind(container, EDataBindAccessorType_Handles_None);
        dataBind.setInitialValue();
        EXPECT_EQ(initVal, dataBind.getInitialValue());

        dataBind.setValue(Vector3(4, 5, 6));
        EXPECT_EQ(initVal, dataBind.getInitialValue());
    }

    TEST_F(AnimationDataBindTest, DataTypeID)
    {
        AnimationDataBindTestHelper::testDataTypeID<bool>();
        AnimationDataBindTestHelper::testDataTypeID<Int32>();
        AnimationDataBindTestHelper::testDataTypeID<Float>();
        AnimationDataBindTestHelper::testDataTypeID<Vector2>();
        AnimationDataBindTestHelper::testDataTypeID<Vector3>();
        AnimationDataBindTestHelper::testDataTypeID<Vector4>();
        AnimationDataBindTestHelper::testDataTypeID<Vector2i>();
        AnimationDataBindTestHelper::testDataTypeID<Vector3i>();
        AnimationDataBindTestHelper::testDataTypeID<Vector4i>();
    }

    TEST_F(AnimationDataBindTest, ContainerTypeID)
    {
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<bool>, bool>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Int32>, Int32>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Float>, Float>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Vector2>, Vector2>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Vector3>, Vector3>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Vector4>, Vector4>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Vector2i>, Vector2i>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Vector3i>, Vector3i>();
        AnimationDataBindTestHelper::testContainerTypeID<AnimationDataBindTestContainer<Vector4i>, Vector4i>();
    }

    TEST_F(AnimationDataBindTest, AccessorTypeID)
    {
        AnimationDataBindTestVector3 container;
        const AnimationDataBindBase* pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3 >(container, EDataBindAccessorType_Handles_None);
        EXPECT_EQ(EDataBindAccessorType_Handles_None, pDataBind->getAccessorType());
        delete pDataBind;

        pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3, MemoryHandle >(container, 0u, EDataBindAccessorType_Handles_1);
        EXPECT_EQ(EDataBindAccessorType_Handles_1, pDataBind->getAccessorType());
        delete pDataBind;

        pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3, MemoryHandle, MemoryHandle >(container, 0u, 0u, EDataBindAccessorType_Handles_2);
        EXPECT_EQ(EDataBindAccessorType_Handles_2, pDataBind->getAccessorType());
        delete pDataBind;
    }

    TEST_F(AnimationDataBindTest, GettersNoHandle)
    {
        AnimationDataBindTestVector3 container;
        const AnimationDataBindBase* pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3 >(container, EDataBindAccessorType_Handles_None);
        EXPECT_EQ(static_cast<TDataBindID>(EDataBindAccessorType_Handles_None), pDataBind->getBindID());
        EXPECT_EQ(InvalidMemoryHandle, pDataBind->getHandle());
        EXPECT_EQ(InvalidMemoryHandle, pDataBind->getHandle2());
        delete pDataBind;
    }

    TEST_F(AnimationDataBindTest, Getters1Handle)
    {
        AnimationDataBindTestVector3 container;
        const AnimationDataBindBase* pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3, MemoryHandle >(container, 11u, EDataBindAccessorType_Handles_1);
        EXPECT_EQ(11u, pDataBind->getHandle());
        EXPECT_EQ(InvalidMemoryHandle, pDataBind->getHandle2());
        EXPECT_EQ(static_cast<TDataBindID>(EDataBindAccessorType_Handles_1), pDataBind->getBindID());
        delete pDataBind;
    }

    TEST_F(AnimationDataBindTest, Getters2Handles)
    {
        AnimationDataBindTestVector3 container;
        const AnimationDataBindBase* pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3, MemoryHandle, MemoryHandle >(container, 11u, 999u, EDataBindAccessorType_Handles_2);
        EXPECT_EQ(11u, pDataBind->getHandle());
        EXPECT_EQ(999u, pDataBind->getHandle2());
        EXPECT_EQ(static_cast<TDataBindID>(EDataBindAccessorType_Handles_2), pDataBind->getBindID());
        delete pDataBind;
    }

    TEST_F(AnimationDataBindTest, canCheckPropertyValidity1Handle)
    {
        AnimationDataBindTestVector3 container;
        const AnimationDataBindBase* pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3, MemoryHandle >(container, AnimationDataBindTestVector3::NumHandles, EDataBindAccessorType_Handles_1);
        EXPECT_FALSE(pDataBind->isPropertyValid());
        delete pDataBind;
    }

    TEST_F(AnimationDataBindTest, canCheckPropertyValidity2Handles)
    {
        AnimationDataBindTestVector3 container;
        const AnimationDataBindBase* pDataBind = new AnimationDataBind < AnimationDataBindTestVector3, Vector3, MemoryHandle, MemoryHandle >(container, AnimationDataBindTestVector3::NumHandles, 1u, EDataBindAccessorType_Handles_2);
        EXPECT_FALSE(pDataBind->isPropertyValid());
        delete pDataBind;
    }

    template <typename EDataType>
    class ContainerBase
    {
    public:
        using GetSetType = typename DataTypeReferenceSelector<EDataType>::PossibleReferenceType;

        virtual ~ContainerBase() {}

        virtual GetSetType getValue(UInt32 handle) const = 0;
        virtual void setValue(UInt32 handle, GetSetType data) = 0;
        virtual bool isPropertyValid(UInt32 handle) const = 0;
    };

    template <typename EDataType>
    class Container : public ContainerBase<EDataType>
    {
    public:
        using GetSetType = typename DataTypeReferenceSelector<EDataType>::PossibleReferenceType;

        virtual GetSetType getValue(UInt32 handle) const override
        {
            return m_data[handle];
        }

        virtual void setValue(UInt32 handle, GetSetType data) override
        {
            m_data[handle] = data;
        }

        virtual bool isPropertyValid(UInt32 handle) const override
        {
            return handle < 10u;
        }

    protected:
        EDataType m_data[10];
    };

    template <typename ContainerBaseType, typename EDataType>
    class ContainerDerived : public ContainerBaseType
    {
    public:
        using GetSetType = typename DataTypeReferenceSelector<EDataType>::PossibleReferenceType;

        virtual GetSetType getValue(UInt32 handle) const override
        {
            UNUSED(handle);
            return InvalidData;
        }

        virtual void setValue(UInt32 handle, GetSetType data) override
        {
            UNUSED(data);
            this->m_data[handle] = InvalidData;
        }

        static const EDataType InvalidData = -1;
    };

    template <typename ContainerBaseType, typename EDataType>
    const EDataType ContainerDerived<ContainerBaseType, EDataType>::InvalidData;

    using BindContainerType = ContainerBase<Int32>;
    using UsedContainerType = ContainerDerived<Container<Int32>, Int32>;

    DATA_BIND_DECLARE_BEGIN(BindContainerType, 1)
        DATA_BIND_DECLARE(0, VALUE0)
    DATA_BIND_DECLARE_END(BindContainerType, EDataBindContainerType(EDataBindContainerType_User + 1000u))

    DATA_BIND_DEFINE_BEGIN(BindContainerType)
        DATA_BIND_DEFINE(0, BindContainerType, BindContainerType::getValue, BindContainerType::setValue, BindContainerType::isPropertyValid, EDataTypeID_Int32, EDataBindAccessorType_Handles_1)
    DATA_BIND_DEFINE_END()

    TEST_F(AnimationDataBindTest, DerivedContainer)
    {
        UsedContainerType container;
        const UInt32 handle = 5u;
        container.setValue(handle, 10);

        AnimationDataBind<BindContainerType, Int32, UInt32> dataBind(container, handle, DataBindContainerToTraitsSelector<BindContainerType>::ContainerTraitsClassType::VALUE0);
        EXPECT_EQ(dataBind.getValue(), UsedContainerType::InvalidData);

        dataBind.setValue(99);
        EXPECT_EQ(dataBind.getValue(), UsedContainerType::InvalidData);

        EXPECT_TRUE(dataBind.isPropertyValid());
    }
}
