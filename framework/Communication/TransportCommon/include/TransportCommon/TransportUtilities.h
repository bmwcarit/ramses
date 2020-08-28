//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSPORTUTILITIES_H
#define RAMSES_TRANSPORTUTILITIES_H

#include <functional>
#include <cstdint>

namespace ramses_internal
{
    namespace TransportUtilities
    {
        bool SplitToChunks(uint32_t maxNumberOfItemsPerMessage, uint32_t totalNumberOfItems, const std::function<bool(uint32_t, uint32_t)>& sendChunkFun);
    }
}

#endif
