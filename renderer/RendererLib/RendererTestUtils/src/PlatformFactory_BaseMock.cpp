//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformFactory_BaseMock.h"

using namespace testing;

namespace ramses_internal
{
    PlatformFactory_BaseMock::PlatformFactory_BaseMock(const RendererConfig& config)
        : PlatformFactory_Base(config)
    {
        ON_CALL(*this, createSystemCompositorController()).WillByDefault(Invoke(this, &PlatformFactory_BaseMock::createSystemCompositorController_fake));
        ON_CALL(*this, createWindow(_, _)).WillByDefault(Invoke(this, &PlatformFactory_BaseMock::createWindow_fake));
        ON_CALL(*this, createContext(_)).WillByDefault(Invoke(this, &PlatformFactory_BaseMock::createContext_fake));
        ON_CALL(*this, createDevice(_)).WillByDefault(Invoke(this, &PlatformFactory_BaseMock::createDevice_fake));
        ON_CALL(*this, createSurface(_, _)).WillByDefault(Invoke(this, &PlatformFactory_BaseMock::createSurface_fake));
        ON_CALL(*this, createEmbeddedCompositor(_)).WillByDefault(Invoke(this, &PlatformFactory_BaseMock::createEmbeddedCompositor_fake));
        ON_CALL(*this, createTextureUploadingAdapter(_, _, _)).WillByDefault(Invoke(this, &PlatformFactory_BaseMock::createTextureUploadingAdapter_fake));
    }

    PlatformFactory_BaseMock::~PlatformFactory_BaseMock()
    {
    }

    void PlatformFactory_BaseMock::createRenderBackendMockObjects()
    {
        window                      = createMockObjectHelper<WindowMockWithDestructor>();
        context                     = createMockObjectHelper<ContextMockWithDestructor>();
        surface                     = createMockObjectHelper<SurfaceMockWithDestructor>();
        device                      = createMockObjectHelper<DeviceMockWithDestructor>();
        embeddedCompositor          = createMockObjectHelper<EmbeddedCompositorMockWithDestructor>();
        textureUploadingAdapter     = createMockObjectHelper<TextureUploadingAdapterMockWithDestructor>();

        ON_CALL(*surface, getWindow()).WillByDefault(ReturnRef(*window));
        ON_CALL(*surface, getContext()).WillByDefault(ReturnRef(*context));
    }
}
