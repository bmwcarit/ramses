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

    MOCK_METHOD(bool, hasResource, (ResourceContentHash, UInt32&), (const, override));
    MOCK_METHOD(bool, getResourceData, (ResourceContentHash, UInt8*, UInt32), (const, override));
    MOCK_METHOD(bool, shouldResourceBeCached, (ResourceContentHash, UInt32, ResourceCacheFlag, SceneId), (const, override));
    MOCK_METHOD(void, storeResource, (ResourceContentHash, const UInt8*, UInt32, ResourceCacheFlag, SceneId), (override));
};
}
#endif
