//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/RamsesRenderer.h"
#include "impl/RamsesObjectFactoryInterfaces.h"

namespace ramses::internal
{
    class RendererFactory : public IRendererFactory
    {
    public:
        static bool RegisterRendererFactory();

        RendererUniquePtr createRenderer(RamsesFrameworkImpl& framework, const ramses::RendererConfig& config) const override;
    };
}

