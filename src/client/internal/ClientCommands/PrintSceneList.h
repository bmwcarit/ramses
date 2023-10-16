//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommand.h"

namespace ramses::internal
{
    class RamsesClientImpl;

    class PrintSceneList : public RamshCommand
    {
    public:
        explicit PrintSceneList(const RamsesClientImpl& client);
        bool executeInput(const std::vector<std::string>& input) override;
    private:
        const RamsesClientImpl& m_client;
    };
}
