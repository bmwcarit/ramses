//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    class IRenderBackend;
    class DisplayConfig;
    class IWindowEventHandler;
    class IResourceUploadRenderBackend;
    class ISystemCompositorController;

    class IPlatform
    {
    public:
        virtual IRenderBackend*               createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) = 0;
        virtual void                          destroyRenderBackend() = 0;
        virtual IResourceUploadRenderBackend* createResourceUploadRenderBackend() = 0;
        virtual void                          destroyResourceUploadRenderBackend() = 0;

        virtual ISystemCompositorController* getSystemCompositorController() = 0;

        virtual ~IPlatform() = default;
    };
}
