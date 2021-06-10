//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INPUTSTREAMMOCK_H
#define RAMSES_INPUTSTREAMMOCK_H

#include "Collections/IInputStream.h"
#include "Components/InputStreamContainer.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class InputStreamMock : public IInputStream
    {
    public:
        InputStreamMock();
        ~InputStreamMock() override;

        MOCK_METHOD(IInputStream&, read, (void* data, size_t size), (override));
        MOCK_METHOD(EStatus, getState, (), (const, override));

        MOCK_METHOD(EStatus, seek, (Int numberOfBytesToSeek, Seek origin), (override));
        MOCK_METHOD(EStatus, getPos, (size_t& position), (const, override));
    };
}

#endif
