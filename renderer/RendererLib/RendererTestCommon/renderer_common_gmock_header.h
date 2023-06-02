//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERER_COMMON_GMOCK_HEADER_H
#define RAMSES_RENDERER_COMMON_GMOCK_HEADER_H

#include "framework_common_gmock_header.h"

namespace ramses_internal
{
    // See base header framework_common_gmock_header.h for more info why this file is needed

    class DisplayConfig;
    class RendererConfig;

    // fill out with renderer specific PrintTo function declarations (definitions go to cpp file!)
    void PrintTo(const DisplayConfig& config, ::std::ostream* os);
    void PrintTo(const RendererConfig& config, ::std::ostream* os);
}


#endif
