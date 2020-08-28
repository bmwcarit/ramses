//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEACTIONUTILS_H
#define RAMSES_SCENEACTIONUTILS_H

#include "Scene/SceneActionCollection.h"

namespace ramses_internal
{
    using SceneActionIdVector = std::vector<ESceneActionId> ;

    class SceneActionCollectionUtils
    {
    public:
        static UInt32 CountNumberOfActionsOfType(const SceneActionCollection& actions, ESceneActionId type);
        static UInt32 CountNumberOfActionsOfType(const SceneActionCollection& actions, const SceneActionIdVector& types);
    };
}

#endif
