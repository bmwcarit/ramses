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
        using ContainerTraitsClassType = T;
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
        using PossibleReferenceType = T;
    };

    template <>
    struct DataTypeReferenceSelector < bool >
    {
        using PossibleReferenceType = bool;
    };

    template <>
    struct DataTypeReferenceSelector < Int32 >
    {
        using PossibleReferenceType = Int32;
    };

    template <>
    struct DataTypeReferenceSelector < Int64 >
    {
        using PossibleReferenceType = Int64;
    };

    template <>
    struct DataTypeReferenceSelector < UInt32 >
    {
        using PossibleReferenceType = UInt32;
    };

    template <>
    struct DataTypeReferenceSelector < UInt64 >
    {
        using PossibleReferenceType = UInt64;
    };

    template <>
    struct DataTypeReferenceSelector < Float >
    {
        using PossibleReferenceType = Float;
    };

    template <>
    struct DataTypeReferenceSelector < Double >
    {
        using PossibleReferenceType = Double;
    };

    template <>
    struct DataTypeReferenceSelector < Vector2 >
    {
        using PossibleReferenceType = const Vector2 &;
    };

    template <>
    struct DataTypeReferenceSelector < Vector3 >
    {
        using PossibleReferenceType = const Vector3 &;
    };

    template <>
    struct DataTypeReferenceSelector < Vector4 >
    {
        using PossibleReferenceType = const Vector4 &;
    };

    template <>
    struct DataTypeReferenceSelector < Matrix44f >
    {
        using PossibleReferenceType = const Matrix44f &;
    };

    template <>
    struct DataTypeReferenceSelector < Vector2i >
    {
        using PossibleReferenceType = const Vector2i &;
    };

    template <>
    struct DataTypeReferenceSelector < Vector3i >
    {
        using PossibleReferenceType = const Vector3i &;
    };

    template <>
    struct DataTypeReferenceSelector < Vector4i >
    {
        using PossibleReferenceType = const Vector4i &;
    };
}

#endif
