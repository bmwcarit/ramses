//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-demoLib/GraphicalItem.h"

#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderBuffer.h"

GraphicalItem::GraphicalItem(ramses::Scene& scene, ramses::RamsesClient& client, ramses::RenderGroup* renderGroup)
    : m_scene(scene)
    , m_client(client)
    , m_renderGroup(renderGroup)
{
}

GraphicalItem::~GraphicalItem()
{
    for (int32_t i = static_cast<int>(m_sceneObjects.size() - 1); i >= 0; i--)
    {
        m_scene.destroy(*m_sceneObjects[i]);
    }

    for (int32_t i = static_cast<int>(m_clientResources.size() - 1); i >= 0; i--)
    {
        m_client.destroy(*m_clientResources[i]);
    }
}

void GraphicalItem::initOutputBuffer(uint32_t width, uint32_t height, int32_t renderOrder)
{
    m_renderGroup = m_scene.createRenderGroup();
    m_sceneObjects.push_back(m_renderGroup);

    m_camera = m_scene.createOrthographicCamera();
    m_sceneObjects.push_back(m_camera);

    m_renderBuffer = m_scene.createRenderBuffer(width,
                                                    height,
                                                    ramses::ERenderBufferType_Color,
                                                    ramses::ERenderBufferFormat_RGBA8,
                                                    ramses::ERenderBufferAccessMode_ReadWrite);
    m_sceneObjects.push_back(m_renderBuffer);

    ramses::RenderTargetDescription renderTargetDescription;
    renderTargetDescription.addRenderBuffer(*m_renderBuffer);
    m_renderTarget = m_scene.createRenderTarget(renderTargetDescription);
    m_sceneObjects.push_back(m_renderTarget);

    m_renderPass = m_scene.createRenderPass();
    m_sceneObjects.push_back(m_renderPass);
    m_renderPass->setRenderOrder(renderOrder);
    m_renderPass->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);


    m_camera->setFrustum(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), 0.1f, 1.0f);
    m_camera->setViewport(0, 0, width, height);

    m_renderPass->setCamera(*m_camera);

    m_renderPass->addRenderGroup(*m_renderGroup);

    m_renderPass->setRenderTarget(m_renderTarget);
    m_renderPass->setRenderOnce(true);
}

ramses::RenderBuffer* GraphicalItem::getOutputBuffer()
{
    return m_renderBuffer;
}
