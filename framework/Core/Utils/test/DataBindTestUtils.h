//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATABINDTESTUTILS_H
#define RAMSES_DATABINDTESTUTILS_H

#include "framework_common_gmock_header.h"
#include "Utils/DataBind.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    template <typename EDataType, typename HandleType = UInt32>
    class DataBindTestContainer
    {
    public:
        using GetSetType = typename DataTypeReferenceSelector<EDataType>::PossibleReferenceType;

        GetSetType getVal0() const
        {
            return m_val[0][0];
        }
        void setVal0(GetSetType val)
        {
            m_val[0][0] = val;
        }
        bool checkProperty0() const
        {
            return true;
        }

        GetSetType getVal1(HandleType handle) const
        {
            return m_val[0][handle];
        }
        void setVal1(HandleType handle, GetSetType val)
        {
            m_val[0][handle] = val;
        }
        bool checkProperty1(HandleType handle) const
        {
            return handle < NumHandles;
        }

        GetSetType getVal2(HandleType handle, HandleType handle2) const
        {
            return m_val[handle2][handle];
        }
        void setVal2(HandleType handle, HandleType handle2, GetSetType val)
        {
            m_val[handle2][handle] = val;
        }
        bool checkProperty2(HandleType handle) const
        {
            return handle < NumHandles;
        }

        static const HandleType NumHandles = 10u;

    private:
        EDataType m_val[NumHandles][NumHandles];
    };

    class DataBindTestHelper
    {
    public:
        template <typename EDataType>
        static void testDataBindingTypes0(const EDataType constainerInitVal, const EDataType initVal)
        {
            DataBindTestContainer<EDataType> container;
            container.setVal0(constainerInitVal);
            EDataType val(initVal);
            getValueViaBinding0(container, EDataBindAccessorType_Handles_None, val);
            EXPECT_EQ(container.getVal0(), val);
            val = initVal;
            setValueViaBinding0(container, EDataBindAccessorType_Handles_None, val);
            EXPECT_EQ(container.getVal0(), val);
        }

        template <typename EDataType, typename HandleType>
        static void testDataBindingTypes1(HandleType handle, const EDataType constainerInitVal, const EDataType initVal)
        {
            DataBindTestContainer<EDataType, HandleType> container;
            container.setVal1(handle, constainerInitVal);
            EDataType val(initVal);
            getValueViaBinding1(container, handle, EDataBindAccessorType_Handles_1, val);
            EXPECT_EQ(container.getVal1(handle), val);
            val = initVal;
            setValueViaBinding1(container, handle, EDataBindAccessorType_Handles_1, val);
            EXPECT_EQ(container.getVal1(handle), val);
        }

        template <typename EDataType, typename HandleType>
        static void testDataBindingTypes2(HandleType handle, HandleType handle2, const EDataType constainerInitVal, const EDataType initVal)
        {
            DataBindTestContainer<EDataType, HandleType> container;
            container.setVal2(handle, handle2, constainerInitVal);
            EDataType val(initVal);
            getValueViaBinding2(container, handle, handle2, EDataBindAccessorType_Handles_2, val);
            EXPECT_EQ(container.getVal2(handle, handle2), val);
            val = initVal;
            setValueViaBinding2(container, handle, handle2, EDataBindAccessorType_Handles_2, val);
            EXPECT_EQ(container.getVal2(handle, handle2), val);
        }

        template <typename ClassType, typename EDataType>
        static void getValueViaBinding0(ClassType& instance, TDataBindID id, EDataType& value)
        {
            DataBind<ClassType, EDataType> dataBind(instance, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            value = dataBind.getValue();
        }
        template <typename ClassType, typename EDataType>
        static void setValueViaBinding0(ClassType& instance, TDataBindID id, const EDataType& value)
        {
            DataBind<ClassType, EDataType> dataBind(instance, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            dataBind.setValue(value);
        }

        template <typename ClassType, typename EDataType, typename HandleType>
        static void getValueViaBinding1(ClassType& instance, HandleType handle, TDataBindID id, EDataType& value)
        {
            DataBind<ClassType, EDataType, HandleType> dataBind(instance, handle, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            value = dataBind.getValue();
        }
        template <typename ClassType, typename EDataType, typename HandleType>
        static void setValueViaBinding1(ClassType& instance, HandleType handle, TDataBindID id, const EDataType& value)
        {
            DataBind<ClassType, EDataType, HandleType> dataBind(instance, handle, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            dataBind.setValue(value);
        }

        template <typename ClassType, typename EDataType, typename HandleType>
        static void getValueViaBinding2(ClassType& instance, HandleType handle, HandleType handle2, TDataBindID id, EDataType& value)
        {
            DataBind<ClassType, EDataType, HandleType, HandleType> dataBind(instance, handle, handle2, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            value = dataBind.getValue();
        }
        template <typename ClassType, typename EDataType, typename HandleType>
        static void setValueViaBinding2(ClassType& instance, HandleType handle, HandleType handle2, TDataBindID id, const EDataType& value)
        {
            DataBind<ClassType, EDataType, HandleType, HandleType> dataBind(instance, handle, handle2, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            dataBind.setValue(value);
        }
    };

    using DataBindTestBool = DataBindTestContainer<bool>;
    using DataBindTestInt32 = DataBindTestContainer<Int32>;
    using DataBindTestFloat = DataBindTestContainer<Float>;
    using DataBindTestVector2 = DataBindTestContainer<Vector2>;
    using DataBindTestVector3 = DataBindTestContainer<Vector3>;
    using DataBindTestVector4 = DataBindTestContainer<Vector4>;
    using DataBindTestMatrix44f = DataBindTestContainer<Matrix44f>;
    using DataBindTestVector2i = DataBindTestContainer<Vector2i>;
    using DataBindTestVector3i = DataBindTestContainer<Vector3i>;
    using DataBindTestVector4i = DataBindTestContainer<Vector4i>;

    DATA_BIND_DECLARE_BEGIN(DataBindTestBool, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestBool, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Boolean))

    DATA_BIND_DECLARE_BEGIN(DataBindTestInt32, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestInt32, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Int32))

    DATA_BIND_DECLARE_BEGIN(DataBindTestFloat, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestFloat, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Float))

    DATA_BIND_DECLARE_BEGIN(DataBindTestVector2, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestVector2, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector2f))

    DATA_BIND_DECLARE_BEGIN(DataBindTestVector3, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestVector3, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector3f))

    DATA_BIND_DECLARE_BEGIN(DataBindTestVector4, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestVector4, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector4f))

    DATA_BIND_DECLARE_BEGIN(DataBindTestMatrix44f, 3)
    DATA_BIND_DECLARE(0, VALUE0)
    DATA_BIND_DECLARE(1, VALUE1)
    DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestMatrix44f, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Matrix44f))

    DATA_BIND_DECLARE_BEGIN(DataBindTestVector2i, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestVector2i, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector2i))

    DATA_BIND_DECLARE_BEGIN(DataBindTestVector3i, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestVector3i, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector3i))

    DATA_BIND_DECLARE_BEGIN(DataBindTestVector4i, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
    DATA_BIND_DECLARE_END(DataBindTestVector4i, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector4i))
}

#endif
