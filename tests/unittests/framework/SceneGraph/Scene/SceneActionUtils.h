//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/SceneActionCollection.h"

namespace ramses::internal
{
    using SceneActionIdVector = std::vector<ESceneActionId> ;

    class SceneActionCollectionUtils
    {
    public:
        static uint32_t CountNumberOfActionsOfType(const SceneActionCollection& actions, ESceneActionId type);
        static uint32_t CountNumberOfActionsOfType(const SceneActionCollection& actions, const SceneActionIdVector& types);
    };
}
