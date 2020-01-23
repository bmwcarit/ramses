//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGRESOURCEMEMORYUSAGE_H
#define RAMSES_LOGRESOURCEMEMORYUSAGE_H

#include "Ramsh/RamshCommandArguments.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class RamsesClientImpl;
}

namespace ramses_internal
{
    class LogResourceMemoryUsage : public RamshCommandArgs<uint64_t>
    {
    public:
        explicit LogResourceMemoryUsage(ramses::RamsesClientImpl& client);
        virtual Bool execute(uint64_t& sceneId) const override;
    private:
        ramses::RamsesClientImpl& m_client;
    };
}

#endif
