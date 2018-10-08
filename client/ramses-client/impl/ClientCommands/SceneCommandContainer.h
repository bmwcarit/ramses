//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECOMMANDCONTAINER_H
#define RAMSES_SCENECOMMANDCONTAINER_H

#include "CommandContainer.h"
#include "SceneCommandTypes.h"

namespace ramses_internal
{
    typedef CommandContainer<ESceneCommand, SceneCommand> SceneCommandContainer;
}

#endif
