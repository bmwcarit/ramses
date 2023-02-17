//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GEOMETRYBINDING_H
#define RAMSES_GEOMETRYBINDING_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    class ArrayBuffer;
    class AttributeInput;
    class ArrayResource;
    class Effect;

    /**
     * @brief A geometry binding together with an appearance describe how an object will be rendered to the screen.
    */
    class RAMSES_API GeometryBinding : public SceneObject
    {
    public:

        /**
        * @brief Assign a data array with data type UInt16 or UInt32 to be used when accessing vertex data.
        *
        * Indices are optional, when not provided rendering uses sequential attribute elements.
        *
        * @param[in] indicesResource An array resource carrying indices data.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setIndices(const ArrayResource& indicesResource);

        /**
        * @brief Assign indices (using index data buffer) to be used when accessing vertex data.
        *
        * Indices are optional, when not provided rendering uses sequential attribute elements.
        *
        * @param[in] arrayBuffer Indices data buffer.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setIndices(const ArrayBuffer& arrayBuffer);

        /**
        * @brief Assign a data array resource to a given effect attribute input.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayResource An array resource carrying vertex data.
        * @param[in] instancingDivisor The vertex attribute divisor used during instanced rendering. Default is 0, which means no instancing.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint32_t instancingDivisor = 0);

        /**
        * @brief   Assign a data array resource to a given effect attribute input with offset and stride.
        * @details Custom offset and/or stride can be only used with array buffers of data type #ramses::EDataType::ByteBlob.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayResource The vertex attribute buffer.
        * @param[in] offset Offset in bytes for where the attribute data is starting within the data blob.
        * @param[in] stride Stride of the interleaved vertex attribute.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint16_t offset, uint16_t stride);

        /**
        * @brief Assign a vertex attribute buffer to a given effect attribute input.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayBuffer The vertex attribute buffer.
        * @param[in] instancingDivisor The vertex attribute divisor used during instanced rendering. Default is 0, which means no instancing.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint32_t instancingDivisor = 0);

        /**
        * @brief   Assign vertex attribute buffer with offset and stride.
        * @details Custom offset and/or stride can be only used with array buffers of data type #ramses::EDataType::ByteBlob.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] arrayBuffer The vertex attribute buffer.
        * @param[in] offset Offset in bytes for where the attribute data is starting within the data blob.
        * @param[in] stride Stride of the interleaved vertex attribute.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint16_t offset, uint16_t stride);

        /**
        * @brief Gets the effect used to create this geometry binding
        *
        * @return The effect used to create the geometry binding.
        */
        [[nodiscard]] const Effect& getEffect() const;

        /**
        * @brief Stores internal data for implementation specifics of GeometryBinding.
        */
        class GeometryBindingImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating GeometryBinding instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of GeometryBinding
        *
        * @param[in] pimpl Internal data for implementation specifics of GeometryBinding (sink - instance becomes owner)
        */
        explicit GeometryBinding(GeometryBindingImpl& pimpl);

        /**
        * @brief Copy constructor of GeometryBinding
        *
        * @param[in] other Other instance of GeometryBinding class
        */
        GeometryBinding(const GeometryBinding& other);

        /**
        * @brief Assignment operator of GeometryBinding.
        *
        * @param[in] other Other instance of GeometryBinding class
        * @return This instance after assignment
        */
        GeometryBinding& operator=(const GeometryBinding& other);

        /**
        * @brief Destructor of the GeometryBinding
        */
        virtual ~GeometryBinding();
    };
}

#endif
