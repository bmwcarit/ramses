//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerTestBase.h"

namespace rlogic::internal
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) must be static and non-const (see SetMockScreenshot)
    ALogicViewerBase::MockScreenshot* ALogicViewerBase::m_mockScreenshot = nullptr;
}
