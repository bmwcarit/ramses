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
    static const ResourceContentHash FakeVertArrayHash;
    static const ResourceContentHash FakeVertArrayHash2;
    static const ResourceContentHash FakeIndexArrayHash;
    static const ResourceContentHash FakeIndexArrayHash2;
    static const ResourceContentHash FakeTextureHash;
    static const ResourceContentHash FakeTextureHash2;
    static const ResourceContentHash FakeEffectHash;

    ResourceProviderMock();

    virtual ~ResourceProviderMock();

    MOCK_METHOD(void, cancelResourceRequest, (const ResourceContentHash& hash, const ResourceRequesterID& requesterID), (override));
    MOCK_METHOD(void, requestResourceAsyncronouslyFromFramework, (const ResourceContentHashVector& ids, const ResourceRequesterID& /*requesterID*/, const SceneId& /*providerID*/), (override));
    MOCK_METHOD(ManagedResourceVector, popArrivedResources, (const ResourceRequesterID& /*requesterID*/), (override));

    virtual ManagedResourceVector fakePopArrivedResources(const ResourceRequesterID& /*requesterID*/)
    {
        ResourceContentHashVector arrivedResourceHashes;
        ManagedResourceVector arrivedResources;
        for(auto resource : requestedResources)
        {
            if (resource == FakeVertArrayHash)
            {
                arrivedResources.push_back(ManagedResource(vertArrayResource, deleterMock));
                arrivedResourceHashes.push_back(resource);
            }
            if (resource == FakeVertArrayHash2)
            {
                arrivedResources.push_back(ManagedResource(vertArrayResource2, deleterMock));
                arrivedResourceHashes.push_back(resource);
            }
            if (resource == FakeEffectHash)
            {
                arrivedResources.push_back(ManagedResource(dummyEffectResource, deleterMock));
                arrivedResourceHashes.push_back(resource);
            }
            if (resource == FakeTextureHash)
            {
                arrivedResources.push_back(ManagedResource(textureResource, deleterMock));
                arrivedResourceHashes.push_back(resource);
            }
            if (resource == FakeTextureHash2)
            {
                arrivedResources.push_back(ManagedResource(textureResource2, deleterMock));
                arrivedResourceHashes.push_back(resource);
            }
            if (resource == FakeIndexArrayHash && indexArrayIsAvailable)
            {
                arrivedResources.push_back(ManagedResource(indexArrayResource, deleterMock));
                arrivedResourceHashes.push_back(resource);
            }
            if (resource == FakeIndexArrayHash2 && indexArrayIsAvailable)
            {
                arrivedResources.push_back(ManagedResource(indexArrayResource2, deleterMock));
                arrivedResourceHashes.push_back(resource);
            }
        }

        for (const auto hash : arrivedResourceHashes)
        {
            auto it = find_c(requestedResources, hash);
            requestedResources.erase(it);
        }

        return arrivedResources;
    }

    void setIndexArrayAvailability(ramses_internal::Bool available)
    {
        indexArrayIsAvailable = available;
    }

private:
    ArrayResource vertArrayResource;
    ArrayResource vertArrayResource2;
    ArrayResource indexArrayResource;
    ArrayResource indexArrayResource2;
    TextureResource textureResource;
    TextureResource textureResource2;
    static const EffectResource dummyEffectResource;

    ramses_internal::Bool indexArrayIsAvailable;

    NiceMock<ManagedResourceDeleterCallbackMock> mock;
    ResourceDeleterCallingCallback deleterMock;

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
