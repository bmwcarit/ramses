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
#include "Math3d/Vector4.h"

namespace ramses
{
    RenderPass::RenderPass(RenderPassImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    RenderPass::~RenderPass()
    {
    }

    status_t RenderPass::setCamera(const Camera& camera)
    {
        const status_t status = impl.setCamera(camera.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(camera));
        return status;
    }

    const Camera* RenderPass::getCamera() const
    {
        return impl.getCamera();
    }

    Camera* RenderPass::getCamera()
    {
        return impl.getCamera();
    }

    status_t RenderPass::addRenderGroup(const RenderGroup& renderGroup, int32_t orderWithinPass)
    {
        const status_t status = impl.addRenderGroup(renderGroup.impl, orderWithinPass);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(renderGroup), orderWithinPass);
        return status;
    }

    status_t RenderPass::removeRenderGroup(const RenderGroup& renderGroup)
    {
        const status_t status = impl.remove(renderGroup.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(renderGroup));
        return status;
    }

    bool RenderPass::containsRenderGroup(const RenderGroup& renderGroup) const
    {
        return impl.contains(renderGroup.impl);
    }

    status_t RenderPass::getRenderGroupOrder(const RenderGroup& renderGroup, int32_t& orderWithinPass) const
    {
        return impl.getRenderGroupOrder(renderGroup.impl, orderWithinPass);
    }

    status_t RenderPass::removeAllRenderGroups()
    {
        const status_t status = impl.removeAllRenderGroups();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }

    status_t RenderPass::setRenderTarget(RenderTarget* renderTarget)
    {
        const status_t status = impl.setRenderTarget(renderTarget ? &renderTarget->impl : nullptr);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_PTR_STRING(renderTarget));
        return status;
    }

    const RenderTarget* RenderPass::getRenderTarget() const
    {
        return impl.getRenderTarget();
    }

    status_t RenderPass::setClearColor(float r, float g, float b, float a)
    {
        const status_t status = impl.setClearColor(ramses_internal::Vector4(r, g, b, a));
        LOG_HL_CLIENT_API4(status, r, g, b, a);
        return status;
    }

    void RenderPass::getClearColor(float &r, float &g, float &b, float &a) const
    {
        const ramses_internal::Vector4& color = impl.getClearColor();
        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a;
    }

    status_t RenderPass::setClearFlags(uint32_t clearFlags)
    {
        const status_t status = impl.setClearFlags(clearFlags);
        LOG_HL_CLIENT_API1(status, clearFlags);
        return status;
    }

    uint32_t RenderPass::getClearFlags() const
    {
        return impl.getClearFlags();
    }

    status_t RenderPass::setRenderOrder(int32_t renderOrder)
    {
        const status_t status = impl.setRenderOrder(renderOrder);
        LOG_HL_CLIENT_API1(status, renderOrder);
        return status;
    }

    int32_t RenderPass::getRenderOrder() const
    {
        return impl.getRenderOrder();
    }

    status_t RenderPass::setEnabled(bool enable)
    {
        const status_t status = impl.setEnabled(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool RenderPass::isEnabled() const
    {
        return impl.isEnabled();
    }

    status_t RenderPass::setRenderOnce(bool enable)
    {
        const status_t status = impl.setRenderOnce(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool RenderPass::isRenderOnce() const
    {
        return impl.isRenderOnce();
    }

    status_t RenderPass::retriggerRenderOnce()
    {
        const status_t status = impl.retriggerRenderOnce();
        LOG_HL_CLIENT_API_NOARG(status);
        return status;
    }
}
