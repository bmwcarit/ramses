
//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositingManager.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    using namespace testing;

    class EmbeddedCompositingManagerMock : public IEmbeddedCompositingManager
    {
    public:
        EmbeddedCompositingManagerMock();
        MOCK_METHOD(void, refStream, (WaylandIviSurfaceId source), (override));
        MOCK_METHOD(void, unrefStream, (WaylandIviSurfaceId source), (override));
        MOCK_METHOD(void, dispatchStateChangesOfSources, (WaylandIviSurfaceIdVector&, WaylandIviSurfaceIdVector&, WaylandIviSurfaceIdVector&), (override));
        MOCK_METHOD(void, processClientRequests, (), (override));
        MOCK_METHOD(bool, hasUpdatedContentFromStreamSourcesToUpload, (), (const, override));
        MOCK_METHOD(void, uploadResourcesAndGetUpdates, (StreamSourceUpdates&), (override));
        MOCK_METHOD(void, notifyClients, (), (override));
        MOCK_METHOD(DeviceResourceHandle, getCompositedTextureDeviceHandleForStreamTexture, (WaylandIviSurfaceId source), (const, override));
        MOCK_METHOD(bool, hasRealCompositor, (), (const, override));
    };
}

