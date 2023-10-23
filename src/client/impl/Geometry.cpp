//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/Geometry.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/ArrayBuffer.h"

// internal
#include "impl/GeometryImpl.h"

namespace ramses
{
    Geometry::Geometry(std::unique_ptr<internal::GeometryImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::GeometryImpl&>(SceneObject::m_impl) }
    {
    }

    bool Geometry::setIndices(const ArrayResource& indicesResource)
    {
        const bool status = m_impl.setIndices(indicesResource.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(indicesResource));
        return status;
    }

    bool Geometry::setIndices(const ArrayBuffer& arrayBuffer)
    {
        const bool status = m_impl.setIndices(arrayBuffer.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(arrayBuffer));
        return status;
    }

    bool Geometry::setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint32_t instancingDivisor)
    {
        const bool status = m_impl.setInputBuffer(attributeInput.impl(), arrayResource.impl(), instancingDivisor, 0u, 0u);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayResource), instancingDivisor);
        return status;
    }

    bool Geometry::setInputBuffer(const AttributeInput& attributeInput, const ArrayResource& arrayResource, uint16_t offset, uint16_t stride)
    {
        const bool status = m_impl.setInputBuffer(attributeInput.impl(), arrayResource.impl(), 0u, offset, stride);
        LOG_HL_CLIENT_API4(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayResource), offset, stride);
        return status;
    }

    bool Geometry::setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint32_t instancingDivisor /*= 0*/)
    {
        const bool status = m_impl.setInputBuffer(attributeInput.impl(), arrayBuffer.impl(), instancingDivisor, 0u, 0u);
        LOG_HL_CLIENT_API3(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayBuffer), instancingDivisor);
        return status;
    }

    bool Geometry::setInputBuffer(const AttributeInput& attributeInput, const ArrayBuffer& arrayBuffer, uint16_t offset, uint16_t stride)
    {
        const bool status = m_impl.setInputBuffer(attributeInput.impl(), arrayBuffer.impl(), 0u, offset, stride);
        LOG_HL_CLIENT_API4(status, LOG_API_GENERIC_OBJECT_STRING(attributeInput), LOG_API_RAMSESOBJECT_STRING(arrayBuffer), offset, stride);
        return status;
    }

    const Effect& Geometry::getEffect() const
    {
        return m_impl.getEffect();
    }

    internal::GeometryImpl& Geometry::impl()
    {
        return m_impl;
    }

    const internal::GeometryImpl& Geometry::impl() const
    {
        return m_impl;
    }
}
