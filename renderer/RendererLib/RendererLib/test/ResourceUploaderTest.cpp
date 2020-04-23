//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererLib/RendererStatistics.h"
#include "RendererAPI/IBinaryShaderCache.h"
#include "RenderBackendMock.h"
#include "ResourceMock.h"
#include "Resource/EffectResource.h"
#include "Resource/TextureResource.h"
#include "Resource/EffectResource.h"
#include "Resource/ArrayResource.h"
#include "RendererLib/ResourceDescriptor.h"

using namespace ramses_internal;

class BinaryShaderProviderMock : public IBinaryShaderCache
{
public:
    MOCK_METHOD1(deviceSupportsBinaryShaderFormats, void(const std::vector<BinaryShaderFormatID>&));
    MOCK_CONST_METHOD1(hasBinaryShader, ramses_internal::Bool(ResourceContentHash));
    MOCK_CONST_METHOD1(getBinaryShaderSize, UInt32(ResourceContentHash));
    MOCK_CONST_METHOD1(getBinaryShaderFormat, BinaryShaderFormatID(ResourceContentHash));
    MOCK_CONST_METHOD2(shouldBinaryShaderBeCached, ramses_internal::Bool(ResourceContentHash, SceneId));
    MOCK_CONST_METHOD3(getBinaryShaderData, void(ResourceContentHash, UInt8*, UInt32));
    MOCK_METHOD5(storeBinaryShader, void(ResourceContentHash, SceneId, const UInt8*, UInt32, BinaryShaderFormatID));
    MOCK_CONST_METHOD2(binaryShaderUploaded, void(ResourceContentHash, ramses_internal::Bool));
};

class BinaryShaderProviderFake final : public StrictMock<BinaryShaderProviderMock>
{
public:
    BinaryShaderProviderFake()
    : m_effectHash()
    , m_binaryShaderData(nullptr)
    , m_binaryShaderDataSize(0)
    , m_binaryShaderFormat(0)
    {
        ON_CALL(*this, shouldBinaryShaderBeCached(_,_)).WillByDefault(Return(true));
    }

    virtual ramses_internal::Bool hasBinaryShader(ResourceContentHash effectHash) const override
    {
        BinaryShaderProviderMock::hasBinaryShader(effectHash);
        if (effectHash == m_effectHash)
        {
            return true;
        }

        return false;
    }

    virtual UInt32 getBinaryShaderSize(ResourceContentHash effectHash) const override
    {
        BinaryShaderProviderMock::getBinaryShaderSize(effectHash);
        if (effectHash == m_effectHash)
        {
            return m_binaryShaderDataSize;
        }

        return 0;
    }

    virtual BinaryShaderFormatID getBinaryShaderFormat(ResourceContentHash effectHash) const override
    {
        BinaryShaderProviderMock::getBinaryShaderFormat(effectHash);
        return (effectHash == m_effectHash ? m_binaryShaderFormat : BinaryShaderFormatID::Invalid());
    }

    ResourceContentHash m_effectHash;
    const UInt8* m_binaryShaderData;
    UInt32 m_binaryShaderDataSize;
    BinaryShaderFormatID m_binaryShaderFormat;
};

class AResourceUploader : public ::testing::Test
{
public:
    AResourceUploader()
        : uploader(stats)
        , dummyManagedResourceCallback(managedResourceDeleter)
    {
    }

    StrictMock<RenderBackendStrictMock> renderer;
    RendererStatistics stats;
    ResourceUploader uploader;

    StrictMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
    ResourceDeleterCallingCallback dummyManagedResourceCallback;
    UInt32 vramSize = 0;
};

TEST_F(AResourceUploader, uploadsVertexArrayResource)
{
    const ArrayResource res(EResourceType_VertexArray, 1, EDataType_Float, nullptr, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData().data(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(res.getDecompressedDataSize(), vramSize);
}

TEST_F(AResourceUploader, uploadsIndexArrayResource16)
{
    const ArrayResource res(EResourceType_IndexArray, 1, EDataType_UInt16, nullptr, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateIndexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadIndexBufferData(DeviceResourceHandle(123), res.getResourceData().data(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(res.getDecompressedDataSize(), vramSize);
}

TEST_F(AResourceUploader, uploadsTexture2DResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 2u, 1u, ETextureFormat_R8, false, DefaultTextureSwizzleArray, { 4, 1 });
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(2u, 2u, ETextureFormat_R8, DefaultTextureSwizzleArray, mipCount, 5u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 1u, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, 0u, 1u, 1u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(2u * 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture2DResourceWithNonDefaultTextureSwizzleArray)
{
    const UInt32 mipCount = 1u;
    const TextureSwizzleArray swizzle {ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Green, ETextureChannelColor::Red};
    const TextureMetaInfo texDesc(2u, 2u, 1u, ETextureFormat_R8, false, swizzle, { 4 });
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(2u, 2u, ETextureFormat_R8, swizzle, mipCount, 4u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(2u * 2, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture2DResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 1u, ETextureFormat_R8, true, DefaultTextureSwizzleArray, { 4 });
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(4u, 1u, ETextureFormat_R8, DefaultTextureSwizzleArray, 3u, 7u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 4u, 1u, 1u, _, _));
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(4u + 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture3DResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 2u, 2u, ETextureFormat_R8, false, DefaultTextureSwizzleArray, { 8, 1 });
    TextureResource res(EResourceType_Texture3D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture3D(2u, 2u, 2u, ETextureFormat_R8, mipCount, 9u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 2u, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, 0u, 1u, 1u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(2u * 2 * 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture3DResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 2u, ETextureFormat_R8, true, DefaultTextureSwizzleArray, { 8 });
    TextureResource res(EResourceType_Texture3D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture3D(4u, 1u, 2u, ETextureFormat_R8, 3u, 11u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 4u, 1u, 2u, _, _));
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(4u * 1 * 2 + 2 * 1 * 1 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTextureCubeResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 1u, 1u, ETextureFormat_R8, false, DefaultTextureSwizzleArray, { 4, 1 });
    TextureResource res(EResourceType_TextureCube, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, allocateTextureCube(2u, ETextureFormat_R8, DefaultTextureSwizzleArray, mipCount, 6 * 5)).WillOnce(Return(DeviceResourceHandle(123)));
    for (UInt32 i = 0u; i < 6u; ++i)
    {
        EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, i, 2u, 2u, 1u, _, _));
        EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, i, 1u, 1u, 1u, _, _));
    }
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(6u * (2 * 2 + 1), vramSize);
}

TEST_F(AResourceUploader, uploadsTextureCubeResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 1u, ETextureFormat_R8, true, DefaultTextureSwizzleArray, { 16 });
    TextureResource res(EResourceType_TextureCube, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, allocateTextureCube(4u, ETextureFormat_R8, DefaultTextureSwizzleArray, 3u, 6 * (16 + 4 + 1))).WillOnce(Return(DeviceResourceHandle(123)));
    for (UInt32 i = 0u; i < 6u; ++i)
    {
        EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, i, 4u, 4u, 1u, _, _));
    }
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(6u * (4 * 4 + 2 * 2 + 1), vramSize);
}

TEST_F(AResourceUploader, uploadsEffectResourceWithoutBinaryShaderCache)
{
    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(res.getDecompressedDataSize(), vramSize);
}

TEST_F(AResourceUploader, ifShaderShouldNotBeCachedNoDownloadWillHappen)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);

    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash(),_)).WillOnce(Return(false));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(_, _, _, _, _)).Times(0);
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(_, _, _)).Times(0);
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, _)).Times(0); // This should only be called when attempting to upload from cache

    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize));
}

TEST_F(AResourceUploader, uploadsEffectResourceWithBinaryShaderCacheWhenCacheMissingAndCachingSupported)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);

    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash(),_)).WillOnce(Return(true));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(res.getHash(), _, _, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(_, _, _)).
        WillOnce(DoAll(SetArgReferee<1>(std::vector<UInt8>(1)), Return(true)));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, _)).Times(0); // This should only be called when attempting to upload from cache

    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize));
}

TEST_F(AResourceUploader, uploadsEffectResourceWithBinaryShaderCacheWhenCacheHit)
{
    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes(res, dummyManagedResourceCallback);

    UInt8 binaryShaderData[] = { 1u, 2u, 3u, 4u };

    BinaryShaderProviderFake binaryShaderProvider;
    binaryShaderProvider.m_effectHash = res.getHash();
    binaryShaderProvider.m_binaryShaderData = binaryShaderData;
    binaryShaderProvider.m_binaryShaderDataSize = sizeof(binaryShaderData) / sizeof(UInt8);
    binaryShaderProvider.m_binaryShaderFormat = BinaryShaderFormatID{ 12u };

    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderSize(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderFormat(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderData(res.getHash(), _, _));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, true)).Times(1);

    EXPECT_CALL(renderer.deviceMock, uploadBinaryShader(_, _, _, _)).WillOnce(Return(DeviceResourceHandle(123)));

    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize));
}

TEST_F(AResourceUploader, uploadsEffectResourceWithBrokenBinaryShaderCache)
{
    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    SceneId sceneUsingResource(14);

    UInt8 binaryShaderData[] = { 1u, 2u, 3u, 4u };

    BinaryShaderProviderFake binaryShaderProvider;
    binaryShaderProvider.m_effectHash = res.getHash();
    binaryShaderProvider.m_binaryShaderData = binaryShaderData;
    binaryShaderProvider.m_binaryShaderDataSize = sizeof(binaryShaderData) / sizeof(UInt8);
    binaryShaderProvider.m_binaryShaderFormat = BinaryShaderFormatID{ 12u };

    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    // Set up so it looks like there is a valid binary shader cache
    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderSize(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderFormat(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderData(res.getHash(), _, _));

    // Make the upload from cache fail and expect the callback telling that the cache was broken
    EXPECT_CALL(renderer.deviceMock, uploadBinaryShader(_, _, _, _)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, false)).Times(1);

    // Expect fallback to compiling from source and storing in the cache
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(_, sceneUsingResource));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(_, _, _)).WillOnce(DoAll(SetArgReferee<1>(UInt8Vector(10)), Return(true)));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(_, sceneUsingResource, _, _, _)).Times(1);

    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    resourceObject.sceneUsage = { sceneUsingResource };
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize));
}

TEST_F(AResourceUploader, doesNotTryToGetBinaryShaderFromDeviceIfEffectDidNotCompile)
{
    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes(res, dummyManagedResourceCallback);

    StrictMock<BinaryShaderProviderMock> binaryShaderProvider;

    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash())).Times(1);
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).Times(1);
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, _)).Times(0);

    ON_CALL(binaryShaderProvider, hasBinaryShader(_)).WillByDefault(Return(false));
    ON_CALL(renderer.deviceMock, uploadShader(_)).WillByDefault(Return(DeviceResourceHandle::Invalid()));

    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_FALSE(uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize).isValid());
}

TEST_F(AResourceUploader, providesSupportedBinaryShaderFormatsOnlyOnce)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);

    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(3);

    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat })).Times(1);

    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash())).Times(3);
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash(), _)).Times(3).WillRepeatedly(Return(false));
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).Times(3).WillRepeatedly(Return(DeviceResourceHandle(123)));

    ResourceDescriptor resourceObject1;
    resourceObject1.resource = ManagedResource{ res, dummyManagedResourceCallback };
    resourceObject1.sceneUsage = { SceneId{1u} };
    ResourceDescriptor resourceObject2 = resourceObject1;
    resourceObject2.resource = ManagedResource{ res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject3 = resourceObject1;
    resourceObject3.resource = ManagedResource{ res, dummyManagedResourceCallback };

    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject1, vramSize));
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject2, vramSize));
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject3, vramSize));
}

TEST_F(AResourceUploader, unloadsVertexArrayResource)
{
    const DeviceResourceHandle handle(123u);
    EXPECT_CALL(renderer.deviceMock, deleteVertexBuffer(handle));
    uploader.unloadResource(renderer, EResourceType_VertexArray, ResourceContentHash(456u, 0), handle);
}

TEST_F(AResourceUploader, unloadsIndexArrayResource)
{
    const DeviceResourceHandle handle(123u);
    EXPECT_CALL(renderer.deviceMock, deleteIndexBuffer(handle));
    uploader.unloadResource(renderer, EResourceType_IndexArray, ResourceContentHash(456u, 0), handle);
}

TEST_F(AResourceUploader, unloadsTexture2DResource)
{
    const DeviceResourceHandle handle(123u);
    EXPECT_CALL(renderer.deviceMock, deleteTexture(handle));
    uploader.unloadResource(renderer, EResourceType_Texture2D, ResourceContentHash(456u, 0), handle);
}

TEST_F(AResourceUploader, unloadsTexture3DResource)
{
    const DeviceResourceHandle handle(123u);
    EXPECT_CALL(renderer.deviceMock, deleteTexture(handle));
    uploader.unloadResource(renderer, EResourceType_Texture3D, ResourceContentHash(456u, 0), handle);
}

TEST_F(AResourceUploader, unloadsTextureCubeResource)
{
    const DeviceResourceHandle handle(123u);
    EXPECT_CALL(renderer.deviceMock, deleteTexture(handle));
    uploader.unloadResource(renderer, EResourceType_TextureCube, ResourceContentHash(456u, 0), handle);
}

TEST_F(AResourceUploader, unloadsEffectResource)
{
    const DeviceResourceHandle handle(123u);
    EXPECT_CALL(renderer.deviceMock, deleteShader(handle));
    uploader.unloadResource(renderer, EResourceType_Effect, ResourceContentHash(456u, 0), handle);
}

TEST_F(AResourceUploader, uploadsWithMultipleRenderBackends)
{
    StrictMock<RenderBackendStrictMock> additionalRenderer;

    const ArrayResource res(EResourceType_VertexArray, 1, EDataType_Float, nullptr, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData().data(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));

    EXPECT_CALL(additionalRenderer.deviceMock, allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(additionalRenderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData().data(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(additionalRenderer, resourceObject, vramSize));
}
