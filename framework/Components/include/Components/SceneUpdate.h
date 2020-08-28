//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEUPDATE_H
#define RAMSES_SCENEUPDATE_H

#include "Components/ManagedResource.h"
#include "Scene/SceneActionCollection.h"

namespace ramses_internal
{
    struct SceneUpdate
    {
        SceneActionCollection actions;
        ManagedResourceVector resources;
    };
}

#endif
