//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERBUNDLE_H
#define RAMSES_RENDERERBUNDLE_H

#include <jni.h>
#include <thread>
#include <atomic>
#include <memory>
#include "ramses-renderer-api/IRendererSceneControlEventHandler_legacy.h"

namespace ramses
{
    class RamsesFramework;
    class RamsesRenderer;
    class RendererSceneControl_legacy;
}

struct ANativeWindow;

class RAMSES_API_EXPORT RendererBundle
{
public:
    RendererBundle(ANativeWindow* nativeWindow, int width, int height,
                  const char* interfaceSelectionIP, const char* daemonIP);

    virtual ~RendererBundle();
    virtual void connect();
    virtual void run();
    ANativeWindow* getNativeWindow();

    class SceneStateAutoShowEventHandler : public ramses::RendererSceneControlEventHandlerEmpty_legacy
    {
    public:
        SceneStateAutoShowEventHandler(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId);

        virtual void scenePublished(ramses::sceneId_t sceneId) override;
        virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;
        virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;

    private:
        ramses::RendererSceneControl_legacy& m_sceneControlAPI;
        ramses::displayId_t m_displayId;
    };

protected:
    ANativeWindow* m_nativeWindow;
    std::unique_ptr<ramses::RamsesFramework> m_framework;
    ramses::RamsesRenderer* m_renderer;
    std::unique_ptr<SceneStateAutoShowEventHandler> m_autoShowHandler;
    std::unique_ptr<std::thread> m_dispatchEventsThread;
    std::atomic<bool> m_cancelDispatchLoop;
};

#endif
