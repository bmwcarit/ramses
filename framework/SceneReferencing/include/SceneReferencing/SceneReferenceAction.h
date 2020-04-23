//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEREFERENCEACTION_H
#define RAMSES_SCENEREFERENCEACTION_H

#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/RendererSceneState.h"
#include "SceneAPI/DataSlot.h"
#include "Scene/SceneActionCollection.h"
#include <vector>

namespace ramses_internal
{
    enum class SceneReferenceActionType : uint8_t
    {
        LinkData,
        UnlinkData
    };

    struct SceneReferenceAction
    {
        SceneReferenceActionType type;
        SceneReferenceHandle consumerScene;
        DataSlotId consumerId;
        SceneReferenceHandle providerScene;
        DataSlotId providerId;
    };
    typedef std::vector<SceneReferenceAction> SceneReferenceActionVector;

    class SceneReferenceActionUtils
    {
    public:
        static void WriteToCollection(SceneReferenceActionVector const& actions, SceneActionCollection& collection);
        static void ReadFromCollection(SceneReferenceActionVector& actions, SceneActionCollection::SceneActionReader& reader);
        static size_t GetSizeEstimate(SceneReferenceActionVector const& actions);
    };
}

#endif
