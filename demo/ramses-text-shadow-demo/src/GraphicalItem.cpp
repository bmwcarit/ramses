//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "GraphicalItem.h"

GraphicalItem::GraphicalItem(ramses::Scene& scene, ramses::RenderGroup* renderGroup)
    : m_scene(scene)
    , m_renderGroup(renderGroup)
{
}

void GraphicalItem::initOutputBuffer(uint32_t width, uint32_t height, int32_t renderOrder)
{
    m_renderGroup = m_scene.createRenderGroup();

    ramses::RenderPass& renderPass = *m_scene.createRenderPass();
    renderPass.setRenderOrder(renderOrder);
    renderPass.setClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    ramses::OrthographicCamera& camera = *m_scene.createOrthographicCamera();
    camera.setFrustum(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), 0.1f, 1.0f);
    camera.setViewport(0, 0, width, height);

    renderPass.setCamera(camera);

    renderPass.addRenderGroup(*m_renderGroup);

    m_renderBuffer = m_scene.createRenderBuffer(width,
                                                height,
                                                ramses::ERenderBufferType_Color,
                                                ramses::ERenderBufferFormat_RGBA8,
                                                ramses::ERenderBufferAccessMode_ReadWrite);

    ramses::RenderTargetDescription renderTargetDescription;
    renderTargetDescription.addRenderBuffer(*m_renderBuffer);
    ramses::RenderTarget& renderTarget = *m_scene.createRenderTarget(renderTargetDescription);

    renderPass.setRenderTarget(&renderTarget);
}

ramses::RenderBuffer* GraphicalItem::getOutputBuffer()
{
    return m_renderBuffer;
}
