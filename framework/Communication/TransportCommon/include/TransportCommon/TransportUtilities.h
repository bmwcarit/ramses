//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSPORTUTILITIES_H
#define RAMSES_TRANSPORTUTILITIES_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/Pair.h"
#include "Utils/Warnings.h"
#include <functional>

namespace ramses_internal
{
    class IResource;
    class SceneActionCollection;
    class String;
    struct CommunicationSendDataSizes;

    namespace TransportUtilities
    {
        void SplitToChunks(UInt32 maxNumberOfItemsPerMessage, UInt32 totalNumberOfItems, std::function<void(UInt32, UInt32)> sendChunkFun);
        void SplitSceneActionsToChunks(const SceneActionCollection& actions, UInt32 maxNumSceneActions, UInt32 maxSizeSceneActions,
                                       const std::function<void(std::pair<UInt32, UInt32>, std::pair<const Byte*, const Byte*>, bool)>& sendFunc);
    }
}

#endif
