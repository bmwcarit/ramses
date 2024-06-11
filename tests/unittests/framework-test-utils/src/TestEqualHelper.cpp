//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestEqualHelper.h"

namespace ramses::internal
{
    bool operator==(const SceneInfo& a, const SceneInfo& b)
    {
        return a.sceneID == b.sceneID
            && a.friendlyName == b.friendlyName
            && a.renderBackendCompatibility == b.renderBackendCompatibility
            && a.vulkanAPIVersion == b.vulkanAPIVersion
            && a.spirvVersion == b.spirvVersion;
    }

    bool operator!=(const SceneInfo& a, const SceneInfo& b)
    {
        return !(a == b);
    }
}
