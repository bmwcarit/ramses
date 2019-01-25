//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DUMPSCENETOFILE_H
#define RAMSES_DUMPSCENETOFILE_H

#include "Ramsh/RamshCommandArguments.h"
#include "Collections/String.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class RamsesClientImpl;
}

namespace ramses_internal
{
    class RamsesObject;

    class DumpSceneToFile : public RamshCommandArgs<ramses::sceneId_t, String, String>
    {
    public:
        explicit DumpSceneToFile(ramses::RamsesClientImpl& client);
        virtual Bool execute(ramses::sceneId_t& sceneId, String& fileName, String& sendViaDLT) const override;

    private:
        ramses::RamsesClientImpl& m_client;
    };
}

#endif
