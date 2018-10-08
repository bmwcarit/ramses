//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneCommandTypes.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    const char* SceneCommandNames[] =
    {
        "ESceneCommand_ForceFallbackImage",
        "ESceneCommand_FlushSceneVersion",
        "ESceneCommand_ValidationRequest",
        "ESceneCommand_DumpSceneToFile",
        "ESceneCommand_LogResourceMemoryUsage"
    };

    ENUM_TO_STRING(ESceneCommand, SceneCommandNames, ESceneCommand_NUMBER_OF_SCENE_COMMANDS);

    const char* getSceneCommandName( ESceneCommand type )
    {
        return EnumToString(type);
    }

}
