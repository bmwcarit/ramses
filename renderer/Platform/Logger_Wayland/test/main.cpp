//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "RendererTestUtils.h"
#include "PlatformAbstraction/PlatformConsole.h"

#include "PlatformFactoryMock.h"
#include "Platform_Base/PlatformFactory_Base.h"

ramses_internal::NiceMock<ramses_internal::PlatformFactoryNiceMock>* gPlatformFactoryMock = nullptr;

ramses_internal::IPlatformFactory* ramses_internal::PlatformFactory_Base::CreatePlatformFactory(const ramses_internal::RendererConfig&)
{
    gPlatformFactoryMock = new ::testing::NiceMock<PlatformFactoryNiceMock>();
    return gPlatformFactoryMock;
}

int main(int argc, char* argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);

    ramses_internal::ArgumentUInt32 waylandIviLayerId(parser, "lid", "waylandIviLayerId", 3);
    ramses_internal::ArgumentBool helpRequested(parser, "help", "help", false);

    RendererTestUtils::SetWaylandIviLayerID(waylandIviLayerId);

    testing::InitGoogleMock(&argc, argv);

    if (helpRequested)
    {
        ramses_internal::StringOutputStream stream;
        stream << "\n";
        stream << "Additional command line parameters:\n";
        stream << waylandIviLayerId.getHelpString();
        stream << "\n";

        ramses_internal::Console::Print(ramses_internal::Console::GREEN, stream.c_str());
    }

    return RUN_ALL_TESTS();
}
