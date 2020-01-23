//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DltLogAppender/DltLogAppender.h"

namespace ramses_internal
{
    DltLogAppender::DltLogAppender()
        : m_dltAdapter(DltAdapter::getDltAdapter())
    {
    }

    void DltLogAppender::log(const LogMessage& message)
    {
        m_dltAdapter->logMessage(message);
    }
}
