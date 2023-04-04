//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <vector>
#include <string>
#include "ramses-logic/ErrorData.h"

namespace rlogic::internal
{
    class ErrorReporting
    {
    public:

        void clear();
        void add(std::string errorMessage, const LogicObject* logicObject, EErrorType type);

        [[nodiscard]] const std::vector<rlogic::ErrorData>& getErrors() const;

    private:
        std::vector<rlogic::ErrorData> m_errors;
    };
}
