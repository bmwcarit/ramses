//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DumpSceneToFile.h"
#include "RamsesClientImpl.h"
#include "SceneCommandBuffer.h"

namespace ramses_internal
{
    DumpSceneToFile::DumpSceneToFile(ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        description = "dump scene to file";
        registerKeyword("dumpSceneToFile");
        getArgument<0>().setDescription("scene id");
        getArgument<1>().setDescription("file name");
        getArgument<2>().setDescription("send dumped scene file and related resource files via dlt (-sendViaDLT)");
        getArgument<2>().setDefaultValue("");
    }

    bool DumpSceneToFile::execute(uint64_t& sceneId, std::string& fileName, std::string& sendViaDLT) const
    {
        SceneCommandDumpSceneToFile command;
        command.fileName = fileName;
        command.sendViaDLT = sendViaDLT == "-sendViaDLT";

        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), std::move(command));
        return true;
    }
}
