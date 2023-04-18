//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYBUFFER_H
#define RAMSES_ARRAYBUFFER_H

#include "ramses-client-api/SceneObject.h"
#include "ramses-framework-api/EDataType.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    /**
     * @brief The ArrayBuffer is a data object used to provide vertex or index data to #ramses::GeometryBinding::setInputBuffer
     *        and #ramses::GeometryBinding::setIndices. The buffer data of an ArrayBuffer is not filled initially and can be fully
     *        or partially updated in between scene flushes.
     *
     * @details If an #ArrayBuffer object is created with type #ramses::EDataType::ByteBlob then an element
     *          is defined as one byte, rather than a logical vertex element. Hence, all functions of #ArrayBuffer
     *          referring to element refer to a single byte within byte array.
     */
    class RAMSES_API ArrayBuffer : public SceneObject
    {
    public:
        /**
        * @brief Update data of the ArrayBuffer object.
        *
        * @details Data type of provided data must pass #ramses::IsArrayResourceDataType.
        *          This method will fail if the provided data type does not match this #ArrayBuffer data type
        *          (see #ramses::ArrayBuffer::getDataType and #ramses::GetEDataType).
        *          For #ArrayBuffer objects of type #ramses::EDataType::ByteBlob an element is defined as 1 byte.
        *
        * @param firstElement       The element index at which data update should begin.
        * @param numElements        The number of elements to be updated.
        *                           FirstElement + numElements must be less than or equal to getMaximumNumberOfElements
        *                           specified at creation time of the ArrayBuffer object. The function writes
        *                           (or overwrites) buffer data up to the specified count only, leaving
        *                           any previously written or un-initialized data untouched.
        * @param bufferData         Pointer in memory to the data provided for update. The function makes
        *                           a copy of the data into ArrayBuffer data structures. ArrayBuffer
        *                           object does not take ownership of the memory data passed to it.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        template <typename T>
        status_t updateData(uint32_t firstElement, uint32_t numElements, const T* bufferData);

        /**
        * @brief Returns the maximum number of data elements that can be stored in the data buffer.
        *
        * @details For #ArrayBuffer objects of type #ramses::EDataType::ByteBlob this function returns the maximum
        *          size of buffer data in bytes.
        *
        * @return Maximum number of elements
        */
        [[nodiscard]] uint32_t getMaximumNumberOfElements() const;

        /**
        * @brief Returns the used number of data elements.
        *
        * @details If data buffer fully set by #updateData, then used and maximum number of elements are equal. For #ArrayBuffer objects
        *          of type #ramses::EDataType::ByteBlob this function returns the used size of buffer data in bytes becuase element is defined
        *          as byte for this type.
        *
        * @return Used size in number of elements
        */
        [[nodiscard]] uint32_t getUsedNumberOfElements() const;

        /**
        * @brief Returns the data type associated with the buffer data
        *
        * @return data type of buffer data
        */
        [[nodiscard]] EDataType getDataType() const;

        /**
        * @brief Copies the data of the data buffer into a user-provided buffer.
        *
        * @details The buffer must be sufficiently large to hold the data for the \c numElements to be copied out.
        *          This method will fail if the provided data type does not match this #ArrayBuffer data type
        *          (see #ramses::ArrayBuffer::getDataType and #ramses::GetEDataType).
        *          For #ArrayBuffer objects of type #ramses::EDataType::ByteBlob an element is defined as 1 byte.
        *
        * @param[out] buffer The buffer where the buffer data will be copied into
        * @param[in] numElements The number of elements to copy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        template <typename T>
        status_t getData(T* buffer, uint32_t numElements) const;

        /**
        * Stores internal data for implementation specifics of ArrayBuffer.
        */
        class ArrayBufferImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating ArrayBuffer instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for ArrayBuffer.
        *
        * @param[in] pimpl Internal data for implementation specifics of ArrayBuffer (sink - instance becomes owner)
        */
        explicit ArrayBuffer(ArrayBufferImpl& pimpl);

        /**
        * @brief Destructor of the ArrayBuffer
        */
        ~ArrayBuffer() override;

    private:
        /// Internal implementation of #updateData
        template <typename T> RAMSES_API status_t updateDataInternal(uint32_t firstElement, uint32_t numElements, const T* bufferData);
        /// Internal implementation of #getData
        template <typename T> RAMSES_API status_t getDataInternal(T* buffer, uint32_t numElements) const;
    };

    template <typename T> status_t ArrayBuffer::updateData(uint32_t firstElement, uint32_t numElements, const T* bufferData)
    {
        static_assert(IsArrayResourceDataType<T>(), "Unsupported data type!");
        return updateDataInternal<T>(firstElement, numElements, bufferData);
    }

    template <typename T> status_t ArrayBuffer::getData(T* buffer, uint32_t numElements) const
    {
        static_assert(IsArrayResourceDataType<T>(), "Unsupported data type!");
        return getDataInternal<T>(buffer, numElements);
    }
}

#endif
