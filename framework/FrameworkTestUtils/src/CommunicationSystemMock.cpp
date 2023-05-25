//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "CommunicationSystemMock.h"

using namespace testing;

namespace ramses_internal
{
    CommunicationSystemMock::CommunicationSystemMock() = default;
    CommunicationSystemMock::~CommunicationSystemMock() = default;

    void CommunicationSystemMock::setSceneProviderServiceHandler(ISceneProviderServiceHandler*)
    {
    }

    void CommunicationSystemMock::setSceneRendererServiceHandler(ISceneRendererServiceHandler*)
    {
    }
}
