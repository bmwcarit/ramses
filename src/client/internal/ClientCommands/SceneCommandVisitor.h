//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#pragma once

#include <string>

namespace ramses::internal
{
    class SceneImpl;

    struct SceneCommandSetProperty;
    struct SceneCommandFlushSceneVersion;
    struct SceneCommandValidationRequest;
    struct SceneCommandDumpSceneToFile;
    struct SceneCommandLogResourceMemoryUsage;

    class SceneCommandVisitor
    {
    public:
        explicit SceneCommandVisitor(SceneImpl& scene)
            : m_scene(scene)
        {}

        void operator()(const SceneCommandSetProperty& cmd);
        void operator()(const SceneCommandFlushSceneVersion& cmd);
        void operator()(const SceneCommandValidationRequest& cmd);
        void operator()(const SceneCommandDumpSceneToFile& cmd) const;
        void operator()(const SceneCommandLogResourceMemoryUsage& cmd) const;

    private:
        SceneImpl& m_scene;
    };

}
