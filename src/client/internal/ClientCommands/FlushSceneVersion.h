//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommandArguments.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

namespace ramses::internal
{
    class RamsesClientImpl;

    class FlushSceneVersion : public RamshCommandArgs<sceneVersionTag_t, uint64_t>
    {
    public:
        explicit FlushSceneVersion(RamsesClientImpl& client);
        bool execute(sceneVersionTag_t& sceneVersion, uint64_t& sceneId) const override;

    private:
        RamsesClientImpl& m_client;
    };
}
