//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDEXIT_H
#define RAMSES_RAMSHCOMMANDEXIT_H

#include "Ramsh/RamshCommand.h"

#include "PlatformAbstraction/PlatformEvent.h"
#include <atomic>

namespace ramses_internal
{
    class Ramsh;

    class RamshCommandExit : public RamshCommand
    {
    public:
        explicit RamshCommandExit();
        virtual Bool executeInput(const RamshInput& input) override;

        Bool exitRequested();
        void waitForExitRequest(UInt32 timeoutMillisec = 0u);

    private:
        std::atomic<bool> m_exitRequested;
        PlatformEvent m_exitEvent;
    };

}

#endif
