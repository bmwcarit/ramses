//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATABIND_H
#define RAMSES_DATABIND_H

#include "Utils/DataBindCommon.h"
#include "Utils/DataBindTypes.h"

namespace ramses_internal
{
    using TypeNone = void *;

    // Forward declaration
    template <typename ContainerType, typename EDataType, typename HandleType = TypeNone, typename HandleType2 = TypeNone>
    class DataBind;

    // Specialization for no handle
    template <typename ContainerType, typename EDataType>
    class DataBind < ContainerType, EDataType, TypeNone, TypeNone >
    {
    public:
        DataBind(ContainerType& instance, const TDataBindID bindID);

        EDataType    getValue() const;
        void         setValue(const EDataType& value) const;
        bool         isPropertyValid() const;
        TDataBindID  getBindID() const;

    private:
        using GetSetType = typename DataTypeReferenceSelector<EDataType>::PossibleReferenceType;
        using TGetterFunc = GetSetType (ContainerType::*)();
        using TSetterFunc = void (ContainerType::*)(GetSetType);
        using TCheckerFunc = bool (ContainerType::*)();

        ContainerType& m_instance;
        const TDataBindID m_bindID;

        TGetterFunc  m_pGetterFunc;
        TSetterFunc  m_pSetterFunc;
        TCheckerFunc m_pCheckerFunc;
    };

    // Specialization for 1 handle
    template <typename ContainerType, typename EDataType, typename HandleType>
    class DataBind < ContainerType, EDataType, HandleType, TypeNone >
    {
    public:
        DataBind(ContainerType& instance, const HandleType handle, const TDataBindID bindID);

        EDataType   getValue() const;
        void        setValue(const EDataType& value) const;
        bool        isPropertyValid() const;
        TDataBindID getBindID() const;
        HandleType  getHandle() const;

    private:
        using GetSetType = typename DataTypeReferenceSelector<EDataType>::PossibleReferenceType;
        using TGetterFunc = GetSetType (ContainerType::*)(HandleType);
        using TSetterFunc = void (ContainerType::*)(HandleType, GetSetType);
        using TCheckerFunc = bool (ContainerType::*)(HandleType);

        ContainerType& m_instance;
        const TDataBindID m_bindID;
        const HandleType m_handle;

        TGetterFunc  m_pGetterFunc;
        TSetterFunc  m_pSetterFunc;
        TCheckerFunc m_pCheckerFunc;
    };

    // Specialization for 2 handles
    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    class DataBind
    {
    public:
        DataBind(ContainerType& instance, const HandleType handle, const HandleType2 handle2, const TDataBindID bindID);

        EDataType   getValue() const;
        void        setValue(const EDataType& value) const;
        bool        isPropertyValid() const;
        TDataBindID getBindID() const;
        HandleType  getHandle() const;
        HandleType2 getHandle2() const;

    private:
        using GetSetType = typename DataTypeReferenceSelector<EDataType>::PossibleReferenceType;
        using TGetterFunc = GetSetType (ContainerType::*)(HandleType, HandleType2);
        using TSetterFunc = void (ContainerType::*)(HandleType, HandleType2, GetSetType);
        using TCheckerFunc = bool (ContainerType::*)(HandleType); // checker func has always maximum 1 handle accessor

        ContainerType& m_instance;
        const TDataBindID m_bindID;
        const HandleType m_handle;
        const HandleType2 m_handle2;

        TGetterFunc  m_pGetterFunc;
        TSetterFunc  m_pSetterFunc;
        TCheckerFunc m_pCheckerFunc;
    };


    // Specialization for no handle
    template <typename ContainerType, typename EDataType>
    DataBind<ContainerType, EDataType, TypeNone, TypeNone>::DataBind(ContainerType& instance, const TDataBindID bindID)
        : m_instance(instance)
        , m_bindID(bindID)
    {
        const DataBindContainerTraits<ContainerType>& containerTraits = DataBindContainerToTraitsSelector<ContainerType>::ContainerTraitsClassType::m_dataBindTraits[bindID];
        static const EDataTypeID eDataType = DataTypeToDataIDSelector<EDataType>::DataTypeID;
        UNUSED(eDataType);
        assert(containerTraits.m_eDataTypeID == eDataType);
        assert(containerTraits.m_eAccessorTypeID == EDataBindAccessorType_Handles_None);

        m_pGetterFunc = reinterpret_cast<TGetterFunc>(containerTraits.m_pGetterFunc);
        m_pSetterFunc = reinterpret_cast<TSetterFunc>(containerTraits.m_pSetterFunc);
        m_pCheckerFunc = reinterpret_cast<TCheckerFunc>(containerTraits.m_pCheckerFunc);
    }

    template <typename ContainerType, typename EDataType>
    EDataType DataBind<ContainerType, EDataType, TypeNone, TypeNone>::getValue() const
    {
        return (m_instance.*m_pGetterFunc)();
    }

    template <typename ContainerType, typename EDataType>
    void DataBind<ContainerType, EDataType, TypeNone, TypeNone>::setValue(const EDataType& value) const
    {
        return (m_instance.*m_pSetterFunc)(value);
    }

    template <typename ContainerType, typename EDataType>
    bool DataBind<ContainerType, EDataType, TypeNone, TypeNone>::isPropertyValid() const
    {
        return (m_instance.*m_pCheckerFunc)();
    }

    template <typename ContainerType, typename EDataType>
    TDataBindID DataBind<ContainerType, EDataType, TypeNone, TypeNone>::getBindID() const
    {
        return m_bindID;
    }


    // Specialization for 1 handle
    template <typename ContainerType, typename EDataType, typename HandleType>
    DataBind<ContainerType, EDataType, HandleType, TypeNone>::DataBind(ContainerType& instance, const HandleType handle, const TDataBindID bindID)
        : m_instance(instance)
        , m_bindID(bindID)
        , m_handle(handle)
    {
        const DataBindContainerTraits<ContainerType>& containerTraits = DataBindContainerToTraitsSelector<ContainerType>::ContainerTraitsClassType::m_dataBindTraits[bindID];
        static const EDataTypeID eDataType = DataTypeToDataIDSelector<EDataType>::DataTypeID;
        UNUSED(eDataType);
        assert(containerTraits.m_eDataTypeID == eDataType);
        assert(containerTraits.m_eAccessorTypeID == EDataBindAccessorType_Handles_1);

        m_pGetterFunc = reinterpret_cast<TGetterFunc>(containerTraits.m_pGetterFunc);
        m_pSetterFunc = reinterpret_cast<TSetterFunc>(containerTraits.m_pSetterFunc);
        m_pCheckerFunc = reinterpret_cast<TCheckerFunc>(containerTraits.m_pCheckerFunc);
    }

    template <typename ContainerType, typename EDataType, typename HandleType>
    EDataType DataBind<ContainerType, EDataType, HandleType, TypeNone>::getValue() const
    {
        return (m_instance.*m_pGetterFunc)(m_handle);
    }

    template <typename ContainerType, typename EDataType, typename HandleType>
    void DataBind<ContainerType, EDataType, HandleType, TypeNone>::setValue(const EDataType& value) const
    {
        return (m_instance.*m_pSetterFunc)(m_handle, value);
    }

    template <typename ContainerType, typename EDataType, typename HandleType>
    bool DataBind<ContainerType, EDataType, HandleType, TypeNone>::isPropertyValid() const
    {
        return (m_instance.*m_pCheckerFunc)(m_handle);
    }

    template <typename ContainerType, typename EDataType, typename HandleType>
    TDataBindID DataBind<ContainerType, EDataType, HandleType, TypeNone>::getBindID() const
    {
        return m_bindID;
    }

    template <typename ContainerType, typename EDataType, typename HandleType>
    HandleType DataBind<ContainerType, EDataType, HandleType, TypeNone>::getHandle() const
    {
        return m_handle;
    }


    // Specialization for 2 handles
    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    DataBind<ContainerType, EDataType, HandleType, HandleType2>::DataBind(ContainerType& instance, const HandleType handle, const HandleType2 handle2, const TDataBindID bindID)
        : m_instance(instance)
        , m_bindID(bindID)
        , m_handle(handle)
        , m_handle2(handle2)
    {
        const DataBindContainerTraits<ContainerType>& containerTraits = DataBindContainerToTraitsSelector<ContainerType>::ContainerTraitsClassType::m_dataBindTraits[bindID];
        static const EDataTypeID eDataType = DataTypeToDataIDSelector<EDataType>::DataTypeID;
        UNUSED(eDataType);
        assert(containerTraits.m_eDataTypeID == eDataType);
        assert(containerTraits.m_eAccessorTypeID == EDataBindAccessorType_Handles_2);

        m_pGetterFunc = reinterpret_cast<TGetterFunc>(containerTraits.m_pGetterFunc);
        m_pSetterFunc = reinterpret_cast<TSetterFunc>(containerTraits.m_pSetterFunc);
        m_pCheckerFunc = reinterpret_cast<TCheckerFunc>(containerTraits.m_pCheckerFunc);
    }

    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    EDataType DataBind<ContainerType, EDataType, HandleType, HandleType2>::getValue() const
    {
        return (m_instance.*m_pGetterFunc)(m_handle, m_handle2);
    }

    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    void DataBind<ContainerType, EDataType, HandleType, HandleType2>::setValue(const EDataType& value) const
    {
        return (m_instance.*m_pSetterFunc)(m_handle, m_handle2, value);
    }

    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    bool DataBind<ContainerType, EDataType, HandleType, HandleType2>::isPropertyValid() const
    {
        return (m_instance.*m_pCheckerFunc)(m_handle);
    }

    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    TDataBindID DataBind<ContainerType, EDataType, HandleType, HandleType2>::getBindID() const
    {
        return m_bindID;
    }

    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    HandleType DataBind<ContainerType, EDataType, HandleType, HandleType2>::getHandle() const
    {
        return m_handle;
    }

    template <typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    HandleType2 DataBind<ContainerType, EDataType, HandleType, HandleType2>::getHandle2() const
    {
        return m_handle2;
    }
}

#endif
