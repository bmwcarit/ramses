//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererFactory.h"
#include "impl/FrameworkFactoryRegistry.h"
#include "impl/RamsesRendererImpl.h"

namespace ramses::internal
{
    RendererUniquePtr RendererFactory::createRenderer(RamsesFrameworkImpl& framework, const ramses::RendererConfig& config) const
    {
        auto impl = std::make_unique<RamsesRendererImpl>(framework, config);
        RendererUniquePtr renderer{ new RamsesRenderer{ std::move(impl) },
            [](RamsesRenderer* renderer_) { delete renderer_; } };
        return renderer;
    }

    bool RendererFactory::RegisterRendererFactory()
    {
        UniquePtrWithDeleter<IRendererFactory> rendererFactory(new RendererFactory, [](IRendererFactory* rendererFactory_) { delete rendererFactory_; });
        FrameworkFactoryRegistry::GetInstance().registerRendererFactory(std::move(rendererFactory));
        return true;
    }
}
