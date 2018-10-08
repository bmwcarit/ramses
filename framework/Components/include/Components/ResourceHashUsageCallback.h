//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEHASHUSAGECALLBACK_H
#define RAMSES_RESOURCEHASHUSAGECALLBACK_H

#include "IResourceHashUsageCallback.h"

namespace ramses_internal
{
    class ResourceHashUsageCallback
    {
    public:

        explicit ResourceHashUsageCallback(IResourceHashUsageCallback& callback)
            : m_callback(callback)
        {
        }

        void operator()(const ResourceContentHash* hash)
        {
            m_callback.resourceHashUsageZero(*hash);
        }

    private:
        IResourceHashUsageCallback& m_callback;
    };
}

#endif
