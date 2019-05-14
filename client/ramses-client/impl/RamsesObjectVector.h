//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTVECTOR_H
#define RAMSES_RAMSESOBJECTVECTOR_H

#include "Collections/Vector.h"
#include "Collections/HashSet.h"

namespace ramses
{
    class RamsesObject;
    class NodeImpl;

    typedef std::vector<RamsesObject*> RamsesObjectVector;
    typedef ramses_internal::HashSet<NodeImpl*> NodeImplSet;
}

#endif
