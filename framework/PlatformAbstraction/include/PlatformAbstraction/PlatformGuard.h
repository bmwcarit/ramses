//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMGUARD_H
#define RAMSES_PLATFORMGUARD_H

#include <PlatformAbstraction/PlatformLock.h>

namespace ramses_internal
{
    /**
     *  A guard instance acquires the lock in the constructor and releases the lock in the destructor.
     *
     *  Therefore you can lay a guard instance onto the stack in order to acquire the lock. When the guard is removed
     *  from the stack the lock is automatically released. This is even valid when an exception occurs.
     */
    template<class Lock>
    class PlatformGuardTemplate
    {
    public:
        /**
         *  Constructor of a guard instance.
         *  @param  lock    Reference to the lock instance.
         */
        explicit inline PlatformGuardTemplate(Lock& lock)
            : m_lock(lock)
        {
            m_lock.lock();
        }

        /**
         *  Destructor.
         */
        inline ~PlatformGuardTemplate()
        {
            m_lock.unlock();
        }

    private:
        /**
         *  Reference to the lock instance used by the guard.
         */
        Lock& m_lock;
    };

    typedef PlatformGuardTemplate<PlatformLock> PlatformGuard;
    typedef PlatformGuardTemplate<PlatformLightweightLock> PlatformLightweightGuard;
}

#endif
