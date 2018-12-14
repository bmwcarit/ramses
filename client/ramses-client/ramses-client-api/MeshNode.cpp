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
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/GeometryBinding.h"

// internal
#include "NodeImpl.h"
#include "MeshNodeImpl.h"

namespace ramses
{
    MeshNode::MeshNode(MeshNodeImpl& pimpl)
        : Node(pimpl)
        , impl(pimpl)
    {
    }

    MeshNode::~MeshNode()
    {
    }

    status_t MeshNode::setAppearance(Appearance& appearance)
    {
        const status_t status = impl.setAppearance(appearance.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(appearance));
        return status;
    }

    status_t MeshNode::setGeometryBinding(GeometryBinding& geometry)
    {
        const status_t status = impl.setGeometryBinding(geometry.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(geometry));
        return status;
    }

    status_t MeshNode::removeAppearanceAndGeometry()
    {
        const status_t status = impl.removeAppearanceAndGeometry();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t MeshNode::setStartIndex(uint32_t startIndex)
    {
        const status_t status = impl.setStartIndex(startIndex);
        LOG_HL_CLIENT_API1(status, startIndex);
        return status;
    }

    status_t MeshNode::setIndexCount(uint32_t indexCount)
    {
        const status_t status = impl.setIndexCount(indexCount);
        LOG_HL_CLIENT_API1(status, indexCount);
        return status;
    }

    const Appearance* MeshNode::getAppearance() const
    {
        return impl.getAppearance();
    }

    Appearance* MeshNode::getAppearance()
    {
        return impl.getAppearance();
    }

    const GeometryBinding* MeshNode::getGeometryBinding() const
    {
        return impl.getGeometryBinding();
    }

    GeometryBinding* MeshNode::getGeometryBinding()
    {
        return impl.getGeometryBinding();
    }

    uint32_t MeshNode::getStartIndex() const
    {
        return impl.getStartIndex();
    }

    uint32_t MeshNode::getIndexCount() const
    {
        return impl.getIndexCount();
    }

    status_t MeshNode::setInstanceCount(uint32_t instanceCount)
    {
        return impl.setInstanceCount(instanceCount);
    }

    uint32_t MeshNode::getInstanceCount() const
    {
        return impl.getInstanceCount();
    }
}
