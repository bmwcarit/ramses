//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EDATATYPE_H
#define RAMSES_EDATATYPE_H
#include <cstdint> // for integer datatypes
#include <cstddef> // for size_t
#include <cassert>

namespace ramses
{
    /**
     * @brief Specifies the data type used for creating and managing #ramses::ArrayResource,
     * #ramses::ArrayBuffer or #ramses::DataObject.
    */
    enum class EDataType
    {
        UInt16 = 0, ///< one component of type uint16_t (per data element if array)
        UInt32,     ///< one component of type uint32_t (per data element if array)
        Float,      ///< one component of type float (per data element if array)
        Vector2F,   ///< #ramses::vec2f (two components of type float) (per data element if array)
        Vector3F,   ///< #ramses::vec3f (three components of type float) (per data element if array)
        Vector4F,   ///< #ramses::vec4f (four components of type float) (per data element if array)
        ByteBlob,   ///< array of #ramses::Byte which gets typed later (e.g. interleaved vertex buffer) where one element is always sized as 1 byte

        Int32,      ///< one component of type int32_t
        Vector2I,   ///< #ramses::vec2i (two components of type int32_t) (per data element if array)
        Vector3I,   ///< #ramses::vec3i (three components of type int32_t) (per data element if array)
        Vector4I,   ///< #ramses::vec4i (four components of type int32_t) (per data element if array)
        Matrix22F,  ///< #ramses::matrix22f (two by two components of type float) (per data element if array)
        Matrix33F,  ///< #ramses::matrix33f (three by three components of type float) (per data element if array)
        Matrix44F,  ///< #ramses::matrix44f (four by four components of type float) (per data element if array)

        TextureSampler2D,       ///< 2D Texture sampler data type
        TextureSampler2DMS,     ///< 2D Texture sampler multi sampled data type
        TextureSampler3D,       ///< 3D Texture sampler data type
        TextureSamplerCube,     ///< Cube Texture sampler data type
        TextureSamplerExternal, ///< External Texture sampler data type
    };

    /**
    * @brief Retrieve number of components per element of specified data type.
    *
    * @param[in] dataType Data type to be queried
    * @return number of components per element
    */
    constexpr size_t GetNumberOfComponents(EDataType dataType)
    {
        switch (dataType)
        {
        case EDataType::Int32:
        case EDataType::UInt16:
        case EDataType::UInt32:
        case EDataType::Float:
        case EDataType::ByteBlob:
            return 1u;
        case EDataType::Vector2F:
        case EDataType::Vector2I:
            return 2u;
        case EDataType::Vector3F:
        case EDataType::Vector3I:
            return 3u;
        case EDataType::Vector4F:
        case EDataType::Vector4I:
        case EDataType::Matrix22F:
            return 4u;
        case EDataType::Matrix33F:
            return 9u;
        case EDataType::Matrix44F:
            return 16u;

        case EDataType::TextureSampler2D:
        case EDataType::TextureSampler2DMS:
        case EDataType::TextureSampler3D:
        case EDataType::TextureSamplerCube:
        case EDataType::TextureSamplerExternal:
            return 0u;
        }

        assert(false);
        return 0u;
    }

    /**
    * @brief Retrieve size of one component of specified data type in bytes.
    *
    * @details For #EDataType::ByteBlob element size is defined to be 1 byte.
    *
    * @param[in] dataType Data type to be queried
    * @return Size of a component of the data type in bytes
    */
    constexpr size_t GetSizeOfComponent(EDataType dataType)
    {
        switch (dataType)
        {
        case EDataType::UInt16:
            return sizeof(uint16_t);
        case EDataType::UInt32:
            return sizeof(uint32_t);
        case EDataType::ByteBlob:
            return 1u;
        case EDataType::Float:
        case EDataType::Vector2F:
        case EDataType::Vector3F:
        case EDataType::Vector4F:
        case EDataType::Matrix22F:
        case EDataType::Matrix33F:
        case EDataType::Matrix44F:
            return sizeof(float);
        case EDataType::Int32:
        case EDataType::Vector2I:
        case EDataType::Vector3I:
        case EDataType::Vector4I:
            return sizeof(int32_t);

        case EDataType::TextureSampler2D:
        case EDataType::TextureSampler2DMS:
        case EDataType::TextureSampler3D:
        case EDataType::TextureSamplerCube:
        case EDataType::TextureSamplerExternal:
            return 0u;
        }

        assert(false);
        return 0u;
    }

    /**
    * @brief Retrieve size of one element of specified data type in bytes.
    *
    * @param[in] dataType Data type to be queried
    * @return Size of one element of the data type in bytes
    */
    constexpr size_t GetSizeOfDataType(EDataType dataType)
    {
        return GetNumberOfComponents(dataType) * GetSizeOfComponent(dataType);
    }
}

#endif
