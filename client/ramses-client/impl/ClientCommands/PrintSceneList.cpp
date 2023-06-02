//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PrintSceneList.h"
#include "ramses-client-api/Scene.h"
#include "RamsesClientImpl.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    PrintSceneList::PrintSceneList(const ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        description = "print scene list";
        registerKeyword("printSceneList");
    }

    bool PrintSceneList::executeInput(const std::vector<std::string>& input)
    {
        UNUSED(input);

        LOG_INFO_F(CONTEXT_CLIENT, ([&](StringOutputStream& sos) {
                    sos << "PrintSceneList::executeInput:  ";
                    sos << "Scenes: \n\r";
                    for (const auto& scene : m_client.getListOfScenes())
                    {
                        sos << "    Id: " << scene->getSceneId() << " Name: \"" << scene->getName() << "\"\n\r";
                    }
                }));

        return true;
    }
}
