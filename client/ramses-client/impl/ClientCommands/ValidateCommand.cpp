//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ValidateCommand.h"
#include "ramses-client-api/Scene.h"
#include "SceneImpl.h"
#include "RamsesClientImpl.h"
#include "Utils/LogMacros.h"
#include "ramses-framework-api/EValidationSeverity.h"

namespace ramses_internal
{
    ValidateCommand::ValidateCommand(ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        description = "validate a scene, or object within scene";
        registerKeyword("validate");
        getArgument<0>().setDescription("scene id");
        getArgument<1>().setDescription("severityLevel(info||warning||error)");
        getArgument<1>().setDefaultValue("error");
        getArgument<2>().setDescription("object name");
        getArgument<2>().setDefaultValue("");
    }

    bool ValidateCommand::execute(uint64_t& sceneId, std::string& severity, std::string& objectName) const
    {
        SceneCommandValidationRequest command;

        if (severity == "info")
        {
            command.severity = ramses::EValidationSeverity::Info;
        }
        else if (severity == "warning")
        {
            command.severity = ramses::EValidationSeverity::Warning;
        }
        else if (severity == "error")
        {
            command.severity = ramses::EValidationSeverity::Error;
        }
        else
        {
            LOG_ERROR(CONTEXT_CLIENT, "Wrong value for severity parameter, must be info, warning or error");
            return false;
        }

        command.optionalObjectName = objectName;

        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), std::move(command));
        return true;
    }
}
