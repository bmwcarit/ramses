//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_SCENECOMMANDVISITOR_H
#define RAMSES_SCENECOMMANDVISITOR_H

namespace ramses
{
    class SceneImpl;
    class ResourceFileDescriptionSet;
}

namespace ramses_internal
{
    struct SceneCommandFlushSceneVersion;
    struct SceneCommandValidationRequest;
    struct SceneCommandDumpSceneToFile;
    struct SceneCommandLogResourceMemoryUsage;
    class String;

    class SceneCommandVisitor
    {
    public:
        explicit SceneCommandVisitor(ramses::SceneImpl& scene)
            : m_scene(scene)
        {}

        void operator()(const SceneCommandFlushSceneVersion& cmd);
        void operator()(const SceneCommandValidationRequest& cmd);
        void operator()(const SceneCommandDumpSceneToFile& cmd) const;
        void operator()(const SceneCommandLogResourceMemoryUsage& cmd) const;

    private:
        static void SendSceneViaDLT(const String& sceneDumpFileName);

        ramses::SceneImpl& m_scene;
    };

}

#endif
