//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PRINTSCENELIST_H
#define RAMSES_PRINTSCENELIST_H

#include "Ramsh/RamshCommand.h"

namespace ramses
{
    class RamsesClientImpl;
}

namespace ramses_internal
{
    class PrintSceneList : public RamshCommand
    {
    public:
        explicit PrintSceneList(const ramses::RamsesClientImpl& client);
        virtual bool executeInput(const std::vector<std::string>& input) override;
    private:
        const ramses::RamsesClientImpl& m_client;
    };
}

#endif //RAMSES_PRINTSCENELIST_H
