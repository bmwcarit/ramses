//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_REFCOUNTED_H
#define RAMSES_REFCOUNTED_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <atomic>

namespace ramses_internal
{

    /**
     * Class for counting the references hold on a single instance.
     */
    class RefCounted
    {

    public:
        /**
         * Default constructor.
         */
        RefCounted();

        /**
         * Virtual destructor.
         */
        virtual ~RefCounted();

        /**
         * Add an additional reference to this instance.
         * @return  The number of references after this increment, not the current reference count.
         */
        UInt32 addRef();
        /**
         * Release a reference of this instance.
         * @return  The number of references after this decrement, not the current reference count.
         */
        UInt32 release();

        /**
         * Returns the current reference count of this instance
         * @return The actual reference count of this instanfe
         */
        UInt32 getReferenceCount() const;

    private:
        /**
         * The number of reference for this instance.
         */
        std::atomic<UInt32> m_refCount;
    };


}

#endif
