//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEVIEWERBUNDLE_H
#define RAMSES_SCENEVIEWERBUNDLE_H

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
    SceneViewerBundle(ANativeWindow* nativeWindow, int width, int height,
                                 const char *sceneFile, const char *resFile);

    virtual ~SceneViewerBundle();

    virtual void run() override;

    ramses::Node* findNodeByName(const char *name);

    void flushScene();

    UniformInputWrapper* findUniformInput(const char* appearanceName, const char* inputName);

private:
    ramses::RamsesClient* m_client;
    ramses::Scene* m_loadedScene;
};

#endif
