//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneViewer.h"
#include "ramses-cli.h"

int main(int argc, char* argv[])
{
    CLI::App cli;
    ramses_internal::SceneViewer sceneViewer;
    sceneViewer.registerOptions(cli);
    CLI11_PARSE(cli, argc, argv);
    return sceneViewer.run();
}
