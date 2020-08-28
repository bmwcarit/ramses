//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FILELOADINGSCENE_H
#define RAMSES_FILELOADINGSCENE_H

#include "IntegrationScene.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/SceneConfig.h"

namespace ramses
{
    class RenderGroup;
    class RamsesFrameworkConfig;
}

namespace ramses_internal
{
    // FileLoadingScene is NO IntegrationScene (IntegrationScene needs scene and initializes it -> not wanted here)
    class FileLoadingScene
    {
    public:
        FileLoadingScene(ramses::RamsesClient& clientForLoading, UInt32 state, ramses::sceneId_t sceneId, const Vector3& cameraPosition, const String& folder, const ramses::RamsesFrameworkConfig& config);

        ramses::Scene* getCreatedScene();

        enum
        {
            CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT = 0,
            CREATE_SAVE_DESTROY_LOAD_USING_SAME_CLIENT
        };

    private:
        ramses::Scene* m_createdScene;

        void createFiles(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const Vector3& cameraPosition, const String& folder, const ramses::SceneConfig& sceneConfig = ramses::SceneConfig());
        void initializeAnimationContent(ramses::Scene& scene, ramses::RenderGroup& renderGroup);
        void loadFromFiles(ramses::RamsesClient& client, const String& folder);
        void cleanupFiles(const String& folder);
    };
}

#endif
