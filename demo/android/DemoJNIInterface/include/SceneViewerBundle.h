//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "RendererBundle.h"
#include "UniformInputWrapper.h"


namespace ramses
{
    class Node;
    class RamsesClient;
    class Scene;
}

class SceneViewerBundle : public RendererBundle {
public:
    SceneViewerBundle(ANativeWindow* nativeWindow, int width, int height, const char *sceneFile);

    virtual ~SceneViewerBundle() override;

    virtual void run() override;

    ramses::Node* findNodeByName(const char *name);

    void flushScene();

    UniformInputWrapper* findUniformInput(const char* appearanceName, const char* inputName);

private:
    ramses::RamsesClient* m_client;
    ramses::Scene* m_loadedScene;
};
