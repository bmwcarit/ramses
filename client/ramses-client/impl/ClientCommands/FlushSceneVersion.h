//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FLUSHSCENEVERSION_H
#define RAMSES_FLUSHSCENEVERSION_H

#include "Ramsh/RamshCommandArguments.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class RamsesClientImpl;
}

namespace ramses_internal
{
    class RamsesObject;

    class FlushSceneVersion : public RamshCommandArgs<ramses::sceneVersionTag_t, ramses::sceneId_t>
    {
    public:
        explicit FlushSceneVersion(ramses::RamsesClientImpl& client);
        virtual Bool execute(ramses::sceneVersionTag_t& sceneVersion, ramses::sceneId_t& sceneId) const override;

    private:
        ramses::RamsesClientImpl& m_client;
    };
}

#endif //RAMSES_FLUSHSCENEVERSION_H
