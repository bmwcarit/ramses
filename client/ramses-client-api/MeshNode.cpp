//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"

// internal
#include "NodeImpl.h"
#include "MeshNodeImpl.h"

namespace ramses
{
    MeshNode::MeshNode(std::unique_ptr<MeshNodeImpl> impl)
        : Node{ std::move(impl) }
        , m_impl{ static_cast<MeshNodeImpl&>(Node::m_impl) }
    {
    }

    status_t MeshNode::setAppearance(Appearance& appearance)
    {
        const status_t status = m_impl.setAppearance(appearance.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(appearance));
        return status;
    }

    status_t MeshNode::setGeometryBinding(GeometryBinding& geometry)
    {
        const status_t status = m_impl.setGeometryBinding(geometry.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(geometry));
        return status;
    }

    status_t MeshNode::removeAppearanceAndGeometry()
    {
        const status_t status = m_impl.removeAppearanceAndGeometry();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t MeshNode::setStartIndex(uint32_t startIndex)
    {
        const status_t status = m_impl.setStartIndex(startIndex);
        LOG_HL_CLIENT_API1(status, startIndex);
        return status;
    }

    status_t MeshNode::setStartVertex(uint32_t startVertex)
    {
        const status_t status = m_impl.setStartVertex(startVertex);
        LOG_HL_CLIENT_API1(status, startVertex);
        return status;
    }

    status_t MeshNode::setIndexCount(uint32_t indexCount)
    {
        const status_t status = m_impl.setIndexCount(indexCount);
        LOG_HL_CLIENT_API1(status, indexCount);
        return status;
    }

    const Appearance* MeshNode::getAppearance() const
    {
        return m_impl.getAppearance();
    }

    Appearance* MeshNode::getAppearance()
    {
        return m_impl.getAppearance();
    }

    const GeometryBinding* MeshNode::getGeometryBinding() const
    {
        return m_impl.getGeometryBinding();
    }

    GeometryBinding* MeshNode::getGeometryBinding()
    {
        return m_impl.getGeometryBinding();
    }

    uint32_t MeshNode::getStartIndex() const
    {
        return m_impl.getStartIndex();
    }

    uint32_t MeshNode::getStartVertex() const
    {
        return m_impl.getStartVertex();
    }

    uint32_t MeshNode::getIndexCount() const
    {
        return m_impl.getIndexCount();
    }

    status_t MeshNode::setInstanceCount(uint32_t instanceCount)
    {
        return m_impl.setInstanceCount(instanceCount);
    }

    uint32_t MeshNode::getInstanceCount() const
    {
        return m_impl.getInstanceCount();
    }
}
