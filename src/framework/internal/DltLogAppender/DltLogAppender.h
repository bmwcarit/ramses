//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogAppenderBase.h"
#include "internal/DltLogAppender/DltAdapter.h"

namespace ramses::internal
{
    /**
     * Logs messages via dlt
     */
    class DltLogAppender: public LogAppenderBase
    {
    public:
        DltLogAppender();
        void log(const LogMessage& message) override;

    private:
        DltAdapter* m_dltAdapter;
    };
}
