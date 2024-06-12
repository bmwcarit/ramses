//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "RendererControl.h"

#include "ramses/client/ramses-client.h"

#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/RendererSceneControl.h"

#include <array>

class ISceneSetup
{
public:
    virtual ~ISceneSetup() = default;

    virtual void apply() = 0;

    [[nodiscard]] virtual uint32_t getWidth() const = 0;

    [[nodiscard]] virtual uint32_t getHeight() const = 0;

    [[nodiscard]] virtual ramses::displayBufferId_t getOffscreenBuffer() const = 0;

    [[nodiscard]] virtual ramses::TextureSampler* getTextureSampler() = 0;
};

class OffscreenSetup : public ISceneSetup
{
public:
    OffscreenSetup(ramses::internal::RendererControl& rendererControl, ramses::Scene* imguiScene, ramses::Scene* scene)
        : m_rendererControl(rendererControl)
        , m_sceneControl(rendererControl.getRenderer()->getSceneControlAPI())
        , m_imguiScene(imguiScene)
        , m_scene(scene)
        , m_width(rendererControl.getDisplayWidth())
        , m_height(rendererControl.getDisplayHeight())
    {
        m_ob = rendererControl.getRenderer()->createOffscreenBuffer(rendererControl.getDisplayId(), m_width, m_height);
        rendererControl.getRenderer()->flush();

        static const std::vector<std::byte> imgbuf(4, std::byte{ 255 });
        const std::vector<ramses::MipLevelData> mipLevelData = { imgbuf };
        auto* texture = imguiScene->createTexture2D(ramses::ETextureFormat::RGBA8, 1, 1, mipLevelData);
        m_sampler = imguiScene->createTextureSampler(
            ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp, ramses::ETextureSamplingMethod::Linear, ramses::ETextureSamplingMethod::Linear, *texture);
        imguiScene->createTextureConsumer(*m_sampler, consumerId);

        rendererControl.setupSceneState(scene->getSceneId(), ramses::RendererSceneState::Ready, m_ob, 0);
        rendererControl.setupSceneState(imguiScene->getSceneId(), ramses::RendererSceneState::Ready, {}, 256);
    }

    void apply() override
    {
        m_rendererControl.waitForSceneState(*m_imguiScene, ramses::RendererSceneState::Ready);
        m_rendererControl.waitForSceneState(*m_scene, ramses::RendererSceneState::Ready);
        m_rendererControl.waitForOffscreenBufferCreated(m_ob);

        const auto guiSceneId = m_imguiScene->getSceneId();
        m_sceneControl->linkOffscreenBuffer(m_ob, guiSceneId, consumerId);
        m_sceneControl->flush();
        m_rendererControl.waitForOffscreenBufferLinked(guiSceneId);
        m_sceneControl->setSceneState(guiSceneId, ramses::RendererSceneState::Rendered);
        m_sceneControl->setSceneState(m_scene->getSceneId(), ramses::RendererSceneState::Rendered);
        m_sceneControl->flush();

        m_rendererControl.waitForSceneState(*m_scene, ramses::RendererSceneState::Rendered);
        m_rendererControl.waitForSceneState(*m_imguiScene, ramses::RendererSceneState::Rendered);
    }

    [[nodiscard]] uint32_t getWidth() const override
    {
        return m_width;
    }

    [[nodiscard]] uint32_t getHeight() const override
    {
        return m_height;
    }

    [[nodiscard]] ramses::displayBufferId_t getOffscreenBuffer() const override
    {
        return m_ob;
    }

    [[nodiscard]] ramses::TextureSampler* getTextureSampler() override
    {
        return m_sampler;
    }

private:
    const ramses::dataConsumerId_t     consumerId{519};
    ramses::internal::RendererControl&    m_rendererControl;
    ramses::RendererSceneControl* m_sceneControl;
    ramses::Scene*                m_imguiScene;
    ramses::Scene*                m_scene;
    uint32_t m_width;
    uint32_t m_height;
    ramses::displayBufferId_t m_ob;
    ramses::TextureSampler*   m_sampler = nullptr;
};

class FramebufferSetup : public ISceneSetup
{
public:
    FramebufferSetup(ramses::internal::RendererControl& rendererControl, ramses::Scene* imguiScene, ramses::Scene* scene)
        : m_rendererControl(rendererControl)
        , m_imguiScene(imguiScene)
        , m_scene(scene)
    {
        if (scene)
        {
            rendererControl.setupSceneState(scene->getSceneId(), ramses::RendererSceneState::Rendered, 0);
        }
        rendererControl.setupSceneState(imguiScene->getSceneId(), ramses::RendererSceneState::Rendered, 256);
    }

    void apply() override
    {
        m_rendererControl.waitForSceneState(*m_imguiScene, ramses::RendererSceneState::Rendered);
        if (m_scene)
        {
            m_rendererControl.waitForSceneState(*m_scene, ramses::RendererSceneState::Rendered);
        }
    }

    [[nodiscard]] uint32_t getWidth() const override
    {
        return m_rendererControl.getDisplayWidth();
    }

    [[nodiscard]] uint32_t getHeight() const override
    {
        return m_rendererControl.getDisplayHeight();
    }

    [[nodiscard]] ramses::displayBufferId_t getOffscreenBuffer() const override
    {
        return ramses::displayBufferId_t();
    }

    [[nodiscard]] ramses::TextureSampler* getTextureSampler() override
    {
        return nullptr;
    }

private:
    ramses::internal::RendererControl&    m_rendererControl;
    ramses::Scene*                m_imguiScene;
    ramses::Scene*                m_scene;
};

