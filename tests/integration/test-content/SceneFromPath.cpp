//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/SceneFromPath.h"

#include "ramses/client/RamsesClient.h"

#include <string>

namespace ramses::internal
{
    SceneFromPath::SceneFromPath(ramses::RamsesClient& ramsesClient, const std::string& folder, const std::string& fileName)
    {
        ramses::Scene* loadedScene = ramsesClient.loadSceneFromFile(folder + fileName + ".ramses");
        loadedScene->flush();

        m_createdScene = loadedScene;
    }

    ramses::Scene* SceneFromPath::getCreatedScene()
    {
        return m_createdScene;
    }
}
