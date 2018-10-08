//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECOMMANDTYPES_H
#define RAMSES_SCENECOMMANDTYPES_H

#include "Command.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/EValidationSeverity.h"
#include "Collections/String.h"


namespace ramses_internal
{
    enum ESceneCommand
    {
        // Scene and display control
        ESceneCommand_ForceFallbackImage = 0,
        ESceneCommand_FlushSceneVersion,
        ESceneCommand_ValidationRequest,
        ESceneCommand_DumpSceneToFile,
        ESceneCommand_LogResourceMemoryUsage,
        ESceneCommand_NUMBER_OF_SCENE_COMMANDS
    };

    const char* getSceneCommandName( ESceneCommand type );

    typedef Command< ESceneCommand > SceneCommand;

    struct ForceFallbackCommand : public SceneCommand
    {
        DEFINE_COMMAND_TYPE(ForceFallbackCommand, ESceneCommand_ForceFallbackImage);

        String            streamTextureName;
        Bool              forceFallback;
    };

    struct FlushSceneVersionCommand : public SceneCommand
    {
        DEFINE_COMMAND_TYPE(FlushSceneVersionCommand, ESceneCommand_FlushSceneVersion);

        ramses::sceneVersionTag_t sceneVersion;
    };

    struct ValidationRequestCommand : public SceneCommand
    {
        DEFINE_COMMAND_TYPE(ValidationRequestCommand, ESceneCommand_ValidationRequest);

        ramses::EValidationSeverity severity;
        ramses_internal::String optionalObjectName;
    };

    struct DumpSceneToFileCommand : public SceneCommand
    {
        DEFINE_COMMAND_TYPE(DumpSceneToFileCommand, ESceneCommand_DumpSceneToFile);

        ramses_internal::String fileName;
        Bool sendViaDLT;
    };

    struct LogResourceMemoryUsageCommand : public SceneCommand
    {
        DEFINE_COMMAND_TYPE(LogResourceMemoryUsageCommand, ESceneCommand_LogResourceMemoryUsage);
    };
}

#endif
