//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMSHAREDPOINTER_H
#define RAMSES_PLATFORMSHAREDPOINTER_H

#include <memory>
#include "ramses-capu/container/Hash.h"

namespace ramses_internal
{
    template <typename T>
    using PlatformSharedPointer = std::shared_ptr<T>;
}

namespace ramses_capu
{
    /**
     * Specialization of Hash in order to calculate the Hash differently for shared_ptr
     */
    template<class T>
    struct Hash<std::shared_ptr<T>>
    {
        uint_t operator()(const std::shared_ptr<T>& key)
        {
            return Hash<T*>()(key.get());
        }
    };
}

#endif
