//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ViewerHeadlessApp.h"
#include "ramses-cli.h"

int main(int argc, char* argv[])
{
    CLI::App cli;
    ramses::internal::ViewerHeadlessApp sceneViewer;
    sceneViewer.registerOptions(cli);
    CLI11_PARSE(cli, argc, argv);
    return static_cast<int>(sceneViewer.run());
}
