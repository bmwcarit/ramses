//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/BlitPass.h"

// internal
#include "BlitPassImpl.h"

namespace ramses
{
    BlitPass::BlitPass(std::unique_ptr<BlitPassImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<BlitPassImpl&>(SceneObject::m_impl) }
    {
    }

    status_t BlitPass::setBlittingRegion(uint32_t sourceX, uint32_t sourceY, uint32_t destinationX, uint32_t destinationY, uint32_t width, uint32_t height)
    {
        const status_t status = m_impl.setBlittingRegion(sourceX, sourceY, destinationX, destinationY, width, height);
        LOG_HL_CLIENT_API6(status, sourceX, sourceY, destinationX, destinationY, width, height);
        return status;
    }

    void BlitPass::getBlittingRegion(uint32_t& sourceX, uint32_t& sourceY, uint32_t& destinationX, uint32_t& destinationY, uint32_t& width, uint32_t& height) const
    {
        m_impl.getBlittingRegion(sourceX, sourceY, destinationX, destinationY, width, height);
    }

    const RenderBuffer& BlitPass::getSourceRenderBuffer() const
    {
        return m_impl.getSourceRenderBuffer();
    }

    const RenderBuffer& BlitPass::getDestinationRenderBuffer() const
    {
        return m_impl.getDestinationRenderBuffer();
    }

    status_t BlitPass::setRenderOrder(int32_t renderOrder)
    {
        const status_t status = m_impl.setRenderOrder(renderOrder);
        LOG_HL_CLIENT_API1(status, renderOrder);
        return status;
    }

    int32_t BlitPass::getRenderOrder() const
    {
        return m_impl.getRenderOrder();
    }

    status_t BlitPass::setEnabled(bool enable)
    {
        const status_t status = m_impl.setEnabled(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool BlitPass::isEnabled() const
    {
        return m_impl.isEnabled();
    }
}
