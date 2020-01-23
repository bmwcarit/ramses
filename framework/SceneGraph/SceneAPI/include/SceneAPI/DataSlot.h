//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATASLOT_H
#define RAMSES_DATASLOT_H

#include "SceneAPI/EDataSlotType.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    struct DataSlotIdTag {};
    typedef StronglyTypedValue<UInt32, 0, DataSlotIdTag> DataSlotId;
    DEFINE_STRINGOUTPUTSTREAM_OPERATOR(DataSlotId)

    struct DataSlot
    {
        EDataSlotType        type;
        DataSlotId           id;
        NodeHandle           attachedNode;
        DataInstanceHandle   attachedDataReference;
        ResourceContentHash  attachedTexture;
        TextureSamplerHandle attachedTextureSampler;
    };
}

#endif
