//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayControllerMock.h"
#include "WindowMock.h"
namespace ramses_internal {

const DeviceResourceHandle DisplayControllerMock::FakeFrameBufferHandle(15);
const ProjectionParams DisplayControllerMock::FakeProjectionParams(ProjectionParams::Perspective(30.0f, 2.666f, 0.00001f, 100.f));

DisplayControllerMock::DisplayControllerMock()
{
    using namespace ::testing;
    ON_CALL(*this, getDisplayBuffer()).WillByDefault(Return(FakeFrameBufferHandle));
    ON_CALL(*this, getDisplayWidth()).WillByDefault(Return(WindowMock::FakeWidth));
    ON_CALL(*this, getDisplayHeight()).WillByDefault(Return(WindowMock::FakeHeight));
    ON_CALL(*this, renderScene(_, _, _, _, _)).WillByDefault(Return(SceneRenderExecutionIterator()));
}

DisplayControllerMock::~DisplayControllerMock()
{
}
}
