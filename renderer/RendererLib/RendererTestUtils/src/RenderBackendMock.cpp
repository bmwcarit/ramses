//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderBackendMock.h"

namespace ramses_internal
{
    using namespace testing;

    template <template<typename> class MOCK_TYPE>
    RenderBackendMock<MOCK_TYPE>::RenderBackendMock()
    {
        ON_CALL(*this, getSurface()).WillByDefault(ReturnRef(surfaceMock));
        ON_CALL(*this, getDevice()).WillByDefault(ReturnRef(deviceMock));
        ON_CALL(*this, getEmbeddedCompositor()).WillByDefault(ReturnRef(embeddedCompositorMock));
        ON_CALL(*this, getTextureUploadingAdapter()).WillByDefault(ReturnRef(textureUploadingAdapterMock));

        EXPECT_CALL(*this, getDevice()).Times(AnyNumber());
        EXPECT_CALL(*this, getSurface()).Times(AnyNumber());
        EXPECT_CALL(*this, getEmbeddedCompositor()).Times(AnyNumber());
        EXPECT_CALL(*this, getTextureUploadingAdapter()).Times(AnyNumber());
    }

    template <template<typename> class MOCK_TYPE>
    RenderBackendMock<MOCK_TYPE>::~RenderBackendMock()
    {
    }

    template class RenderBackendMock < NiceMock > ;
    template class RenderBackendMock < StrictMock > ;
}
