//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/RenderGroup.h"

// internal
#include "RenderPassImpl.h"

namespace ramses
{
    RenderPass::RenderPass(std::unique_ptr<RenderPassImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<RenderPassImpl&>(SceneObject::m_impl) }
    {
    }

    status_t RenderPass::setCamera(const Camera& camera)
    {
        const status_t status = m_impl.setCamera(camera.m_impl);
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

    status_t RenderPass::addRenderGroup(const RenderGroup& renderGroup, int32_t orderWithinPass)
    {
        const status_t status = m_impl.addRenderGroup(renderGroup.m_impl, orderWithinPass);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(renderGroup), orderWithinPass);
        return status;
    }

    status_t RenderPass::removeRenderGroup(const RenderGroup& renderGroup)
    {
        const status_t status = m_impl.remove(renderGroup.m_impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderGroup));
        return status;
    }

    bool RenderPass::containsRenderGroup(const RenderGroup& renderGroup) const
    {
        return m_impl.contains(renderGroup.m_impl);
    }

    status_t RenderPass::getRenderGroupOrder(const RenderGroup& renderGroup, int32_t& orderWithinPass) const
    {
        return m_impl.getRenderGroupOrder(renderGroup.m_impl, orderWithinPass);
    }

    status_t RenderPass::removeAllRenderGroups()
    {
        const status_t status = m_impl.removeAllRenderGroups();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t RenderPass::setRenderTarget(RenderTarget* renderTarget)
    {
        const status_t status = m_impl.setRenderTarget(renderTarget ? &renderTarget->m_impl : nullptr);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_PTR_STRING(renderTarget));
        return status;
    }

    const RenderTarget* RenderPass::getRenderTarget() const
    {
        return m_impl.getRenderTarget();
    }

    status_t RenderPass::setClearColor(const vec4f& color)
    {
        const status_t status = m_impl.setClearColor(color);
        LOG_HL_CLIENT_API4(status, color.r, color.g, color.b, color.a);
        return status;
    }

    vec4f RenderPass::getClearColor() const
    {
        return m_impl.getClearColor();
    }

    status_t RenderPass::setClearFlags(uint32_t clearFlags)
    {
        const status_t status = m_impl.setClearFlags(clearFlags);
        LOG_HL_CLIENT_API1(status, clearFlags);
        return status;
    }

    uint32_t RenderPass::getClearFlags() const
    {
        return m_impl.getClearFlags();
    }

    status_t RenderPass::setRenderOrder(int32_t renderOrder)
    {
        const status_t status = m_impl.setRenderOrder(renderOrder);
        LOG_HL_CLIENT_API1(status, renderOrder);
        return status;
    }

    int32_t RenderPass::getRenderOrder() const
    {
        return m_impl.getRenderOrder();
    }

    status_t RenderPass::setEnabled(bool enable)
    {
        const status_t status = m_impl.setEnabled(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool RenderPass::isEnabled() const
    {
        return m_impl.isEnabled();
    }

    status_t RenderPass::setRenderOnce(bool enable)
    {
        const status_t status = m_impl.setRenderOnce(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool RenderPass::isRenderOnce() const
    {
        return m_impl.isRenderOnce();
    }

    status_t RenderPass::retriggerRenderOnce()
    {
        const status_t status = m_impl.retriggerRenderOnce();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }
}
