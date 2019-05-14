//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECHANGEUTILS_H
#define RAMSES_RESOURCECHANGEUTILS_H

#include "Transfer/ResourceTypes.h"

namespace ramses_internal
{
    class ResourceChangeUtils
    {
    public:
        static void ConsolidateResource(ResourceContentHashVector& toAddIfNew, ResourceContentHashVector& toRemoveIfContained, const ResourceContentHash& res)
        {
            const auto it = find_c(toRemoveIfContained, res);
            if (it == toRemoveIfContained.end())
            {
                assert(!contains_c(toAddIfNew, res));
                toAddIfNew.push_back(res);
            }
            else
            {
                toRemoveIfContained.erase(it);
            }
        }
    };
}

#endif
