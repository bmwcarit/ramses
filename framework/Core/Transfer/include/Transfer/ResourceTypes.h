//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCETYPES_H
#define RAMSES_RESOURCETYPES_H

#include "SceneAPI/Handles.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Collections/Vector.h"
#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    typedef std::vector<ResourceContentHash> ResourceContentHashVector;
    typedef StronglyTypedValue<UInt32, static_cast<UInt32>(-1), struct RequesterIDTag> RequesterID;
    typedef std::vector<RenderTargetHandle> RenderTargetHandleVector;
}

#endif
