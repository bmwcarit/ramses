//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "LogicViewerApp.h"

namespace rlogic
{
    class LogicViewerHeadlessApp : public LogicViewerApp
    {
    public:
        LogicViewerHeadlessApp(int argc, char const* const* argv);

        ~LogicViewerHeadlessApp() override;

        [[nodiscard]] bool doOneLoop() override;

    private:
        [[nodiscard]] int init(int argc, char const* const* argv);
    };
}

