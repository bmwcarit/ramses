//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEPROVIDERMOCK_H
#define RAMSES_RESOURCEPROVIDERMOCK_H

#include "renderer_common_gmock_header.h"
#include "RendererFramework/IResourceProvider.h"
#include "ResourceMock.h"
#include "Resource/ArrayResource.h"
#include "Resource/TextureResource.h"
#include "Resource/EffectResource.h"
#include "Components/IManagedResourceDeleterCallback.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Components/ManagedResource.h"
#include "RendererLib/IResourceDeviceHandleAccessor.h"

namespace ramses_internal {

class ResourceProviderMock : public IResourceProvider
{
public:
    static constexpr ResourceContentHash FakeEffectHash{ 120u, 0u };
    static constexpr ResourceContentHash FakeVertArrayHash{ 123u, 0u };
    static constexpr ResourceContentHash FakeVertArrayHash2{ 124u, 0u };
    static constexpr ResourceContentHash FakeIndexArrayHash{ 125u, 0u };
    static constexpr ResourceContentHash FakeIndexArrayHash2{ 126u, 0u };
    static constexpr ResourceContentHash FakeIndexArrayHash3{ 127u, 0u };
    static constexpr ResourceContentHash FakeTextureHash{ 128u, 0u };
    static constexpr ResourceContentHash FakeTextureHash2{ 129u, 0u };

    ResourceProviderMock();
    virtual ~ResourceProviderMock();

    MOCK_METHOD(void, cancelResourceRequest, (const ResourceContentHash& hash, const ResourceRequesterID& requesterID), (override));
    MOCK_METHOD(void, requestResourceAsyncronouslyFromFramework, (const ResourceContentHashVector& ids, const ResourceRequesterID& /*requesterID*/, const SceneId& /*providerID*/), (override));
    MOCK_METHOD(ManagedResourceVector, popArrivedResources, (const ResourceRequesterID& /*requesterID*/), (override));

    ManagedResourceVector fakePopArrivedResources(const ResourceRequesterID& /*requesterID*/)
    {
        ResourceContentHashVector arrivedResourceHashes;
        ManagedResourceVector arrivedResources;
        for (const auto& resource : requestedResources)
        {
            if ((resource == FakeIndexArrayHash || resource == FakeIndexArrayHash2 || resource == FakeIndexArrayHash3) && !indexArrayIsAvailable)
                continue;

            const auto mr = GetFakeManagedResource(resource);
            if (mr)
            {
                arrivedResources.push_back(mr);
                arrivedResourceHashes.push_back(resource);
            }
        }

        for (const auto& hash : arrivedResourceHashes)
            requestedResources.erase(find_c(requestedResources, hash));

        return arrivedResources;
    }

    static ManagedResource GetFakeManagedResource(const ResourceContentHash& hash)
    {
        std::unique_ptr<ResourceBase> res;
        if (hash == FakeEffectHash)
            res.reset(new EffectResource("", "", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache));
        else if (hash == FakeVertArrayHash || hash == FakeVertArrayHash2)
            res.reset(new ArrayResource(EResourceType_VertexArray, 0, EDataType::Float, nullptr, ResourceCacheFlag_DoNotCache, String()));
        else if (hash == FakeIndexArrayHash || hash == FakeIndexArrayHash2 || hash == FakeIndexArrayHash3)
            res.reset(new ArrayResource(EResourceType_IndexArray, 0, EDataType::UInt16, nullptr, ResourceCacheFlag_DoNotCache, String()));
        else if (hash == FakeTextureHash)
            res.reset(new TextureResource(EResourceType_Texture2D, TextureMetaInfo(1u, 1u, 1u, ETextureFormat::R8, false, {}, { 1u }), ResourceCacheFlag_DoNotCache, String()));
        else if (hash == FakeTextureHash2)
            res.reset(new TextureResource(EResourceType_Texture2D, TextureMetaInfo(2u, 2u, 1u, ETextureFormat::R8, true, {}, { 4u }), ResourceCacheFlag_DoNotCache, String()));

        if (res)
            res->setResourceData(ResourceBlob{ 1 }, hash);

        return ManagedResource{ res.release() };
    }

    void setIndexArrayAvailability(ramses_internal::Bool available)
    {
        indexArrayIsAvailable = available;
    }

private:
    bool indexArrayIsAvailable = true;
    ResourceContentHashVector requestedResources;
};

class ResourceDeviceHandleAccessorMock : public IResourceDeviceHandleAccessor
{
public:
    MOCK_METHOD(DeviceResourceHandle, getClientResourceDeviceHandle, (const ResourceContentHash& resourceHash), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getRenderTargetDeviceHandle, (RenderTargetHandle targetHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getRenderTargetBufferDeviceHandle, (RenderBufferHandle bufferHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(void, getBlitPassRenderTargetsDeviceHandle, (BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle&, DeviceResourceHandle&), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getOffscreenBufferDeviceHandle, (OffscreenBufferHandle bufferHandle), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getOffscreenBufferColorBufferDeviceHandle, (OffscreenBufferHandle bufferHandle), (const, override));
    MOCK_METHOD(OffscreenBufferHandle, getOffscreenBufferHandle, (DeviceResourceHandle bufferDeviceHandle), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getDataBufferDeviceHandle, (DataBufferHandle dataBufferHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getTextureBufferDeviceHandle, (TextureBufferHandle textureBufferHandle, SceneId sceneId), (const, override));
    MOCK_METHOD(DeviceResourceHandle, getTextureSamplerDeviceHandle, (TextureSamplerHandle textureSamplerHandle, SceneId sceneId), (const, override));
};
}
#endif
