//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererBundle.h"

#include <thread>
#include <atomic>
#include <memory>
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-framework-api/RamsesFramework.h"

class SceneStateAutoShowEventHandler : public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    SceneStateAutoShowEventHandler(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId);

    virtual void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override;

private:
    ramses::RendererSceneControl& m_sceneControlAPI;
    ramses::displayId_t m_displayId;
};

SceneStateAutoShowEventHandler::SceneStateAutoShowEventHandler(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId)
    : m_sceneControlAPI(*renderer.getSceneControlAPI())
    , m_displayId(displayId)
{
}

void SceneStateAutoShowEventHandler::sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
{
    if (state == ramses::RendererSceneState::Available)
    {
        m_sceneControlAPI.setSceneMapping(sceneId, m_displayId);
        m_sceneControlAPI.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
        m_sceneControlAPI.flush();
    }
}


@implementation RendererBundle {
    CAMetalLayer* m_nativeWindow;
    std::unique_ptr<ramses::RamsesFramework> m_framework;
    ramses::RamsesRenderer* m_renderer;
    std::unique_ptr<SceneStateAutoShowEventHandler> m_autoShowHandler;
    std::unique_ptr<std::thread> m_dispatchEventsThread;
    std::atomic<bool> m_cancelDispatchLoop;
}


- (instancetype)initWithMetalLayer:(CAMetalLayer *)metalLayer width:(int)width height:(int)height interfaceSelectionIP:(NSString*)interfaceSelectionIP daemonIP:(NSString*)daemonIP {
    self = [super init];
    m_nativeWindow = metalLayer;
    
    const char* argv[] = {"", "--guid", "FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"};
    ramses::RamsesFrameworkConfig frameworkConfig(3, argv);
    frameworkConfig.setInterfaceSelectionIPForTCPCommunication([interfaceSelectionIP cStringUsingEncoding:kCFStringEncodingUTF8]);
    frameworkConfig.setDaemonIPForTCPCommunication([daemonIP cStringUsingEncoding:kCFStringEncodingUTF8]);
    m_framework.reset(new ramses::RamsesFramework(frameworkConfig));
    ramses::RendererConfig rendererConfig;
    m_renderer = m_framework->createRenderer(rendererConfig);

    ramses::DisplayConfig displayConfig;
    displayConfig.setWindowRectangle(0, 0, width, height);
    displayConfig.setIOSNativeWindow(m_nativeWindow);
    ramses::displayId_t displayId = m_renderer->createDisplay(displayConfig);

    m_autoShowHandler.reset(new SceneStateAutoShowEventHandler(*m_renderer, displayId));

    m_renderer->setMaximumFramerate(60);
    m_renderer->flush();
    return self;
}

-(void)dealloc {
    m_cancelDispatchLoop = true;
    m_dispatchEventsThread->join();
    m_framework->destroyRenderer(*m_renderer);
    [super dealloc];
}

-(void)connect {
    m_framework->connect();
}

-(void) run {
    m_renderer->startThread();
    m_dispatchEventsThread.reset(new std::thread([self]
    {
        while (m_cancelDispatchLoop == false)
        {
            m_renderer->getSceneControlAPI()->dispatchEvents(*m_autoShowHandler);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }));
}

-(CAMetalLayer*) getNativeWindow {
    return m_nativeWindow;
}

@end
