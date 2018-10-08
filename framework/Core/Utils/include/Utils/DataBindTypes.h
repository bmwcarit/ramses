//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATABINDTYPES_H
#define RAMSES_DATABINDTYPES_H

#include "Utils/DataTypeUtils.h"

namespace ramses_internal
{
    enum EDataBindContainerType
    {
        EDataBindContainerType_Invalid = 0u,
        EDataBindContainerType_Scene,
        EDataBindContainerType_User = 64u
    };

    enum EDataBindAccessorType
    {
        EDataBindAccessorType_Handles_None = 0u,
        EDataBindAccessorType_Handles_1,
        EDataBindAccessorType_Handles_2
    };

    // Maps registered container to its container traits class
    template <typename T>
    struct DataBindContainerToTraitsSelector
    {
        typedef T ContainerTraitsClassType;
    };

    // Maps registered container to its ID
    template <typename T>
    struct DataBindContainerToContainerIDSelector
    {
        static const EDataBindContainerType ContainerID = EDataBindContainerType_Invalid;
    };

    // Maps container ID to the corresponding container type
    template <EDataBindContainerType T>
    struct DataBindContainerIDToContainerTypeSelector
    {
    };

    // Maps data type to itself for built-in types and const reference for custom types
    template <typename T>
    struct DataTypeReferenceSelector
    {
        typedef T PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Bool >
    {
        typedef Bool PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Int32 >
    {
        typedef Int32 PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Int64 >
    {
        typedef Int64 PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < UInt32 >
    {
        typedef UInt32 PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < UInt64 >
    {
        typedef UInt64 PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Float >
    {
        typedef Float PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Double >
    {
        typedef Double PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Vector2 >
    {
        typedef const Vector2& PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Vector3 >
    {
        typedef const Vector3& PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Vector4 >
    {
        typedef const Vector4& PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Matrix44f >
    {
        typedef const Matrix44f& PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Vector2i >
    {
        typedef const Vector2i& PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Vector3i >
    {
        typedef const Vector3i& PossibleReferenceType;
    };

    template <>
    struct DataTypeReferenceSelector < Vector4i >
    {
        typedef const Vector4i& PossibleReferenceType;
    };
}

#endif
