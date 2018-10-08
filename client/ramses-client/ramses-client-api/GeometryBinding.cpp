//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"

// internal
#include "GeometryBindingImpl.h"

namespace ramses
{
    GeometryBinding::GeometryBinding(GeometryBindingImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    GeometryBinding::~GeometryBinding()
    {
    }

    status_t GeometryBinding::setIndices(const UInt16Array& indicesResource)
    {
        const status_t status = impl.setIndices(indicesResource.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(indicesResource));
        return status;
    }

    status_t GeometryBinding::setIndices(const UInt32Array& indicesResource)
    {
        const status_t status = impl.setIndices(indicesResource.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(indicesResource));
        return status;
    }

    status_t GeometryBinding::setIndices(const IndexDataBuffer& dataBuffer)
    {
        const status_t status = impl.setIndices(dataBuffer.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(dataBuffer));
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const FloatArray& bufferResource, uint32_t instancingDivisor)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, bufferResource.impl, instancingDivisor);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(bufferResource), instancingDivisor);
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const Vector2fArray& bufferResource, uint32_t instancingDivisor)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, bufferResource.impl, instancingDivisor);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(bufferResource), instancingDivisor);
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const Vector3fArray& bufferResource, uint32_t instancingDivisor)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, bufferResource.impl, instancingDivisor);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(bufferResource), instancingDivisor);
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const Vector4fArray& bufferResource, uint32_t instancingDivisor)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, bufferResource.impl, instancingDivisor);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(bufferResource), instancingDivisor);
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const VertexDataBuffer& bufferResource, uint32_t instancingDivisor /*= 0*/)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, bufferResource.impl, instancingDivisor);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(bufferResource), instancingDivisor);
        return status;
    }

    const Effect& GeometryBinding::getEffect() const
    {
        return impl.getEffect();
    }
}
