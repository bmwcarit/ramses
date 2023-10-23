
//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositingManagerMock.h"

namespace ramses::internal
{
    EmbeddedCompositingManagerMock::EmbeddedCompositingManagerMock()
    {
        EXPECT_CALL(*this, dispatchStateChangesOfSources(_, _, _)).Times(AnyNumber());
        EXPECT_CALL(*this, processClientRequests()).Times(AnyNumber());
        EXPECT_CALL(*this, hasUpdatedContentFromStreamSourcesToUpload()).Times(AnyNumber()).WillRepeatedly(Return(false));
        EXPECT_CALL(*this, hasRealCompositor()).Times(AnyNumber()).WillRepeatedly(Return(true));
    }
}
