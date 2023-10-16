//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DumpSceneToFile.h"
#include "impl/RamsesClientImpl.h"
#include "SceneCommandBuffer.h"

namespace ramses::internal
{
    DumpSceneToFile::DumpSceneToFile(ramses::internal::RamsesClientImpl& client)
        : m_client(client)
    {
        description = "Usage: [-sendViaDLT] [-flush] sceneId [filename] - dump scene to file or DLT";
        registerKeyword("dumpSceneToFile");
        registerKeyword("dumpScene");
    }

    bool DumpSceneToFile::executeInput(const std::vector<std::string>& input)
    {
        std::vector<std::string> arguments;
        SceneCommandDumpSceneToFile command{};
        bool flush = false;

        for (auto& str : input)
        {
            if (str == "-sendViaDLT")
            {
                command.sendViaDLT = true;
            }
            else if (str == "-flush")
            {
                flush = true;
            }
            else
            {
                arguments.push_back(str);
            }
        }

        switch (arguments.size())
        {
        case 3:
            command.fileName = arguments[2];
            break;
        case 2:
            if (!command.sendViaDLT)
            {
                LOG_ERROR_P(CONTEXT_RAMSH, "Expected [filename] or -sendViaDlt");
                return false;
            }
            break;
        default:
            LOG_ERROR_P(CONTEXT_RAMSH, "None or too many arguments provided: {}", arguments.size() - 1);
            return false;
        }

        ramses::sceneId_t sceneId;
        ramses::internal::ArgumentConverter<uint64_t>::tryConvert(arguments[1], sceneId.getReference());
        if (!sceneId.isValid())
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid SceneId: {}", arguments[0]);
            return false;
        }

        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), std::move(command));
        if (flush)
        {
            auto* scene = m_client.getScene(sceneId);
            if (!scene)
            {
                LOG_ERROR_P(CONTEXT_RAMSH, "Scene not available: {}", sceneId);
                return false;
            }
            scene->flush();
        }
        return true;
    }
}
