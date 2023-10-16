//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include <vector>

namespace ramses::internal
{
    enum class SceneReferenceActionType : uint8_t
    {
        LinkData,
        UnlinkData
    };

    const std::array SceneReferenceActionTypeNames =
    {
        "LinkData",
        "UnlinkData",
    };

    struct SceneReferenceAction
    {
        SceneReferenceActionType type = SceneReferenceActionType::LinkData;
        SceneReferenceHandle consumerScene;
        DataSlotId consumerId;
        SceneReferenceHandle providerScene;
        DataSlotId providerId;
    };

    inline bool operator==(const SceneReferenceAction& a, const SceneReferenceAction& b)
    {
        return a.type == b.type
            && a.consumerScene == b.consumerScene
            && a.consumerId == b.consumerId
            && a.providerScene == b.providerScene
            && a.providerId == b.providerId;
    }

    using SceneReferenceActionVector = std::vector<SceneReferenceAction>;

    class SceneReferenceActionUtils
    {
    public:
        static void WriteToCollection(SceneReferenceActionVector const& actions, SceneActionCollection& collection);
        static void ReadFromCollection(SceneReferenceActionVector& actions, SceneActionCollection::SceneActionReader& reader);
        static size_t GetSizeEstimate(SceneReferenceActionVector const& actions);
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::SceneReferenceActionType,
                                        "SceneReferenceActionType",
                                        ramses::internal::SceneReferenceActionTypeNames,
                                        ramses::internal::SceneReferenceActionType::UnlinkData);
