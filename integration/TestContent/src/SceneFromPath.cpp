//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/SceneFromPath.h"

#include "ramses-client-api/RamsesClient.h"

namespace ramses_internal
{
    SceneFromPath::SceneFromPath(ramses::RamsesClient& ramsesClient, const String& folder, const String& fileName)
    {
        ramses::Scene* loadedScene = ramsesClient.loadSceneFromFile((folder + fileName + String(".ramses")).c_str());
        loadedScene->flush();

        m_createdScene = loadedScene;
    }

    ramses::Scene* SceneFromPath::getCreatedScene()
    {
        return m_createdScene;
    }
}
