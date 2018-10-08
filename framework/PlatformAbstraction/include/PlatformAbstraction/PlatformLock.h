//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMLOCK_H
#define RAMSES_PLATFORMLOCK_H

#include <ramses-capu/os/Mutex.h>
#include <ramses-capu/os/LightweightMutex.h>
#include "PlatformTypes.h"

namespace ramses_internal
{
    template<class Mutex>
    class PlatformLockTemplate
    {
        friend class PlatformEvent;
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

        Bool trylock()
        {
            return m_mutex.trylock();
        }

    private:
        Mutex m_mutex;
    };

    class PlatformLock : public PlatformLockTemplate<ramses_capu::Mutex>
    {};

    class PlatformLightweightLock : public PlatformLockTemplate<ramses_capu::LightweightMutex>
    {};
}

#endif
