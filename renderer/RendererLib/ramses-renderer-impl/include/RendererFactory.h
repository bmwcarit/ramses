//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERFACTORY_H
#define RAMSES_RENDERERFACTORY_H

#include "ramses-renderer-api/RamsesRenderer.h"
#include "RamsesObjectFactoryInterfaces.h"

namespace ramses
{
    class RendererFactory : public IRendererFactory
    {
    public:
        static bool RegisterRendererFactory();

        virtual RendererUniquePtr createRenderer(RamsesFrameworkImpl* impl, const RendererConfig& config) const override;
    };
}
#endif
