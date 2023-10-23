//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/RenderGroup.h"
#include "ramses/client/MeshNode.h"

// internal
#include "impl/RenderGroupImpl.h"

namespace ramses
{
    RenderGroup::RenderGroup(std::unique_ptr<internal::RenderGroupImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::RenderGroupImpl&>(SceneObject::m_impl) }
    {
    }

    bool RenderGroup::addMeshNode(const MeshNode& mesh, int32_t orderWithinGroup)
    {
        const bool status = m_impl.addMeshNode(mesh.impl(), orderWithinGroup);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(mesh), orderWithinGroup);
        return status;
    }

    bool RenderGroup::removeMeshNode(const MeshNode& mesh)
    {
        const bool status = m_impl.remove(mesh.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(mesh));
        return status;
    }

    bool RenderGroup::containsMeshNode(const MeshNode& mesh) const
    {
        return m_impl.contains(mesh.impl());
    }

    bool RenderGroup::getMeshNodeOrder(const MeshNode& mesh, int32_t& orderWithinGroup) const
    {
        return m_impl.getMeshNodeOrder(mesh.impl(), orderWithinGroup);
    }

    bool RenderGroup::addRenderGroup(const RenderGroup& renderGroup, int32_t orderWithinGroup)
    {
        const bool status = m_impl.addRenderGroup(renderGroup.m_impl, orderWithinGroup);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(renderGroup), orderWithinGroup);
        return status;
    }

    bool RenderGroup::removeRenderGroup(const RenderGroup& renderGroup)
    {
        const bool status = m_impl.remove(renderGroup.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderGroup));
        return status;
    }

    bool RenderGroup::containsRenderGroup(const RenderGroup& renderGroup) const
    {
        return m_impl.contains(renderGroup.m_impl);
    }

    bool RenderGroup::getRenderGroupOrder(const RenderGroup& renderGroup, int32_t& orderWithinGroup) const
    {
        return m_impl.getRenderGroupOrder(renderGroup.m_impl, orderWithinGroup);
    }

    bool RenderGroup::removeAllRenderables()
    {
        const bool status = m_impl.removeAllRenderables();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool RenderGroup::removeAllRenderGroups()
    {
        const bool status = m_impl.removeAllRenderGroups();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    internal::RenderGroupImpl& RenderGroup::impl()
    {
        return m_impl;
    }

    const internal::RenderGroupImpl& RenderGroup::impl() const
    {
        return m_impl;
    }
}
