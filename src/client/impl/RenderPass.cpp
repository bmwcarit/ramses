//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/Camera.h"
#include "ramses/client/RenderGroup.h"
#include "impl/RamsesFrameworkTypesImpl.h"

// internal
#include "impl/RenderPassImpl.h"

namespace ramses
{
    RenderPass::RenderPass(std::unique_ptr<internal::RenderPassImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::RenderPassImpl&>(SceneObject::m_impl) }
    {
    }

    bool RenderPass::setCamera(const Camera& camera)
    {
        const bool status = m_impl.setCamera(camera.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(camera));
        return status;
    }

    const Camera* RenderPass::getCamera() const
    {
        return m_impl.getCamera();
    }

    Camera* RenderPass::getCamera()
    {
        return m_impl.getCamera();
    }

    bool RenderPass::addRenderGroup(const RenderGroup& renderGroup, int32_t orderWithinPass)
    {
        const bool status = m_impl.addRenderGroup(renderGroup.impl(), orderWithinPass);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(renderGroup), orderWithinPass);
        return status;
    }

    bool RenderPass::removeRenderGroup(const RenderGroup& renderGroup)
    {
        const bool status = m_impl.remove(renderGroup.impl());
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderGroup));
        return status;
    }

    bool RenderPass::containsRenderGroup(const RenderGroup& renderGroup) const
    {
        return m_impl.contains(renderGroup.impl());
    }

    bool RenderPass::getRenderGroupOrder(const RenderGroup& renderGroup, int32_t& orderWithinPass) const
    {
        return m_impl.getRenderGroupOrder(renderGroup.impl(), orderWithinPass);
    }

    bool RenderPass::removeAllRenderGroups()
    {
        const bool status = m_impl.removeAllRenderGroups();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    bool RenderPass::setRenderTarget(RenderTarget* renderTarget)
    {
        const bool status = m_impl.setRenderTarget(renderTarget ? &renderTarget->impl() : nullptr);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_PTR_STRING(renderTarget));
        return status;
    }

    const RenderTarget* RenderPass::getRenderTarget() const
    {
        return m_impl.getRenderTarget();
    }

    bool RenderPass::setClearColor(const vec4f& color)
    {
        const bool status = m_impl.setClearColor(color);
        LOG_HL_CLIENT_API4(status, color.r, color.g, color.b, color.a);
        return status;
    }

    vec4f RenderPass::getClearColor() const
    {
        return m_impl.getClearColor();
    }

    bool RenderPass::setClearFlags(ClearFlags clearFlags)
    {
        const bool status = m_impl.setClearFlags(clearFlags);
        LOG_HL_CLIENT_API1(status, clearFlags);
        return status;
    }

    ClearFlags RenderPass::getClearFlags() const
    {
        return m_impl.getClearFlags();
    }

    bool RenderPass::setRenderOrder(int32_t renderOrder)
    {
        const bool status = m_impl.setRenderOrder(renderOrder);
        LOG_HL_CLIENT_API1(status, renderOrder);
        return status;
    }

    int32_t RenderPass::getRenderOrder() const
    {
        return m_impl.getRenderOrder();
    }

    bool RenderPass::setEnabled(bool enable)
    {
        const bool status = m_impl.setEnabled(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool RenderPass::isEnabled() const
    {
        return m_impl.isEnabled();
    }

    bool RenderPass::setRenderOnce(bool enable)
    {
        const bool status = m_impl.setRenderOnce(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool RenderPass::isRenderOnce() const
    {
        return m_impl.isRenderOnce();
    }

    bool RenderPass::retriggerRenderOnce()
    {
        const bool status = m_impl.retriggerRenderOnce();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    internal::RenderPassImpl& RenderPass::impl()
    {
        return m_impl;
    }

    const internal::RenderPassImpl& RenderPass::impl() const
    {
        return m_impl;
    }
}
