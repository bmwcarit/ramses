//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneViewerBundle.h"

#include <android/log.h>

#include "ramses-client.h"
#include "ramses-utils.h"


SceneViewerBundle::SceneViewerBundle(ANativeWindow* nativeWindow, int width, int height, const char* sceneFile, const char* resFile)
: RendererBundle(nativeWindow, width, height, "127.0.0.1", "127.0.0.1")
{
    m_client = m_framework->createClient("client-scene-reader");

    ramses::ResourceFileDescriptionSet resourceFileInformation;
    resourceFileInformation.add(ramses::ResourceFileDescription(resFile));
    m_loadedScene = m_client->loadSceneFromFile(sceneFile, resourceFileInformation);
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
    ramses::RamsesObject* nodeObject = m_loadedScene->findObjectByName(name);
    if (nodeObject)
    {
        return ramses::RamsesUtils::TryConvert<ramses::Node>(*nodeObject);
    }
    return nullptr;
}

void SceneViewerBundle::flushScene()
{
    m_loadedScene->flush();
}

UniformInputWrapper* SceneViewerBundle::findUniformInput(const char* appearanceName, const char* inputName)
{
    ramses::RamsesObject* appearanceObj = m_loadedScene->findObjectByName(appearanceName);
    if (appearanceObj)
    {
        ramses::Appearance* appearance = ramses::RamsesUtils::TryConvert<ramses::Appearance>(*appearanceObj);
        if (appearance)
        {
                return new UniformInputWrapper(inputName, *appearance);
        }
    }
    return nullptr;
}
