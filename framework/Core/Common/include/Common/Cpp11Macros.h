//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CPP11MACROS_H
#define RAMSES_CPP11MACROS_H

// better for loop workaround until range based for loop is supported everywhere
#define ramses_foreach(Iteratable, Iter)\
    for(auto Iter = (Iteratable).begin(), \
        _##Iter##_end = (Iteratable).end(); \
        Iter != _##Iter##_end; ++Iter)

#endif
