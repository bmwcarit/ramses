//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SurfaceMock.h"

using namespace testing;

namespace ramses_internal
{
    template <template<typename> class MOCK_TYPE>
    SurfaceMock<MOCK_TYPE>::SurfaceMock()
    {
        ON_CALL(*this, getWindow()).WillByDefault(ReturnRef(windowMock));
        ON_CALL(*this, getContext()).WillByDefault(ReturnRef(contextMock));

        EXPECT_CALL(*this, getWindow()).Times(AnyNumber());
        EXPECT_CALL(*this, getContext()).Times(AnyNumber());
    }

    template <template<typename> class MOCK_TYPE>
    SurfaceMock<MOCK_TYPE>::~SurfaceMock()
    {
    }

    SurfaceMockWithDestructor::SurfaceMockWithDestructor()
    {
    }

    SurfaceMockWithDestructor::~SurfaceMockWithDestructor()
    {
        Die();
    }

    template class SurfaceMock < NiceMock > ;
    template class SurfaceMock < StrictMock > ;
}

