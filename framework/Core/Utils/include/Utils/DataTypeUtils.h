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
#include "DataTypesImpl.h"

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
    struct DataTypeToDataIDSelector < float >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Float;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::vec2 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector2f;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::vec3 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector3f;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::vec4 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector4f;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::mat2 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Matrix22f;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::mat3 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Matrix33f;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::mat4 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Matrix44f;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::ivec2 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector2i;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::ivec3 >
    {
        static const EDataTypeID DataTypeID = EDataTypeID_Vector3i;
    };

    template <>
    struct DataTypeToDataIDSelector < glm::ivec4 >
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
        using DataType = float;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector2f >
    {
        using DataType = glm::vec2;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector3f >
    {
        using DataType = glm::vec3;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector4f >
    {
        using DataType = glm::vec4;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix22f >
    {
        using DataType = glm::mat2;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix33f >
    {
        using DataType = glm::mat3;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Matrix44f >
    {
        using DataType = glm::mat4;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector2i >
    {
        using DataType = glm::ivec2;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector3i >
    {
        using DataType = glm::ivec3;
    };

    template <>
    struct DataIDToDataTypeSelector < EDataTypeID_Vector4i >
    {
        using DataType = glm::ivec4;
    };
}

#endif
