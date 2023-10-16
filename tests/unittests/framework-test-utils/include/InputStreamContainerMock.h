//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/InputStreamContainer.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    class InputStreamContainerMock : public IInputStreamContainer
    {
    public:
        InputStreamContainerMock();
        ~InputStreamContainerMock() override;

        MOCK_METHOD(IInputStream&, getStream, (), (override));
    };
}
