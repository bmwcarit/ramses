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
#include <stddef.h> // for size_t

namespace ramses
{
    /**
     * Specifies the data type used for creating data buffers
    */
    enum class EDataType
    {
        UInt16 = 0, ///< one component of type uint16_t per data element
        UInt32,     ///< one component of type uint32_t per data element
        Float,      ///< one component of type float per data element
        Vector2F,   ///< two components of type float per data element
        Vector3F,   ///< three components of type float per data element
        Vector4F    ///< four components of type float per data element
    };

    /**
    * Retrieve number of components per element of specified data type
    *
    * @param[in] dataType Data type to be queried
    * @return number of components per element
    */
    constexpr size_t GetNumberOfComponents(EDataType dataType)
    {
        return (dataType == EDataType::UInt16) ? 1u :
            (dataType == EDataType::UInt32) ? 1u :
            (dataType == EDataType::Float) ? 1u :
            (dataType == EDataType::Vector2F) ? 2u :
            (dataType == EDataType::Vector3F) ? 3u :
            (dataType == EDataType::Vector4F) ? 4u : 0u;
    }

    /**
    * Retrieve size of one component of specified data type in bytes
    *
    * @param[in] dataType Data type to be queried
    * @return Size of a component of the data type in bytes
    */
    constexpr size_t GetSizeOfComponent(EDataType dataType)
    {
        return (dataType == EDataType::Float) ? sizeof(float) :
            (dataType == EDataType::UInt16) ? sizeof(uint16_t) :
            (dataType == EDataType::UInt32) ? sizeof(uint32_t) :
            (dataType == EDataType::Vector2F) ? sizeof(float) :
            (dataType == EDataType::Vector3F) ? sizeof(float) :
            (dataType == EDataType::Vector4F) ? sizeof(float) : 0u;
    }

    /**
    * Retrieve size of one element of specified data type in bytes
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
