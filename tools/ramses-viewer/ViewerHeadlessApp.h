//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ViewerApp.h"

namespace ramses::internal
{

    class ViewerHeadlessApp : public ViewerApp
    {
    public:
        ViewerHeadlessApp();

        void registerOptions(CLI::App& cli);

        [[nodiscard]] ExitCode run();
    };
}
