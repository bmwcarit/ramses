//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_SCOPEDPOINTER_H
#define RAMSES_SCOPEDPOINTER_H

#include <memory>

namespace ramses_internal
{
    template <typename T>
    using ScopedPointer = std::unique_ptr<T>;
}

#endif
