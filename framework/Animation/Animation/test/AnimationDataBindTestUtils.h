//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONDATABINDTESTUTILS_H
#define RAMSES_ANIMATIONDATABINDTESTUTILS_H

#include "framework_common_gmock_header.h"
#include "Animation/AnimationProcessDataDispatch.h"
#include "Animation/AnimationDataBind.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    template <typename EDataType, typename HandleType = MemoryHandle>
    class AnimationDataBindTestContainer
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
        bool isPropertyValid0() const
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
        bool isPropertyValid1(HandleType handle) const
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
        bool isPropertyValid2(HandleType handle) const
        {
            return handle < NumHandles;
        }

        static const UInt32 NumHandles = 10u;

    private:
        EDataType m_val[NumHandles][NumHandles];
    };

    class AnimationDataBindTestHelper
    {
    public:
        template <typename EDataType>
        static void testDataBindingTypes0(const EDataType constainerInitVal, const EDataType initVal)
        {
            AnimationDataBindTestContainer<EDataType> container;
            container.setVal0(constainerInitVal);
            EDataType val(initVal);
            getValueViaBinding0(container, EDataBindAccessorType_Handles_None, val);
            EXPECT_EQ(container.getVal0(), val);
            val = initVal;
            setValueViaBinding0(container, EDataBindAccessorType_Handles_None, val);
            EXPECT_EQ(container.getVal0(), val);
        }

        template <typename EDataType>
        static void testDataBindingTypes1(MemoryHandle handle, const EDataType constainerInitVal, const EDataType initVal)
        {
            AnimationDataBindTestContainer<EDataType> container;
            container.setVal1(handle, constainerInitVal);
            EDataType val(initVal);
            getValueViaBinding1(container, handle, EDataBindAccessorType_Handles_1, val);
            EXPECT_EQ(container.getVal1(handle), val);
            val = initVal;
            setValueViaBinding1(container, handle, EDataBindAccessorType_Handles_1, val);
            EXPECT_EQ(container.getVal1(handle), val);
        }

        template <typename EDataType>
        static void testDataBindingTypes2(MemoryHandle handle, MemoryHandle handle2, const EDataType constainerInitVal, const EDataType initVal)
        {
            AnimationDataBindTestContainer<EDataType> container;
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
            AnimationDataBind<ClassType, EDataType> dataBind(instance, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            value = dataBind.getValue();
        }
        template <typename ClassType, typename EDataType>
        static void setValueViaBinding0(ClassType& instance, TDataBindID id, const EDataType& value)
        {
            AnimationDataBind<ClassType, EDataType> dataBind(instance, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            dataBind.setValue(value);
        }

        template <typename ClassType, typename EDataType, typename HandleType>
        static void getValueViaBinding1(ClassType& instance, HandleType handle, TDataBindID id, EDataType& value)
        {
            AnimationDataBind<ClassType, EDataType, HandleType> dataBind(instance, handle, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            value = dataBind.getValue();
        }
        template <typename ClassType, typename EDataType, typename HandleType>
        static void setValueViaBinding1(ClassType& instance, HandleType handle, TDataBindID id, const EDataType& value)
        {
            AnimationDataBind<ClassType, EDataType, HandleType> dataBind(instance, handle, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            dataBind.setValue(value);
        }

        template <typename ClassType, typename EDataType, typename HandleType>
        static void getValueViaBinding2(ClassType& instance, HandleType handle, HandleType handle2, TDataBindID id, EDataType& value)
        {
            AnimationDataBind<ClassType, EDataType, HandleType, HandleType> dataBind(instance, handle, handle2, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            value = dataBind.getValue();
        }
        template <typename ClassType, typename EDataType, typename HandleType>
        static void setValueViaBinding2(ClassType& instance, HandleType handle, HandleType handle2, TDataBindID id, const EDataType& value)
        {
            AnimationDataBind<ClassType, EDataType, HandleType, HandleType> dataBind(instance, handle, handle2, id);
            EXPECT_TRUE(dataBind.isPropertyValid());
            dataBind.setValue(value);
        }

        template <typename EDataType>
        static void testDataTypeID()
        {
            AnimationDataBindTestContainer<EDataType> container;
            const AnimationDataBindBase* const pDataBind = new AnimationDataBind < AnimationDataBindTestContainer<EDataType>, EDataType >(container, 0u);
            const EDataTypeID dataTypeID = DataTypeToDataIDSelector<EDataType>::DataTypeID;
            EXPECT_EQ(dataTypeID, pDataBind->getDataType());
            delete pDataBind;
        }

        template <typename ContainerType, typename EDataType>
        static void testContainerTypeID()
        {
            ContainerType container;
            const AnimationDataBindBase* const pDataBind = new AnimationDataBind < ContainerType, EDataType, MemoryHandle >(container, 0u, EDataBindAccessorType_Handles_1);
            const EDataBindContainerType containerTypeID = DataBindContainerToContainerIDSelector<ContainerType>::ContainerID;
            EXPECT_EQ(containerTypeID, pDataBind->getContainerType());
            delete pDataBind;
        }
    };

    using AnimationDataBindTestBool = AnimationDataBindTestContainer<bool>;
    using AnimationDataBindTestInt32 = AnimationDataBindTestContainer<Int32>;
    using AnimationDataBindTestFloat = AnimationDataBindTestContainer<Float>;
    using AnimationDataBindTestVector2 = AnimationDataBindTestContainer<Vector2>;
    using AnimationDataBindTestVector3 = AnimationDataBindTestContainer<Vector3>;
    using AnimationDataBindTestVector4 = AnimationDataBindTestContainer<Vector4>;
    using AnimationDataBindTestVector2i = AnimationDataBindTestContainer<Vector2i>;
    using AnimationDataBindTestVector3i = AnimationDataBindTestContainer<Vector3i>;
    using AnimationDataBindTestVector4i = AnimationDataBindTestContainer<Vector4i>;

    DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestBool, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestBool, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Boolean))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestInt32, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestInt32, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Int32))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestFloat, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestFloat, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Float))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestVector2, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestVector2, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector2f))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestVector3, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestVector3, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector3f))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestVector4, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestVector4, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector4f))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestVector2i, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestVector2i, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector2i))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestVector3i, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestVector3i, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector3i))

        DATA_BIND_DECLARE_BEGIN(AnimationDataBindTestVector4i, 3)
        DATA_BIND_DECLARE(0, VALUE0)
        DATA_BIND_DECLARE(1, VALUE1)
        DATA_BIND_DECLARE(2, VALUE2)
        DATA_BIND_DECLARE_END(AnimationDataBindTestVector4i, EDataBindContainerType(EDataBindContainerType_User + EDataTypeID_Vector4i))
}

#endif
