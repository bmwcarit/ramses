//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cassert>
#include <atomic>

namespace ramses::internal
{
    class RefCounted
    {
    public:
        RefCounted() = default;
        virtual ~RefCounted() = default;

        uint32_t addRef();
        uint32_t release();

        [[nodiscard]] uint32_t getReferenceCount() const;

    private:
        std::atomic<uint32_t> m_refCount{1u};
    };

    inline uint32_t RefCounted::addRef()
    {
        return ++m_refCount;
    }

    inline uint32_t RefCounted::release()
    {
        const uint32_t refCountBefore = m_refCount--;
        assert(refCountBefore > 0);
        if (1u == refCountBefore)
        {
            delete this;
            // attention:   no access to instance members after this line!
        }
        return refCountBefore - 1;
    }

    inline uint32_t RefCounted::getReferenceCount() const
    {
        return m_refCount;
    }
}
