//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/EDataSlotType.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/Core/Common/StronglyTypedValue.h"

namespace ramses::internal
{
    struct DataSlotIdTag {};
    using DataSlotId = StronglyTypedValue<uint32_t, 0, DataSlotIdTag>;

    struct DataSlot
    {
        EDataSlotType        type{EDataSlotType::Undefined};
        DataSlotId           id;
        NodeHandle           attachedNode;
        DataInstanceHandle   attachedDataReference;
        ResourceContentHash  attachedTexture;
        TextureSamplerHandle attachedTextureSampler;
    };
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::DataSlotId)
