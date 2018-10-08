//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENERESOURCEDATA_H
#define RAMSES_SCENERESOURCEDATA_H

#include "Utils/MemoryBlob.h"
#include "Utils/CompressedMemoryBlob.h"
#include "PlatformAbstraction/PlatformSharedPointer.h"
#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    typedef PlatformSharedPointer<MemoryBlob> SceneResourceData;
    typedef PlatformSharedPointer<CompressedMemoryBlob> CompressedSceneResourceData;

    struct ResourceCacheFlagTag {};
    typedef StronglyTypedValue<UInt32, static_cast<UInt32>(-1), ResourceCacheFlagTag> ResourceCacheFlag;
    static const ResourceCacheFlag ResourceCacheFlag_DoNotCache(ResourceCacheFlag::DefaultValue());
}

#endif
