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
    PlatformMock<MOCK_TYPE>::PlatformMock()
    {
        createDefaultMockCalls();

        EXPECT_CALL(*this, getSystemCompositorController()).Times(AnyNumber());
        EXPECT_CALL(*this, getWindowEventsPollingManager()).Times(AnyNumber());
    }

    template <template<typename> class MOCK_TYPE>
    PlatformMock<MOCK_TYPE>::~PlatformMock()
    {
    }

    template <template<typename> class MOCK_TYPE>
    void PlatformMock<MOCK_TYPE>::createDefaultMockCalls()
    {
        ON_CALL(*this, createWindow(_, _)).WillByDefault(Return(&renderBackendMock.surfaceMock.windowMock));
        ON_CALL(*this, createContext(_, nullptr)).WillByDefault(Return(&renderBackendMock.surfaceMock.contextMock)); //non-shared context
        ON_CALL(*this, createDevice(Ref(renderBackendMock.surfaceMock.contextMock))).WillByDefault(Return(&renderBackendMock.deviceMock));
        ON_CALL(*this, createSurface(_, _)).WillByDefault(Return(&renderBackendMock.surfaceMock));
        ON_CALL(*this, createEmbeddedCompositor(_, _)).WillByDefault(Return(&renderBackendMock.embeddedCompositorMock));
        ON_CALL(*this, getSystemCompositorController()).WillByDefault(Return(&systemCompositorControllerMock));
        ON_CALL(*this, getWindowEventsPollingManager()).WillByDefault(Return(&windowEventsPollingManagerMock));
        ON_CALL(*this, createRenderBackend(_, _)).WillByDefault(Return(&renderBackendMock));

        ON_CALL(*this, createResourceUploadRenderBackend(_)).WillByDefault(Return(&resourceUploadRenderBackendMock));
        ON_CALL(*this, createContext(_, Ne(nullptr))).WillByDefault(Return(&resourceUploadRenderBackendMock.contextMock)); //shared context (not null)
        ON_CALL(*this, createDevice(Ref(resourceUploadRenderBackendMock.contextMock))).WillByDefault(Return(&resourceUploadRenderBackendMock.deviceMock));
    }

    template class PlatformMock < NiceMock > ;
    template class PlatformMock < StrictMock > ;

    template <template<typename> class MOCK_TYPE>
    PlatformMockWithPerRendererComponents<MOCK_TYPE>::PlatformMockWithPerRendererComponents(bool testPerRendererComponents /*= true*/)
    {
        EXPECT_CALL(*this, createPerRendererComponents());
        EXPECT_CALL(*this, destroyPerRendererComponents());

        if (!testPerRendererComponents)
        {
            ON_CALL(*this, getSystemCompositorController()).WillByDefault(Return(nullptr));
            ON_CALL(*this, getWindowEventsPollingManager()).WillByDefault(Return(nullptr));
        }
    }
    template class PlatformMockWithPerRendererComponents < NiceMock >;
    template class PlatformMockWithPerRendererComponents < StrictMock >;
}
