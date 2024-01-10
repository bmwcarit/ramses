//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/SceneObject.h"

namespace ramses
{
    namespace internal
    {
        class GeometryImpl;
    }

    class ArrayBuffer;
    class AttributeInput;
    class ArrayResource;
    class Effect;

    /**
     * @brief A geometry binding together with an appearance describe how an object will be rendered to the screen.
     * @ingroup CoreAPI
    */
    class RAMSES_API Geometry : public SceneObject
    {
    public:
        /**
        * @brief Assign a data array with data type UInt16 or UInt32 to be used when accessing vertex data.
        *
        * Indices are optional, when not provided rendering uses sequential attribute elements.
        *
        * @param[in] indicesResource An array resource carrying indices data.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setIndices(const ArrayResource& indicesResource);

        /**
        * @brief Assign indices (using index data buffer) to be used when accessing vertex data.
        *
        * Indices are optional, when not provided rendering uses sequential attribute elements.
        *
        * @param[in] arrayBuffer Indices data buffer.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setIndices(const ArrayBuffer& arrayBuffer);

        /**
        * @brief Assign a data array resource to a given effect attribute input.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayResource An array resource carrying vertex data.
        * @param[in] instancingDivisor The vertex attribute divisor used during instanced rendering. Default is 0, which means no instancing.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint32_t instancingDivisor = 0);

        /**
        * @brief   Assign a data array resource to a given effect attribute input with offset and stride.
        * @details Custom offset and/or stride can be only used with array buffers of data type #ramses::EDataType::ByteBlob.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayResource The vertex attribute buffer.
        * @param[in] offset Offset in bytes for where the attribute data is starting within the data blob.
        * @param[in] stride Stride of the interleaved vertex attribute.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint16_t offset, uint16_t stride);

        /**
        * @brief Assign a vertex attribute buffer to a given effect attribute input.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayBuffer The vertex attribute buffer.
        * @param[in] instancingDivisor The vertex attribute divisor used during instanced rendering. Default is 0, which means no instancing.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint32_t instancingDivisor = 0);

        /**
        * @brief   Assign vertex attribute buffer with offset and stride.
        * @details Custom offset and/or stride can be only used with array buffers of data type #ramses::EDataType::ByteBlob.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayBuffer The vertex attribute buffer.
        * @param[in] offset Offset in bytes for where the attribute data is starting within the data blob.
        * @param[in] stride Stride of the interleaved vertex attribute.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint16_t offset, uint16_t stride);

        /**
        * @brief Gets the effect used to create this geometry binding
        *
        * @return The effect used to create the geometry binding.
        */
        [[nodiscard]] const Effect& getEffect() const;

        /**
         * Get the internal data for implementation specifics of Geometry.
         */
        [[nodiscard]] internal::GeometryImpl& impl();

        /**
         * Get the internal data for implementation specifics of Geometry.
         */
        [[nodiscard]] const internal::GeometryImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating Geometry instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor of Geometry
        *
        * @param[in] impl Internal data for implementation specifics of Geometry (sink - instance becomes owner)
        */
        explicit Geometry(std::unique_ptr<internal::GeometryImpl> impl);

        /**
        * @brief Stores internal data for implementation specifics of Geometry.
        */
        internal::GeometryImpl& m_impl;
    };
}
