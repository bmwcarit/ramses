//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Scene.h"

#include <string>

namespace ramses::internal
{
    class SceneFromPath
    {
    public:
        SceneFromPath(ramses::RamsesClient& ramsesClient, const std::string& folder, const std::string& fileName);
        ramses::Scene* getCreatedScene();
    private:
        ramses::Scene* m_createdScene = nullptr;
    };
}
