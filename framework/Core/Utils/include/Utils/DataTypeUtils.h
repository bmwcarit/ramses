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
    struct DataTypeToDataIDSelector < Bool >
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
        typedef bool DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Int32 >
    {
        typedef Int32 DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Float >
    {
        typedef Float DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector2f >
    {
        typedef Vector2 DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector3f >
    {
        typedef Vector3 DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector4f >
    {
        typedef Vector4 DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix22f >
    {
        typedef Matrix22f DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix33f >
    {
        typedef Matrix33f DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix44f >
    {
        typedef Matrix44f DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector2i >
    {
        typedef Vector2i DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector3i >
    {
        typedef Vector3i DataType;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector4i >
    {
        typedef Vector4i DataType;
    };
}

#endif
