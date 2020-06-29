//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererFactory.h"
#include "FrameworkFactoryRegistry.h"
#include "RamsesRendererImpl.h"

namespace ramses
{
    RendererUniquePtr RendererFactory::createRenderer(RamsesFrameworkImpl* impl, const RendererConfig& config) const
    {
        RendererUniquePtr renderer(new RamsesRenderer(*new RamsesRendererImpl(*impl, config)),
            [](RamsesRenderer* renderer_) { delete renderer_; });
        return renderer;
    }

    bool RendererFactory::RegisterRendererFactory()
    {
        FrameworkFactoryRegistry::GetInstance().registerRendererFactory(std::make_unique<RendererFactory>());
        return true;
    }
}
