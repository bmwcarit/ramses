//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositorMock.h"

using namespace testing;

namespace ramses_internal
{
    EmbeddedCompositorMock::EmbeddedCompositorMock()
    {
        ON_CALL(*this, dispatchUpdatedStreamTextureSourceIds()).WillByDefault(Return(StreamTextureSourceIdSet()));
        ON_CALL(*this, dispatchNewStreamTextureSourceIds()).WillByDefault(Return(StreamTextureSourceIdSet()));
        ON_CALL(*this, dispatchObsoleteStreamTextureSourceIds()).WillByDefault(Return(StreamTextureSourceIdSet()));

        EXPECT_CALL(*this, isRealCompositor()).Times(AnyNumber()).WillRepeatedly(Return(true));
    }

    EmbeddedCompositorMock::~EmbeddedCompositorMock()
    {
    }

    EmbeddedCompositorMockWithDestructor::EmbeddedCompositorMockWithDestructor()
    {
    }

    EmbeddedCompositorMockWithDestructor::~EmbeddedCompositorMockWithDestructor()
    {
        Die();
    }
}
