//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCONFIGUTILS_H
#define RAMSES_RENDERERCONFIGUTILS_H

#include "RendererAPI/Types.h"
#include "Math3d/CameraMatrixHelper.h"

namespace ramses_internal
{
    class CommandLineParser;
    class RendererConfig;
    class DisplayConfig;

    class RendererConfigUtils
    {
    public:
        static void ApplyValuesFromCommandLine(const CommandLineParser& parser, RendererConfig& config);
        static void ApplyValuesFromCommandLine(const CommandLineParser& parser, DisplayConfig& config);
        static void PrintCommandLineOptions();
    };
}

#endif
