//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/TransportUtilities.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "Scene/SceneActionCollection.h"
#include "Utils/LogMacros.h"
#include <algorithm>

namespace ramses_internal
{
    namespace TransportUtilities
    {
        bool SplitToChunks(UInt32 maxNumberOfItemsPerMessage, UInt32 totalNumberOfItems, const std::function<bool(UInt32, UInt32)>& sendChunkFun)
        {
            UInt32 offsetOfItemsInThisMessage = 0;
            while (offsetOfItemsInThisMessage < totalNumberOfItems)
            {
                const UInt32 itemsToSend = std::min(totalNumberOfItems - offsetOfItemsInThisMessage, maxNumberOfItemsPerMessage);
                assert(itemsToSend > 0);

                if (!sendChunkFun(offsetOfItemsInThisMessage, offsetOfItemsInThisMessage + itemsToSend))
                    return false;
                offsetOfItemsInThisMessage += itemsToSend;
            }
            return true;
        }
    }
}
