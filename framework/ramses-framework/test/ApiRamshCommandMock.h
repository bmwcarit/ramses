//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEST_APIRAMSHCOMMANDMOCK_H
#define RAMSES_TEST_APIRAMSHCOMMANDMOCK_H

#include "ramses-framework-api/IRamshCommand.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class ApiRamshCommandMock : public ramses::IRamshCommand
    {
    public:
        ApiRamshCommandMock();
        ~ApiRamshCommandMock() override;

        MOCK_METHOD(const std::string&, keyword, (), (const, override));
        MOCK_METHOD(const std::string&, help, (), (const, override));
        MOCK_METHOD(bool, execute, (const std::vector<std::string>& input), (override));
    };
}

#endif
