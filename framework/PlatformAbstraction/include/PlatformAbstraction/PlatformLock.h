//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMLOCK_H
#define RAMSES_PLATFORMLOCK_H

#include <mutex>

namespace ramses_internal
{
    template<class Mutex>
    class PlatformLockTemplate
    {
        friend class PlatformConditionVariable;

    public:
        void lock()
        {
            m_mutex.lock();
        }

        void unlock()
        {
            m_mutex.unlock();
        }

        bool trylock()
        {
            return m_mutex.trylock();
        }

    private:
        Mutex m_mutex;
    };

    class PlatformLock : public PlatformLockTemplate<std::recursive_mutex>
    {};

    class PlatformLightweightLock : public PlatformLockTemplate<std::mutex>
    {};
}

#endif
