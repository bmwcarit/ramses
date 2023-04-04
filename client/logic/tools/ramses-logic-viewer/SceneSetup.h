//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ImguiClientHelper.h"

#include "ramses-client.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/RendererSceneControl.h"

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
    OffscreenSetup(rlogic::ImguiClientHelper& imguiHelper, ramses::RamsesRenderer& renderer, ramses::Scene* scene, ramses::displayId_t display, uint32_t width, uint32_t height)
        : m_imguiHelper(imguiHelper)
        , m_sceneControl(renderer.getSceneControlAPI())
        , m_scene(scene)
        , m_width(width)
        , m_height(height)
    {
        m_ob = renderer.createOffscreenBuffer(display, width, height);
        renderer.flush();

        static const std::array<uint8_t, 4> imgbuf = {255, 255, 255, 255};
        ramses::MipLevelData mipLevelData(4, imgbuf.data());
        auto* texture = imguiHelper.getScene()->createTexture2D(ramses::ETextureFormat::RGBA8, 1, 1, 1, &mipLevelData);
        m_sampler = imguiHelper.getScene()->createTextureSampler(
            ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp, ramses::ETextureSamplingMethod_Linear, ramses::ETextureSamplingMethod_Linear, *texture);

        const auto guiSceneId = imguiHelper.getScene()->getSceneId();
        m_sceneControl->setSceneMapping(scene->getSceneId(), display);
        m_sceneControl->setSceneMapping(guiSceneId, display);
        m_sceneControl->setSceneState(scene->getSceneId(), ramses::RendererSceneState::Ready);
        m_sceneControl->setSceneState(guiSceneId, ramses::RendererSceneState::Ready);
        m_sceneControl->flush();
    }

    void apply() override
    {
        const auto guiSceneId = m_imguiHelper.getScene()->getSceneId();
        const ramses::dataConsumerId_t consumerId(519);

        m_imguiHelper.getScene()->createTextureConsumer(*m_sampler, consumerId);
        m_imguiHelper.getScene()->flush(42);
        m_imguiHelper.waitForSceneVersion(guiSceneId, 42);
        m_imguiHelper.waitForOffscreenBufferCreated(m_ob);

        m_imguiHelper.waitForSceneState(guiSceneId, ramses::RendererSceneState::Ready);
        m_imguiHelper.waitForSceneState(m_scene->getSceneId(), ramses::RendererSceneState::Ready);

        m_sceneControl->setSceneDisplayBufferAssignment(m_scene->getSceneId(), m_ob);
        m_sceneControl->linkOffscreenBuffer(m_ob, guiSceneId, consumerId);
        m_sceneControl->flush();
        m_imguiHelper.waitForOffscreenBufferLinked(guiSceneId);
        m_sceneControl->setSceneState(guiSceneId, ramses::RendererSceneState::Rendered);
        m_sceneControl->setSceneState(m_scene->getSceneId(), ramses::RendererSceneState::Rendered);
        m_sceneControl->flush();

        m_imguiHelper.waitForSceneState(m_scene->getSceneId(), ramses::RendererSceneState::Rendered);
        m_imguiHelper.waitForSceneState(guiSceneId, ramses::RendererSceneState::Rendered);
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
    rlogic::ImguiClientHelper&    m_imguiHelper;
    ramses::RendererSceneControl* m_sceneControl;
    ramses::Scene*                m_scene;
    uint32_t m_width;
    uint32_t m_height;
    ramses::displayBufferId_t m_ob;
    ramses::TextureSampler*   m_sampler = nullptr;
};

class FramebufferSetup : public ISceneSetup
{
public:
    FramebufferSetup(rlogic::ImguiClientHelper& imguiHelper, ramses::RamsesRenderer& renderer, ramses::Scene* scene, ramses::displayId_t display)
        : m_imguiHelper(imguiHelper)
        , m_sceneControl(renderer.getSceneControlAPI())
        , m_scene(scene)
    {
        const auto guiSceneId = imguiHelper.getScene()->getSceneId();
        m_sceneControl->setSceneMapping(scene->getSceneId(), display);
        m_sceneControl->setSceneMapping(guiSceneId, display);
        m_sceneControl->setSceneState(scene->getSceneId(), ramses::RendererSceneState::Rendered);
        m_sceneControl->flush();
    }

    void apply() override
    {
        m_imguiHelper.waitForSceneState(m_scene->getSceneId(), ramses::RendererSceneState::Rendered);
        const auto guiSceneId = m_imguiHelper.getScene()->getSceneId();
        m_sceneControl->setSceneState(guiSceneId, ramses::RendererSceneState::Rendered);
        m_sceneControl->flush();
    }

    [[nodiscard]] uint32_t getWidth() const override
    {
        return m_imguiHelper.getViewportWidth();
    }

    [[nodiscard]] uint32_t getHeight() const override
    {
        return m_imguiHelper.getViewportHeight();
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
    rlogic::ImguiClientHelper&    m_imguiHelper;
    ramses::RendererSceneControl* m_sceneControl;
    ramses::Scene*                m_scene;
};

