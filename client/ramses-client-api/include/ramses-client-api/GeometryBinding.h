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
    class UInt16Array;
    class UInt32Array;
    class IndexDataBuffer;
    class VertexDataBuffer;
    class AttributeInput;
    class FloatArray;
    class Vector2fArray;
    class Vector3fArray;
    class Vector4fArray;
    class Effect;

    /**
     * @brief A geometry binding together with an appearance describe how an object will be rendered to the screen.
    */
    class RAMSES_API GeometryBinding : public SceneObject
    {
    public:

        /**
        * @brief Assign indices resource (using 16 bit indices) to be used when accessing vertex data.
        *
        * Indices are optional, when not provided rendering uses sequential attribute elements.
        *
        * @param[in] indicesResource Indices resource.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setIndices(const UInt16Array& indicesResource);

        /**
        * @brief Assign indices resource (using 32 bit indices) to be used when accessing vertex data.
        *
        * Indices are optional, when not provided rendering uses sequential attribute elements.
        *
        * @param[in] indicesResource Indices resource.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setIndices(const UInt32Array& indicesResource);

        /**
        * @brief Assign indices resource (using index data buffer) to be used when accessing vertex data.
        *
        * Indices are optional, when not provided rendering uses sequential attribute elements.
        *
        * @param[in] dataBuffer Indices data buffer.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setIndices(const IndexDataBuffer& dataBuffer);

        /**
        * @brief Assign a vertex attribute buffer to a given effect attribute input.
        *
        * @param[in] attributeInput The effect attribute input to set the buffer to
        * @param[in] bufferResource Vertex attribute buffer resource.
        * @param[in] instancingDivisor The vertex attribute divisor used during instanced rendering. Default is 0, which means no instancing.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInputBuffer(const AttributeInput& attributeInput, const FloatArray& bufferResource, uint32_t instancingDivisor = 0);

        /** @copydoc setInputBuffer(const AttributeInput&, const FloatArray&, uint32_t instancingDivisor) */
        status_t setInputBuffer(const AttributeInput& attributeInput, const Vector2fArray& bufferResource, uint32_t instancingDivisor = 0);

        /** @copydoc setInputBuffer(const AttributeInput&, const FloatArray&, uint32_t instancingDivisor) */
        status_t setInputBuffer(const AttributeInput& attributeInput, const Vector3fArray& bufferResource, uint32_t instancingDivisor = 0);

        /** @copydoc setInputBuffer(const AttributeInput&, const FloatArray&, uint32_t instancingDivisor) */
        status_t setInputBuffer(const AttributeInput& attributeInput, const Vector4fArray& bufferResource, uint32_t instancingDivisor = 0);

        /** @copydoc setInputBuffer(const AttributeInput&, const FloatArray&, uint32_t instancingDivisor) */
        status_t setInputBuffer(const AttributeInput& attributeInput, const VertexDataBuffer& bufferResource, uint32_t instancingDivisor = 0);

        /**
        * @brief Gets the effect used to create this geometry binding
        *
        * @return The effect used to create the geometry binding.
        */
        const Effect& getEffect() const;

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
