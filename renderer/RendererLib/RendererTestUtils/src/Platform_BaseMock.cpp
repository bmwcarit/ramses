//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_BaseMock.h"

using namespace testing;

namespace ramses_internal
{
    Platform_BaseMock::Platform_BaseMock(const RendererConfig& config)
        : Platform_Base(config)
    {
        ON_CALL(*this, createSystemCompositorController()).WillByDefault(Invoke(this, &Platform_BaseMock::createSystemCompositorController_fake));
        ON_CALL(*this, createWindow(_, _)).WillByDefault(Invoke(this, &Platform_BaseMock::createWindow_fake));
        ON_CALL(*this, createContext(_)).WillByDefault(Invoke(this, &Platform_BaseMock::createContext_fake));
        ON_CALL(*this, createDevice(_)).WillByDefault(Invoke(this, &Platform_BaseMock::createDevice_fake));
        ON_CALL(*this, createEmbeddedCompositor(_, _)).WillByDefault(Invoke(this, &Platform_BaseMock::createEmbeddedCompositor_fake));
        ON_CALL(*this, createTextureUploadingAdapter(_, _, _)).WillByDefault(Invoke(this, &Platform_BaseMock::createTextureUploadingAdapter_fake));
    }

    Platform_BaseMock::~Platform_BaseMock()
    {
    }

    void Platform_BaseMock::createRenderBackendMockObjects()
    {
        window                      = createMockObjectHelper<WindowMockWithDestructor>();
        context                     = createMockObjectHelper<ContextMockWithDestructor>();
        device                      = createMockObjectHelper<DeviceMockWithDestructor>();
        embeddedCompositor          = createMockObjectHelper<EmbeddedCompositorMockWithDestructor>();
    }
}
