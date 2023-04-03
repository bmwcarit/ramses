//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/WarningData.h"

#include <vector>
#include <string>

namespace rlogic::internal
{
    class ValidationResults
    {
    public:
        void clear();
        void add(std::string warningMessage, const LogicObject* logicObject, EWarningType type);
        [[nodiscard]] const std::vector<rlogic::WarningData>& getWarnings() const;

    private:
        std::vector<rlogic::WarningData> m_warnings;
    };
}
