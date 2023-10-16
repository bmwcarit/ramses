//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once


#include "IDiscoveryDaemon.h"

namespace ramses::internal
{
    class FakeDiscoveryDaemon final : public IDiscoveryDaemon
    {
    public:
        bool start() override
        {
            if (m_started)
            {
                return false;
            }
            m_started = true;
            return true;
        }

        bool stop() override
        {
            if (!m_started)
            {
                return false;
            }
            m_started = false;
            return true;
        }

    private:
        bool m_started{false};
    };
}
