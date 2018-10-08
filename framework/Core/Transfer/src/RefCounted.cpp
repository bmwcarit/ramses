//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Transfer/RefCounted.h"
#include <assert.h>

namespace ramses_internal
{
    RefCounted::RefCounted()
        : m_refCount(1U)
    {
    }

    UInt32 RefCounted::getReferenceCount() const
    {
        return m_refCount;
    }


    RefCounted::~RefCounted()
    {
    }


    UInt32 RefCounted::addRef()
    {
        return ++m_refCount;
    }

    UInt32 RefCounted::release()
    {
        const UInt32 refCountBefore = m_refCount--;
        assert(refCountBefore > 0);
        if (1u == refCountBefore)
        {
            delete this;
            // attention:   no access to instance members after this line!
        }
        return refCountBefore - 1;
    }


}
