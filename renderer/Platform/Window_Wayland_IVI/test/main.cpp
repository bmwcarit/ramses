//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "RendererTestUtils.h"
#include "PlatformAbstraction/PlatformConsole.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"

int main(int argc, char* argv[])
{
    testing::InitGoogleMock(&argc, argv);

    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig displayConfig;
    RendererTestUtils::SetDefaultConfigForAllTests(rendererConfig, displayConfig);

    return RUN_ALL_TESTS();
}
