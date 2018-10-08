//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERRESOURCECACHEMOCK_H
#define RAMSES_RENDERERRESOURCECACHEMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IRendererResourceCache.h"
#include "SceneAPI/ResourceContentHash.h"

namespace ramses_internal{

class RendererResourceCacheMock : public IRendererResourceCache
{
public:

    MOCK_CONST_METHOD2(hasResource, bool(ResourceContentHash, UInt32&));
    MOCK_CONST_METHOD3(getResourceData, bool(ResourceContentHash, UInt8*, UInt32));
    MOCK_CONST_METHOD4(shouldResourceBeCached, bool(ResourceContentHash, UInt32, ResourceCacheFlag, SceneId));
    MOCK_METHOD5(storeResource, void(ResourceContentHash, const UInt8*, UInt32, ResourceCacheFlag, SceneId));
};
}
#endif
