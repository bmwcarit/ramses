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
        void SplitToChunks(UInt32 maxNumberOfItemsPerMessage, UInt32 totalNumberOfItems, std::function<void(UInt32, UInt32)> sendChunkFun)
        {
            UInt32 offsetOfItemsInThisMessage = 0;
            while (offsetOfItemsInThisMessage < totalNumberOfItems)
            {
                const UInt32 itemsToSend = std::min(totalNumberOfItems - offsetOfItemsInThisMessage, maxNumberOfItemsPerMessage);
                assert(itemsToSend > 0);

                sendChunkFun(offsetOfItemsInThisMessage, offsetOfItemsInThisMessage + itemsToSend);
                offsetOfItemsInThisMessage += itemsToSend;
            }
        }

        void SplitSceneActionsToChunks(const SceneActionCollection& actions, UInt32 maxNumSceneActions, UInt32 maxSizeSceneActions,
                                       const std::function<void(std::pair<UInt32, UInt32>, std::pair<const Byte*, const Byte*>, bool)>& sendFunc)
        {
            assert(maxNumSceneActions > 0);
            assert(maxSizeSceneActions > 0);

            const UInt32 numActions = actions.numberOfActions();
            const Byte* data = actions.collectionData().data();
            UInt32 chunkStartAction = 0;
            UInt32 chunkOffsetStart = 0;
            UInt32 idx = 0;
            while (idx < numActions)
            {
                const UInt32 numActionsToSend = idx - chunkStartAction + 1;
                const UInt32 chunkOffsetEnd = actions[idx].offsetInCollection() + actions[idx].size();
                const UInt32 chunkSizeFullAction = chunkOffsetEnd - chunkOffsetStart;

                if (numActionsToSend == maxNumSceneActions ||
                    chunkSizeFullAction >= maxSizeSceneActions ||
                    idx == numActions - 1)
                {
                    const bool isIncomplete = chunkSizeFullAction > maxSizeSceneActions;
                    const UInt32 chunkSizeReal = std::min(chunkSizeFullAction, maxSizeSceneActions);
                    sendFunc(std::make_pair(chunkStartAction, chunkStartAction + numActionsToSend), std::make_pair(data + chunkOffsetStart, data + chunkOffsetStart + chunkSizeReal), isIncomplete);
                    if (isIncomplete)
                    {
                        chunkStartAction = idx;
                    }
                    else
                    {
                        chunkStartAction = idx + 1;
                        ++idx;
                    }
                    chunkOffsetStart += chunkSizeReal;
                }
                else
                {
                    ++idx;
                }
            }
        }
    }
}
