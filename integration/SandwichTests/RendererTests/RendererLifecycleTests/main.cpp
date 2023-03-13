//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestUtils.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "gmock/gmock.h"
#include "CLI/CLI.hpp"

int main(int argc, char *argv[])
{
    testing::InitGoogleMock(&argc, argv);

    CLI::App cli;
    ramses::RamsesFrameworkConfig framworkConfig;
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig displayConfig;
    framworkConfig.registerOptions(cli);
    rendererConfig.registerOptions(cli);
    displayConfig.registerOptions(cli);
    CLI11_PARSE(cli, argc, argv);
    RendererTestUtils::SetDefaultConfigForAllTests(rendererConfig, displayConfig);

    return RUN_ALL_TESTS();
}
