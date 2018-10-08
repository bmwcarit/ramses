//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEMO_OFFSCREENRENDERER_H
#define RAMSES_DEMO_OFFSCREENRENDERER_H

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "lodepng.h"

#include <sstream>
#include <cmath>
#include <map>
#include <assert.h>

class OffscreenRenderer : public ramses::RendererEventHandlerEmpty
{
public:
    OffscreenRenderer(ramses::RamsesFramework& framework, int argc, char* argv[])
        : m_renderer(framework, ramses::RendererConfig(argc, argv))
    {
        ramses::DisplayConfig displayConfig(argc, argv);
        displayConfig.setWindowRectangle(0, 0, ScreenWidth, ScreenHeight);
        displayConfig.setOffscreen(true);
        m_display = m_renderer.createDisplay(displayConfig);
    }

    void getSceneToRenderedState(ramses::sceneId_t sceneToCapture)
    {
        // since the sceneToCapture is a local scene, this will return immediately
        waitForPublication(sceneToCapture);

        m_renderer.subscribeScene(sceneToCapture);
        m_renderer.flush();
        waitForSubscription(sceneToCapture);

        m_renderer.mapScene(m_display, sceneToCapture);
        m_renderer.flush();
        waitForMapped(sceneToCapture);

        m_renderer.showScene(sceneToCapture);
        m_renderer.flush();
        waitForShown(sceneToCapture);
    }

    void takeScreenshotOfScene(ramses::sceneId_t sceneToCapture, ramses::sceneVersionTag_t sceneVersion)
    {
        m_sceneVersionForScreenshot = sceneVersion;
        waitForSceneVersion(sceneToCapture, sceneVersion);

        m_renderer.readPixels(m_display, 0, 0, ScreenWidth, ScreenHeight);
        m_renderer.flush();
        waitForScreenshot();
    }

private:
    enum class ESceneState
    {
        Published = 0,
        Subscribed,
        Mapped,
        Rendered
    };

    void waitForSceneVersion(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion)
    {
        while (m_sceneVersions.count(sceneId) == 0 || m_sceneVersions[sceneId] != sceneVersion)
        {
            loopOnceAndFetchEvents();
        }
    }

    void waitForPublication(const ramses::sceneId_t sceneId)
    {
        waitForSceneState(sceneId, ESceneState::Published);
    }

    void waitForSubscription(const ramses::sceneId_t sceneId)
    {
        waitForSceneState(sceneId, ESceneState::Subscribed);
    }

    void waitForMapped(const ramses::sceneId_t sceneId)
    {
        waitForSceneState(sceneId, ESceneState::Mapped);
    }

    void waitForShown(const ramses::sceneId_t sceneId)
    {
        waitForSceneState(sceneId, ESceneState::Rendered);
    }

    void waitForScreenshot()
    {
        m_screenshotPixelsArrived = false;
        while (!m_screenshotPixelsArrived)
        {
            loopOnceAndFetchEvents();
        }
    }

    void loopOnceAndFetchEvents()
    {
        m_renderer.doOneLoop();
        m_renderer.dispatchEvents(*this);
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override final
    {
        assert(0u == m_sceneStates.count(sceneId));
        m_sceneStates[sceneId] = ESceneState::Published;
    }

    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_sceneStates[sceneId] = ESceneState::Subscribed;
        }
    }

    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_sceneStates[sceneId] = ESceneState::Mapped;
        }
    }

    virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_sceneStates[sceneId] = ESceneState::Rendered;
        }
    }

    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override final
    {
        m_sceneStates.erase(sceneId);
        m_sceneVersions.erase(sceneId);
    }

    virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult) override final
    {
        m_sceneStates[sceneId] = ESceneState::Published;
    }

    virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult) override final
    {
        m_sceneStates[sceneId] = ESceneState::Subscribed;
    }

    virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult) override final
    {
        m_sceneStates[sceneId] = ESceneState::Mapped;
    }

    virtual void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion, ramses::ESceneResourceStatus) override final
    {
        m_sceneVersions[sceneId] = sceneVersion;
    }

    virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, ramses::displayId_t display, ramses::ERendererEventResult result) override final
    {
        // In this demo the check is irrelevant; But a proper application should check the values correctness
        if (result != ramses::ERendererEventResult_OK || pixelDataSize != ScreenWidth * ScreenHeight * 4 || display != m_display)
        {
            assert(false && "Screenshot capture failed!");
        }

        // Store the screenshot as "screenshotX.png" file
        std::stringstream screenshotFileName;
        screenshotFileName << "screenshot" << m_sceneVersionForScreenshot << ".png";
        std::vector<unsigned char> pixelDataVec(pixelData, pixelData+pixelDataSize);
        lodepng::encode(screenshotFileName.str(), pixelDataVec, ScreenWidth, ScreenHeight);
        m_screenshotPixelsArrived = true;
    }

    void waitForSceneState(const ramses::sceneId_t sceneId, ESceneState state)
    {
        while (m_sceneStates.find(sceneId) == m_sceneStates.end() || m_sceneStates[sceneId] != state)
        {
            m_renderer.doOneLoop();
            m_renderer.dispatchEvents(*this);
        }
    }

    ramses::RamsesRenderer m_renderer;
    const uint32_t ScreenWidth = 1280;
    const uint32_t ScreenHeight = 640;
    ramses::displayId_t m_display;

    typedef std::map<ramses::sceneId_t, ESceneState> SceneStates;
    SceneStates m_sceneStates;

    typedef std::map<ramses::sceneId_t, ramses::sceneVersionTag_t> SceneVersions;
    SceneVersions m_sceneVersions;

    ramses::sceneVersionTag_t m_sceneVersionForScreenshot;
    bool m_screenshotPixelsArrived = false;

};

#endif
