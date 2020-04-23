//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <android_native_app_glue.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "RendererBundle.h"


static const char* INTERFACE_SELECTION_IP = "10.0.2.15";
static const char* DAEMON_IP = "10.0.2.2";

void handle_cmd(android_app* app, int32_t cmd)
{
    std::unique_ptr<RendererBundle>& renderer = *(static_cast<std::unique_ptr<RendererBundle>*>(app->userData));

    switch(cmd)
    {
        case APP_CMD_INIT_WINDOW:
        {
            renderer.reset(new RendererBundle(app->window, ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window), INTERFACE_SELECTION_IP, DAEMON_IP));
            renderer->connect();
            renderer->run();
            break;
        }
        case APP_CMD_TERM_WINDOW:
        {
            renderer.reset();
            break;
        }
    }
}

void android_main(struct android_app* app)
{
    std::unique_ptr<RendererBundle> renderer;
    app->onAppCmd = handle_cmd;
    app->userData = &renderer;

    int events;
    struct android_poll_source* source;
    while(1)
    {
        if (ALooper_pollAll(-1, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0)
        {
            if (source)
            {
                source->process(app, source);
            }
            if (app->destroyRequested)
            {
                return;
            }
        }
    }
}
