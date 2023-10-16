//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"

namespace ramses::internal
{
    class ClientScene;

    namespace DataSlotUtils
    {
        bool HasDataSlotId(const ClientScene& scene, DataSlotId id);
        bool HasDataSlotIdForNode(const ClientScene& scene, NodeHandle handle);
        bool HasDataSlotIdForDataObject(const ClientScene& scene, DataInstanceHandle dataRef);
        bool HasDataSlotIdForTextureSampler(const ClientScene& scene, TextureSamplerHandle sampler);
        bool HasDataSlotIdForTexture(const ClientScene& scene, const ResourceContentHash& texture);
    }
}
