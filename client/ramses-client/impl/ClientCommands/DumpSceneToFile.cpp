//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DumpSceneToFile.h"
#include "RamsesClientImpl.h"
#include "SceneCommandTypes.h"

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

    Bool DumpSceneToFile::execute(uint64_t& sceneId, String& fileName, String& sendViaDLT) const
    {
        DumpSceneToFileCommand command;
        command.fileName = fileName;
        command.sendViaDLT = sendViaDLT == String("-sendViaDLT");

        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), command);
        return true;
    }
}
