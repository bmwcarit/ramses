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

    MOCK_METHOD2(cancelResourceRequest, void(const ResourceContentHash& hash, const RequesterID& requesterID));
    MOCK_METHOD3(requestResourceAsyncronouslyFromFramework, void(const ResourceContentHashVector& ids, const RequesterID& /*requesterID*/, const SceneId& /*providerID*/));
    MOCK_METHOD1(popArrivedResources, ManagedResourceVector(const RequesterID& /*requesterID*/));

    virtual ManagedResourceVector fakePopArrivedResources(const RequesterID& /*requesterID*/)
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
    MOCK_CONST_METHOD1(getClientResourceDeviceHandle, DeviceResourceHandle(const ResourceContentHash& resourceHash));
    MOCK_CONST_METHOD2(getRenderTargetDeviceHandle, DeviceResourceHandle(RenderTargetHandle targetHandle, SceneId sceneId));
    MOCK_CONST_METHOD2(getRenderTargetBufferDeviceHandle, DeviceResourceHandle(RenderBufferHandle bufferHandle, SceneId sceneId));
    MOCK_CONST_METHOD4(getBlitPassRenderTargetsDeviceHandle, void(BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle&, DeviceResourceHandle&));
    MOCK_CONST_METHOD1(getOffscreenBufferDeviceHandle, DeviceResourceHandle(OffscreenBufferHandle bufferHandle));
    MOCK_CONST_METHOD1(getOffscreenBufferColorBufferDeviceHandle, DeviceResourceHandle(OffscreenBufferHandle bufferHandle));
    MOCK_CONST_METHOD1(getOffscreenBufferHandle, OffscreenBufferHandle(DeviceResourceHandle bufferDeviceHandle));
    MOCK_CONST_METHOD2(getDataBufferDeviceHandle, DeviceResourceHandle(DataBufferHandle dataBufferHandle, SceneId sceneId));
    MOCK_CONST_METHOD2(getTextureBufferDeviceHandle, DeviceResourceHandle(TextureBufferHandle textureBufferHandle, SceneId sceneId));
    MOCK_CONST_METHOD2(getTextureSamplerDeviceHandle, DeviceResourceHandle(TextureSamplerHandle textureSamplerHandle, SceneId sceneId));
};
}
#endif
