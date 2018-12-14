//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECOMMANDEXECUTOR_H
#define RAMSES_SCENECOMMANDEXECUTOR_H

namespace ramses
{
    class SceneImpl;
    class ResourceFileDescriptionSet;
}

namespace ramses_internal
{
    class SceneCommandBuffer;
    class String;

    class SceneCommandExecutor
    {
    public:
        static void Execute(ramses::SceneImpl& scene, SceneCommandBuffer& commandBuffer);
        static void SendSceneAndResourceFilesViaDLT(const String& sceneDumpFileName, const ramses::ResourceFileDescriptionSet& resourceFileDescriptionSet);
    };
}

#endif
