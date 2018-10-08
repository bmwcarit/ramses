/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_SCOPED_LOCK_H
#define RAMSES_CAPU_SCOPED_LOCK_H

#include "ramses-capu/os/Mutex.h"
#include "ramses-capu/os/LightweightMutex.h"

namespace ramses_capu
{

    /**
     * Scoped lock which locks a given lockable object in the constructor and unlocks it
     * in the descructor when going out of scope. This class is templated and can be used
     * with any class providing the methods void lock() and void unlock().
     */
    template<typename T>
    class ScopedLock
    {
    public:
        /**
         * Constructor. Locks the given object
         *
         * @param mutex The lockable object
         */
        explicit ScopedLock(T& lockable);

        /**
         * Destructor. Unlocks the object
         */
        ~ScopedLock();
    private:

        /**
         * Private copy constructor without implementation.
         *
         * This ensures that the ScopedLock cannot be copied which would result
         * in unlock called too often.
         *
         * @param other other ScopedLock
         */
        ScopedLock(const ScopedLock<T>& other);

        /**
         * Private assignment operator without implementation
         *
         * This ensures that the ScopedLock cannot be copied by an assignment which
         * would result in unlock called too often.
         *
         * @param other other ScopedLock
         * @return this
         */
        ScopedLock<T>& operator=(const ScopedLock<T>& other);

        T& m_lockable;
    };

    /**
     * Typedef for syntactic sugar when using with ramses_capu::Mutex.
     */
    typedef ScopedLock<Mutex> ScopedMutexLock;
    typedef ScopedLock<LightweightMutex> ScopedLightweightMutexLock;


    template<typename T>
    inline ScopedLock<T>::ScopedLock(T& lockable)
        : m_lockable(lockable)
    {
        m_lockable.lock();
    }

    template<typename T>
    inline ScopedLock<T>::~ScopedLock()
    {
        m_lockable.unlock();
    }
}

#endif // RAMSES_CAPU_SCOPED_LOCK_H
