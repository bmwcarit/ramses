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
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/ArrayBuffer.h"

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

    status_t GeometryBinding::setIndices(const ArrayResource& indicesResource)
    {
        const status_t status = impl.setIndices(indicesResource.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(indicesResource));
        return status;
    }

    status_t GeometryBinding::setIndices(const ArrayBuffer& arrayBuffer)
    {
        const status_t status = impl.setIndices(arrayBuffer.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(arrayBuffer));
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint32_t instancingDivisor)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, arrayResource.impl, instancingDivisor, 0u, 0u);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayResource), instancingDivisor);
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint16_t offset, uint16_t stride)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, arrayResource.impl, 0u, offset, stride);
        LOG_HL_CLIENT_API4(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayResource), offset, stride);
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint32_t instancingDivisor /*= 0*/)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, arrayBuffer.impl, instancingDivisor, 0u, 0u);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayBuffer), instancingDivisor);
        return status;
    }

    status_t GeometryBinding::setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint16_t offset, uint16_t stride)
    {
        const status_t status = impl.setInputBuffer(attributeInput.impl, arrayBuffer.impl, 0u, offset, stride);
        LOG_HL_CLIENT_API4(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayBuffer), offset, stride);
        return status;
    }

    const Effect& GeometryBinding::getEffect() const
    {
        return impl.getEffect();
    }
}
