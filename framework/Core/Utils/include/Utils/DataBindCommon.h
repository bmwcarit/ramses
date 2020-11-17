//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATABINDCOMMON_H
#define RAMSES_DATABINDCOMMON_H

#include "Utils/DataBindTypes.h"

// Generate a data bind class used for data access to given container type
#define DATA_BIND_GET_CLASS(_containerType) _containerType##DataBinding

// Generate code used for data access.
// In order to access data of arbitrary type from arbitrary container a helper class is generated for every such container interface.
// This helper class then defines data binding to the container interface via function pointers to getter/setter of the data.
// Every data member registered this way has its data bind ID, which is exposed and refers to the
// corresponding data member getter/setter.
// This allows generic way of access to any data that is registered.
//
// REQUIREMENTS for a class and data members to be able to use data binding:
//  - getter and setter exists
//  - getter is of type
//       const DataType& getter() const
//       const DataType& getter(Handle) const
//       const DataType& getter(Handle1, Handle2) const
//  - setter is of type
//       void setter(const DataType&)
//       void setter(Handle, const DataType&)
//       void setter(Handle1, Handle2, const DataType&)
#define DATA_BIND_DECLARE_BEGIN(_containerType, _count) \
    class DATA_BIND_GET_CLASS(_containerType) \
    { \
    public: \
        static const DataBindContainerTraits<_containerType> m_dataBindTraits[_count];
#define DATA_BIND_DECLARE(_id, _name) \
        static const TDataBindID _name = (_id);
#define DATA_BIND_DECLARE_END(_containerType, _containerTypeID) \
    }; \
    DATA_BIND_REGISTER_CLASS_TYPE(_containerType, _containerTypeID);

#define DATA_BIND_DEFINE_BEGIN(_containerType) \
    const DataBindContainerTraits<_containerType> DATA_BIND_GET_CLASS(_containerType)::m_dataBindTraits[] = \
    {
#define DATA_BIND_DEFINE(_id, _containerType, _getter, _setter, _checker, _dataTypeID, _accessorTypeID) \
        { \
            reinterpret_cast<DataBindContainerTraits<_containerType>::TFuncGeneric>(&_getter), \
            reinterpret_cast<DataBindContainerTraits<_containerType>::TFuncGeneric>(&_setter), \
            reinterpret_cast<DataBindContainerTraits<_containerType>::TFuncGeneric>(&_checker), \
            _dataTypeID, \
            _accessorTypeID \
        },
#define DATA_BIND_DEFINE_END() \
    };

#define DATA_BIND_REGISTER_CLASS_TYPE(_containerType, _containerTypeID) \
    template <> \
    struct DataBindContainerToTraitsSelector < _containerType > \
    { \
        typedef DATA_BIND_GET_CLASS(_containerType) ContainerTraitsClassType; \
    }; \
    template <> \
    struct DataBindContainerToContainerIDSelector < _containerType > \
    { \
        static const EDataBindContainerType ContainerID = _containerTypeID; \
    }; \
    template <> \
    struct DataBindContainerIDToContainerTypeSelector < _containerTypeID > \
    { \
        typedef _containerType ContainerType; \
    }



namespace ramses_internal
{
    using TDataBindID = UInt32;

    template <typename ClassType>
    struct DataBindContainerTraits
    {
        using TFuncGeneric = void (ClassType::*)();

        const TFuncGeneric m_pGetterFunc;
        const TFuncGeneric m_pSetterFunc;
        const TFuncGeneric m_pCheckerFunc;
        const EDataTypeID m_eDataTypeID;
        const EDataBindAccessorType m_eAccessorTypeID;
    };
}

#endif
