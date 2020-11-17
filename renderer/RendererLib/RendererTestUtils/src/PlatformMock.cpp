//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformMock.h"

using namespace testing;

namespace ramses_internal
{
    template <template<typename> class MOCK_TYPE>
    PlatformMock<MOCK_TYPE>::PlatformMock(Bool testPerRendererConponents)
    {
        createDefaultMockCalls(testPerRendererConponents);

        EXPECT_CALL(*this, createPerRendererComponents()).Times(1u);
        EXPECT_CALL(*this, destroyPerRendererComponents()).Times(1u);

        EXPECT_CALL(*this, getSystemCompositorController()).Times(AnyNumber());
        EXPECT_CALL(*this, getWindowEventsPollingManager()).Times(AnyNumber());
    }

    template <template<typename> class MOCK_TYPE>
    PlatformMock<MOCK_TYPE>::~PlatformMock()
    {
    }

    template <template<typename> class MOCK_TYPE>
    void PlatformMock<MOCK_TYPE>::createDefaultMockCalls(Bool testPerRendererConponents)
    {
        ON_CALL(*this, createWindow(_, _)).WillByDefault(Return(&renderBackendMock.surfaceMock.windowMock));
        ON_CALL(*this, createContext(_)).WillByDefault(Return(&renderBackendMock.surfaceMock.contextMock));
        ON_CALL(*this, createDevice(_)).WillByDefault(Return(&renderBackendMock.deviceMock));
        ON_CALL(*this, createSurface(_, _)).WillByDefault(Return(&renderBackendMock.surfaceMock));
        ON_CALL(*this, createEmbeddedCompositor(_, _)).WillByDefault(Return(&renderBackendMock.embeddedCompositorMock));
        ON_CALL(*this, getSystemCompositorController()).WillByDefault(Return(testPerRendererConponents ? &systemCompositorControllerMock : nullptr));
        ON_CALL(*this, getWindowEventsPollingManager()).WillByDefault(Return(testPerRendererConponents ? &windowEventsPollingManagerMock : nullptr));
        ON_CALL(*this, createRenderBackend(_, _)).WillByDefault(Return(&renderBackendMock));
    }

    template class PlatformMock < NiceMock > ;
    template class PlatformMock < StrictMock > ;
}
