//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"

// internal
#include "impl/NodeImpl.h"
#include "impl/MeshNodeImpl.h"

namespace ramses
{
    MeshNode::MeshNode(std::unique_ptr<internal::MeshNodeImpl> impl)
        : Node{ std::move(impl) }
        , m_impl{ static_cast<internal::MeshNodeImpl&>(Node::m_impl) }
    {
    }

    bool MeshNode::setAppearance(Appearance& appearance)
    {
        const bool status = m_impl.setAppearance(appearance.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(appearance));
        return status;
    }

    bool MeshNode::setGeometry(Geometry& geometry)
    {
        const bool status = m_impl.setGeometry(geometry.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(geometry));
        return status;
    }

    bool MeshNode::removeAppearanceAndGeometry()
    {
        const bool status = m_impl.removeAppearanceAndGeometry();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool MeshNode::setStartIndex(uint32_t startIndex)
    {
        const bool status = m_impl.setStartIndex(startIndex);
        LOG_HL_CLIENT_API1(status, startIndex);
        return status;
    }

    bool MeshNode::setStartVertex(uint32_t startVertex)
    {
        const bool status = m_impl.setStartVertex(startVertex);
        LOG_HL_CLIENT_API1(status, startVertex);
        return status;
    }

    bool MeshNode::setIndexCount(uint32_t indexCount)
    {
        const bool status = m_impl.setIndexCount(indexCount);
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

    const Geometry* MeshNode::getGeometry() const
    {
        return m_impl.getGeometry();
    }

    Geometry* MeshNode::getGeometry()
    {
        return m_impl.getGeometry();
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

    bool MeshNode::setInstanceCount(uint32_t instanceCount)
    {
        return m_impl.setInstanceCount(instanceCount);
    }

    uint32_t MeshNode::getInstanceCount() const
    {
        return m_impl.getInstanceCount();
    }

    internal::MeshNodeImpl& MeshNode::impl()
    {
        return m_impl;
    }

    const internal::MeshNodeImpl& MeshNode::impl() const
    {
        return m_impl;
    }
}
