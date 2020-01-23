//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceProviderMock.h"

namespace ramses_internal {

const ResourceContentHash ResourceProviderMock::FakeVertArrayHash(123u, 0);
const ResourceContentHash ResourceProviderMock::FakeVertArrayHash2(124u, 0);
const ResourceContentHash ResourceProviderMock::FakeIndexArrayHash(125u, 0);
const ResourceContentHash ResourceProviderMock::FakeIndexArrayHash2(126u, 0);
const ResourceContentHash ResourceProviderMock::FakeTextureHash(127u, 0);
const ResourceContentHash ResourceProviderMock::FakeTextureHash2(128u, 0);

const EffectResource ResourceProviderMock::dummyEffectResource("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
const ResourceContentHash ResourceProviderMock::FakeEffectHash(ResourceProviderMock::dummyEffectResource.getHash());

ResourceProviderMock::ResourceProviderMock()
    : vertArrayResource(EResourceType_VertexArray, 0, EDataType_Float, nullptr, ResourceCacheFlag_DoNotCache, String())
    , vertArrayResource2(EResourceType_VertexArray, 0, EDataType_Float, nullptr, ResourceCacheFlag_DoNotCache, String())
    , indexArrayResource(EResourceType_IndexArray, 0, EDataType_UInt16, nullptr, ResourceCacheFlag_DoNotCache, String())
    , indexArrayResource2(EResourceType_IndexArray, 0, EDataType_UInt16, nullptr, ResourceCacheFlag_DoNotCache, String())
    , textureResource(EResourceType_Texture2D, TextureMetaInfo(1u, 1u, 1u, ETextureFormat_R8, false, {}, { 1u }), ResourceCacheFlag_DoNotCache, String())
    , textureResource2(EResourceType_Texture2D, TextureMetaInfo(2u, 2u, 1u, ETextureFormat_R8, true, {}, { 4u }), ResourceCacheFlag_DoNotCache, String())
    , indexArrayIsAvailable(true)
    , deleterMock(mock)
{
    vertArrayResource.setResourceData(ResourceBlob(1), FakeVertArrayHash);
    vertArrayResource2.setResourceData(ResourceBlob(1), FakeVertArrayHash2);
    indexArrayResource.setResourceData(ResourceBlob(1), FakeIndexArrayHash);
    indexArrayResource2.setResourceData(ResourceBlob(1), FakeIndexArrayHash2);
    textureResource.setResourceData(ResourceBlob(1), FakeTextureHash);
    textureResource2.setResourceData(ResourceBlob(1), FakeTextureHash2);

    ON_CALL(*this, requestResourceAsyncronouslyFromFramework(_, _, _))
        .WillByDefault(Invoke([&](const auto& ids, auto, auto) {
                                  requestedResources.insert(requestedResources.end(), ids.begin(), ids.end());
                              }));
    EXPECT_CALL(*this, popArrivedResources(_)).Times(AnyNumber()).WillRepeatedly(Invoke(this, &ResourceProviderMock::fakePopArrivedResources));
}

ResourceProviderMock::~ResourceProviderMock()
{
}
}
