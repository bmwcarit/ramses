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
#include "ramses-cli.h"

int main(int argc, char *argv[])
{
    testing::InitGoogleMock(&argc, argv);

    CLI::App cli;
    ramses::RamsesFrameworkConfig framworkConfig{ramses::EFeatureLevel_Latest};
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig displayConfig;
    try
    {
        ramses::registerOptions(cli, framworkConfig);
        ramses::registerOptions(cli, rendererConfig);
        ramses::registerOptions(cli, displayConfig);
    }
    catch (const CLI::Error& error)
    {
        // configuration error
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);
    RendererTestUtils::SetDefaultConfigForAllTests(rendererConfig, displayConfig);

    return RUN_ALL_TESTS();
}
