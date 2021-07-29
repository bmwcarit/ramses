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
        ON_CALL(*this, createRenderBackend(_, _)).WillByDefault(Return(&renderBackendMock));
        ON_CALL(*this, createResourceUploadRenderBackend()).WillByDefault(Return(&resourceUploadRenderBackendMock));

        // SC disabled by default
        ON_CALL(*this, getSystemCompositorController()).WillByDefault(Return(nullptr));
        EXPECT_CALL(*this, getSystemCompositorController()).Times(AnyNumber());
    }

    template <template<typename> class MOCK_TYPE>
    PlatformMock<MOCK_TYPE>::~PlatformMock() = default;

    template class PlatformMock < NiceMock > ;
    template class PlatformMock < StrictMock > ;
}
