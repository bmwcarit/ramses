//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IPLATFORMFACTORY_H
#define RAMSES_IPLATFORMFACTORY_H

#include "Types.h"
#include "EDeviceTypeId.h"

namespace ramses_internal
{
    class IRenderBackend;
    class IWindow;
    class IContext;
    class IDevice;
    class ISurface;
    class IEmbeddedCompositor;
    class DisplayConfig;
    class ISystemCompositorController;
    class IWindowEventHandler;
    class ITextureUploadingAdapter;
    class IWindowEventsPollingManager;

    class IPlatformFactory
    {
    public:
        virtual ~IPlatformFactory(){}

        virtual Bool                         createPerRendererComponents() = 0;
        virtual void                         destroyPerRendererComponents() = 0;
        virtual IRenderBackend*              createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) = 0;
        virtual void                         destroyRenderBackend(IRenderBackend& renderBackend) = 0;

        virtual ISystemCompositorController* getSystemCompositorController() const = 0;
        virtual const IWindowEventsPollingManager* getWindowEventsPollingManager() const = 0;

    protected:
        virtual ISystemCompositorController* createSystemCompositorController() = 0;
        virtual void                         destroySystemCompositorController() = 0;

        virtual IWindow*                     createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) = 0;
        virtual Bool                         destroyWindow(IWindow& window) = 0;

        virtual IContext*                    createContext(IWindow& window) = 0;
        virtual Bool                         destroyContext(IContext& context) = 0;

        virtual ISurface*                    createSurface(IWindow& window, IContext& context) = 0;
        virtual Bool                         destroySurface(ISurface& surface) = 0;

        virtual IDevice*                     createDevice(IContext& context) = 0;
        virtual Bool                         destroyDevice(IDevice& device) = 0;

        virtual IEmbeddedCompositor*         createEmbeddedCompositor(IContext& context) = 0;
        virtual Bool                         destroyEmbeddedCompositor(IEmbeddedCompositor& compositor) = 0;

        virtual ITextureUploadingAdapter*    createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window) = 0;
        virtual Bool                         destroyTextureUploadingAdapter(ITextureUploadingAdapter& textureUploadingAdapter) = 0;
    };
}

#endif
