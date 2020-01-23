//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererBundle.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-framework-api/RamsesFramework.h"


RendererBundle::RendererBundle(JNIEnv *env, jobject /*instance*/,
    jobject javaSurface, jint width, jint height, const char* interfaceSelectionIP, const char* daemonIP)
    : m_nativeWindow(ANativeWindow_fromSurface(env, javaSurface), [](ANativeWindow* ptr) { ANativeWindow_release(ptr);})
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
    displayConfig.setPerspectiveProjection(19.f, static_cast<float>(width)/static_cast<float>(height), 0.1f, 1500.f);
    displayConfig.setAndroidNativeWindow(m_nativeWindow.get());
    ramses::displayId_t displayId = m_renderer->createDisplay(displayConfig);

    m_autoShowHandler.reset(new SceneStateAutoShowEventHandler(*m_renderer, displayId));

    m_renderer->setMaximumFramerate(60);
}

RendererBundle::~RendererBundle()
{
    m_framework->destroyRenderer(*m_renderer);
    m_cancelDispatchLoop = true;
    m_dispatchEventsThread->join();
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
            m_renderer->dispatchEvents(*m_autoShowHandler);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }));
}

RendererBundle::SceneStateAutoShowEventHandler::SceneStateAutoShowEventHandler(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId)
    : m_renderer(renderer)
    , m_displayId(displayId)
{
}

void RendererBundle::SceneStateAutoShowEventHandler::scenePublished(ramses::sceneId_t sceneId)
{
    m_renderer.subscribeScene(sceneId);
    m_renderer.flush();
}

void RendererBundle::SceneStateAutoShowEventHandler::sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    if (ramses::ERendererEventResult_OK == result)
    {
        m_renderer.mapScene(m_displayId, sceneId);
        m_renderer.flush();
    }
}

void RendererBundle::SceneStateAutoShowEventHandler::sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
{
    if (ramses::ERendererEventResult_OK == result)
    {
        m_renderer.showScene(sceneId);
        m_renderer.flush();
    }
}
