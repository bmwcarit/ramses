//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DLTLOGAPPENDER_H
#define RAMSES_DLTLOGAPPENDER_H

#include "Utils/LogAppenderBase.h"
#include "DltLogAppender/DltAdapter.h"

namespace ramses_internal
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

#endif
