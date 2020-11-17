//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATATYPEUTILS_H
#define RAMSES_DATATYPEUTILS_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    enum EDataTypeID
    {
        EDataTypeID_Invalid = 0u,
        EDataTypeID_Boolean,
        EDataTypeID_Int32,
        EDataTypeID_Float,
        EDataTypeID_Vector2f,
        EDataTypeID_Vector3f,
        EDataTypeID_Vector4f,
        EDataTypeID_Vector2i,
        EDataTypeID_Vector3i,
        EDataTypeID_Vector4i,
        EDataTypeID_Matrix22f,
        EDataTypeID_Matrix33f,
        EDataTypeID_Matrix44f
    };

    class Vector2;
    class Vector3;
    class Vector4;
    class Vector2i;
    class Vector3i;
    class Vector4i;
    class Matrix22f;
    class Matrix33f;
    class Matrix44f;

    // Maps data type to its data type ID
    template <typename T>
    struct DataTypeToDataIDSelector
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Invalid;
    };

    template <>
    struct DataTypeToDataIDSelector < bool >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Boolean;
    };

    template <>
    struct DataTypeToDataIDSelector < Int32 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Int32;
    };

    template <>
    struct DataTypeToDataIDSelector < Float >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Float;
    };

    template <>
    struct DataTypeToDataIDSelector < Vector2 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector2f;
    };

    template <>
    struct DataTypeToDataIDSelector < Vector3 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector3f;
    };

    template <>
    struct DataTypeToDataIDSelector < Vector4 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector4f;
    };

    template <>
    struct DataTypeToDataIDSelector < Matrix22f >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Matrix22f;
    };

    template <>
    struct DataTypeToDataIDSelector < Matrix33f >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Matrix33f;
    };

    template <>
    struct DataTypeToDataIDSelector < Matrix44f >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Matrix44f;
    };

    template <>
    struct DataTypeToDataIDSelector < Vector2i >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector2i;
    };

    template <>
    struct DataTypeToDataIDSelector < Vector3i >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector3i;
    };

    template <>
    struct DataTypeToDataIDSelector < Vector4i >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector4i;
    };

    // Maps data type ID to the corresponding data type
    template <EDataTypeID T>
    struct DataIDToDataTypeSelector
    {
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Boolean >
    {
        using DataType = bool;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Int32 >
    {
        using DataType = Int32;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Float >
    {
        using DataType = Float;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector2f >
    {
        using DataType = Vector2;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector3f >
    {
        using DataType = Vector3;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector4f >
    {
        using DataType = Vector4;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix22f >
    {
        using DataType = Matrix22f;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix33f >
    {
        using DataType = Matrix33f;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix44f >
    {
        using DataType = Matrix44f;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector2i >
    {
        using DataType = Vector2i;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector3i >
    {
        using DataType = Vector3i;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector4i >
    {
        using DataType = Vector4i;
    };
}

#endif
