//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererBundle.h"

#include <android/native_window.h>
#include <android/log.h>

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-framework-api/RamsesFramework.h"


RendererBundle::RendererBundle(ANativeWindow* nativeWindow, int width, int height,
                                                 const char* interfaceSelectionIP, const char* daemonIP)
    : m_nativeWindow(nativeWindow)
    , m_cancelDispatchLoop(false)
{
    //workaround to ensure that the socket connection is always initiated outbound from Android, inbound connections might be blocked
    const char* argv[] = {"", "--guid", "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"};
    ramses::RamsesFrameworkConfig frameworkConfig(3, argv);
    frameworkConfig.setInterfaceSelectionIPForTCPCommunication(interfaceSelectionIP);
    frameworkConfig.setDaemonIPForTCPCommunication(daemonIP);
    m_framework.reset(new ramses::RamsesFramework(frameworkConfig));
    ramses::RendererConfig rendererConfig;
    m_renderer = m_framework->createRenderer(rendererConfig);

    ramses::DisplayConfig displayConfig;
    displayConfig.setWindowRectangle(0, 0, width, height);
    displayConfig.setAndroidNativeWindow(m_nativeWindow);
    ramses::displayId_t displayId = m_renderer->createDisplay(displayConfig);

    m_autoShowHandler.reset(new SceneStateAutoShowEventHandler(*m_renderer, displayId));

    m_renderer->setMaximumFramerate(60);
    m_renderer->flush();
}

RendererBundle::~RendererBundle()
{
    m_cancelDispatchLoop = true;
    m_dispatchEventsThread->join();
    m_framework->destroyRenderer(*m_renderer);
};

void RendererBundle::connect()
{
    m_framework->connect();
}

void RendererBundle::run()
{
    m_renderer->startThread();
    m_dispatchEventsThread.reset(new std::thread([this]
    {
        while (m_cancelDispatchLoop == false)
        {
            m_renderer->getSceneControlAPI()->dispatchEvents(*m_autoShowHandler);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }));
}

RendererBundle::SceneStateAutoShowEventHandler::SceneStateAutoShowEventHandler(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId)
    : m_sceneControlAPI(*renderer.getSceneControlAPI())
    , m_displayId(displayId)
{
}

void RendererBundle::SceneStateAutoShowEventHandler::sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
{
    if (state == ramses::RendererSceneState::Available)
    {
        m_sceneControlAPI.setSceneMapping(sceneId, m_displayId);
        m_sceneControlAPI.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
        m_sceneControlAPI.flush();
    }
}

ANativeWindow* RendererBundle::getNativeWindow()
{
    return m_nativeWindow;
}
