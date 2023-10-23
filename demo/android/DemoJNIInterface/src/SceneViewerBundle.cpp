//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneViewerBundle.h"

#include <android/log.h>

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"


SceneViewerBundle::SceneViewerBundle(ANativeWindow* nativeWindow, int width, int height, const char* sceneFile)
: RendererBundle(nativeWindow, width, height, "127.0.0.1", "127.0.0.1")
{
    m_client = m_framework->createClient("client-scene-reader");

    m_loadedScene = m_client->loadSceneFromFile(sceneFile);
    if (m_loadedScene == nullptr)
    {
        __android_log_print(ANDROID_LOG_ERROR, "RamsesNativeInterface", "Loading scene failed!");
    }
}

SceneViewerBundle::~SceneViewerBundle()
{
};

void SceneViewerBundle::run()
{
    m_loadedScene->publish();
    m_loadedScene->flush();
    RendererBundle::run();
}

ramses::Node* SceneViewerBundle::findNodeByName(const char* name)
{
    return m_loadedScene->findObject<ramses::Node>(name);
}

void SceneViewerBundle::flushScene()
{
    m_loadedScene->flush();
}

UniformInputWrapper* SceneViewerBundle::findUniformInput(const char* appearanceName, const char* inputName)
{
    auto* appearance = m_loadedScene->findObject<ramses::Appearance>(appearanceName);
    if (appearance)
    {
        return new UniformInputWrapper(inputName, *appearance);
    }
    return nullptr;
}
