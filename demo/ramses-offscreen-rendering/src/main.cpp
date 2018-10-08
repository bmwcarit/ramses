//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TriangleScene.h"
#include "OffscreenRenderer.h"
#include <chrono>

uint64_t GetTimestampMsec()
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

int main(int argc, char* argv[])
{
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient client("ramses-offscreen-rendering", framework);
    OffscreenRenderer renderer(framework, argc, argv);
    framework.connect();

    const ramses::sceneId_t remoteSceneId   = 1u;
    const ramses::sceneId_t localSceneId    = 2u;
    TriangleScene remoteScene   = CreateTriangleScene(client, remoteSceneId , { 1.0f, 0.0f, 0.3f }, ramses::EScenePublicationMode_LocalAndRemote);
    TriangleScene localScene    = CreateTriangleScene(client, localSceneId  , { 0.0f, 1.0f, 0.3f }, ramses::EScenePublicationMode_LocalOnly);

    // Uncomment this line to also put the remote scene in the screenshot
    //renderer.getSceneToRenderedState(remoteSceneId);
    renderer.getSceneToRenderedState(localSceneId);

    ramses::sceneVersionTag_t localSceneVersion = 0;
    uint64_t lastScreenshotTime = GetTimestampMsec();
    for (;;)
    {
        const uint64_t currentTime = GetTimestampMsec();

        // Continuously animate the remote scene
        remoteScene.rotateNode->setRotation(currentTime / 10.0f, 0.0f, 0.0f);
        remoteScene.scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);

        // Update state and take a screenshot of the local scene every second
        if (currentTime - lastScreenshotTime > 1000u)
        {
            ++localSceneVersion;
            localScene.rotateNode->setRotation(0.0f, 0.0f, currentTime / 10.0f);
            localScene.scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources, localSceneVersion);
            renderer.takeScreenshotOfScene(localSceneId, localSceneVersion);
            lastScreenshotTime = currentTime;
        }
    }
}
