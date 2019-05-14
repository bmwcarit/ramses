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

using namespace ramses_internal;

class BinaryShaderProviderMock : public IBinaryShaderCache
{
public:
    MOCK_CONST_METHOD1(hasBinaryShader, ramses_internal::Bool(ResourceContentHash));
    MOCK_CONST_METHOD1(getBinaryShaderSize, UInt32(ResourceContentHash));
    MOCK_CONST_METHOD1(getBinaryShaderFormat, UInt32(ResourceContentHash));
    MOCK_CONST_METHOD1(shouldBinaryShaderBeCached, ramses_internal::Bool(ResourceContentHash));
    MOCK_CONST_METHOD3(getBinaryShaderData, void(ResourceContentHash, UInt8*, UInt32));
    MOCK_METHOD4(storeBinaryShader, void(ResourceContentHash, const UInt8*, UInt32, UInt32));
    MOCK_CONST_METHOD2(binaryShaderUploaded, void(ResourceContentHash, ramses_internal::Bool));
};

class BinaryShaderProviderFake : public BinaryShaderProviderMock
{
public:
    BinaryShaderProviderFake()
    : m_effectHash()
    , m_binaryShaderData(NULL)
    , m_binaryShaderDataSize(0)
    , m_binaryShaderFormat(0)
    {
        ON_CALL(*this, shouldBinaryShaderBeCached(_)).WillByDefault(Return(true));
    }

    virtual ramses_internal::Bool hasBinaryShader(ResourceContentHash effectHash) const
    {
        BinaryShaderProviderMock::hasBinaryShader(effectHash);
        if (effectHash == m_effectHash)
        {
            return true;
        }

        return false;
    }

    virtual UInt32 getBinaryShaderSize(ResourceContentHash effectHash) const
    {
        BinaryShaderProviderMock::getBinaryShaderSize(effectHash);
        if (effectHash == m_effectHash)
        {
            return m_binaryShaderDataSize;
        }

        return 0;
    }

    virtual UInt32 getBinaryShaderFormat(ResourceContentHash effectHash) const
    {
        BinaryShaderProviderMock::getBinaryShaderFormat(effectHash);
        if (effectHash == m_effectHash)
        {
            return m_binaryShaderFormat;
        }

        return 0;
    }

    ResourceContentHash m_effectHash;
    const UInt8* m_binaryShaderData;
    UInt32 m_binaryShaderDataSize;
    UInt32 m_binaryShaderFormat;
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
    const ArrayResource res(EResourceType_VertexArray, 0, EDataType_Float, 0, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData()->getRawData(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(res.getDecompressedDataSize(), vramSize);
}

TEST_F(AResourceUploader, uploadsIndexArrayResource16)
{
    const ArrayResource res(EResourceType_IndexArray, 0, EDataType_UInt16, 0, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateIndexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadIndexBufferData(DeviceResourceHandle(123), res.getResourceData()->getRawData(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(res.getDecompressedDataSize(), vramSize);
}

TEST_F(AResourceUploader, uploadsTexture2DResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 2u, 1u, ETextureFormat_R8, false, MipDataSizeVector(mipCount));
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(2u, 2u, _, mipCount, 5u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 1u, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, 0u, 1u, 1u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(2u * 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture2DResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 1u, ETextureFormat_R8, true, MipDataSizeVector(1u));
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(4u, 1u, _, 3u, 7u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 4u, 1u, 1u, _, _));
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(4u + 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture3DResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 2u, 2u, ETextureFormat_R8, false, MipDataSizeVector(mipCount));
    TextureResource res(EResourceType_Texture3D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture3D(2u, 2u, 2u, _, mipCount, _)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 2u, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, 0u, 1u, 1u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(2u * 2 * 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture3DResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 2u, ETextureFormat_R8, true, MipDataSizeVector(1u));
    TextureResource res(EResourceType_Texture3D, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture3D(4u, 1u, 2u, _, 3u, _)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 4u, 1u, 2u, _, _));
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(4u * 1 * 2 + 2 * 1 * 1 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTextureCubeResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 1u, 1u, ETextureFormat_R8, false, MipDataSizeVector(mipCount));
    TextureResource res(EResourceType_TextureCube, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, allocateTextureCube(2u, _, mipCount, _)).WillOnce(Return(DeviceResourceHandle(123)));
    for (UInt32 i = 0u; i < 6u; ++i)
    {
        EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, i, 2u, 2u, 1u, _, _));
        EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, i, 1u, 1u, 1u, _, _));
    }
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(6u * (2 * 2 + 1), vramSize);
}

TEST_F(AResourceUploader, uploadsTextureCubeResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 1u, ETextureFormat_R8, true, MipDataSizeVector(1u));
    TextureResource res(EResourceType_TextureCube, texDesc, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, allocateTextureCube(4u, _, 3u, _)).WillOnce(Return(DeviceResourceHandle(123)));
    for (UInt32 i = 0u; i < 6u; ++i)
    {
        EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, i, 4u, 4u, 1u, _, _));
    }
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(6u * (4 * 4 + 2 * 2 + 1), vramSize);
}

TEST_F(AResourceUploader, uploadsEffectResourceWithoutBinaryShaderCache)
{
    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));
    EXPECT_EQ(res.getDecompressedDataSize(), vramSize);
}

TEST_F(AResourceUploader, ifShaderShouldNotBeCachedNoDownloadWillHappen)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);

    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash())).WillOnce(Return(false));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(_, _, _, _)).Times(0);
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(_, _, _)).Times(0);
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, _)).Times(0); // This should only be called when attempting to upload from cache

    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, managedRes, vramSize));
}

TEST_F(AResourceUploader, uploadsEffectResourceWithBinaryShaderCacheWhenCacheMissingAndCachingSupported)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);

    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash())).WillOnce(Return(true));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(res.getHash(), _, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(_, _, _)).
        WillOnce(DoAll(SetArgReferee<1>(std::vector<UInt8>(1)), Return(true)));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, _)).Times(0); // This should only be called when attempting to upload from cache

    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, managedRes, vramSize));
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
    binaryShaderProvider.m_binaryShaderFormat = 12u;

    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderSize(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderFormat(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderData(res.getHash(), _, _));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, true)).Times(1);

    EXPECT_CALL(renderer.deviceMock, uploadBinaryShader(_, _, _, _)).WillOnce(Return(DeviceResourceHandle(123)));

    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, managedRes, vramSize));
}

TEST_F(AResourceUploader, uploadsEffectResourceWithBrokenBinaryShaderCache)
{
    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes(res, dummyManagedResourceCallback);

    UInt8 binaryShaderData[] = { 1u, 2u, 3u, 4u };

    BinaryShaderProviderFake binaryShaderProvider;
    binaryShaderProvider.m_effectHash = res.getHash();
    binaryShaderProvider.m_binaryShaderData = binaryShaderData;
    binaryShaderProvider.m_binaryShaderDataSize = sizeof(binaryShaderData) / sizeof(UInt8);
    binaryShaderProvider.m_binaryShaderFormat = 12u;

    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    // Set up so it looks like there is a valid binary shader cache
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderSize(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderFormat(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderData(res.getHash(), _, _));

    // Make the upload from cache fail and expect the callback telling that the cache was broken
    EXPECT_CALL(renderer.deviceMock, uploadBinaryShader(_, _, _, _)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, false)).Times(1);

    // Expect fallback to compiling from source and storing in the cache
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(_));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(_, _, _)).WillOnce(DoAll(SetArgReferee<1>(UInt8Vector(10)), Return(true)));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(_, _, _, _)).Times(1);

    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, managedRes, vramSize));
}

TEST_F(AResourceUploader, doesNotTryToGetBinaryShaderFromDeviceIfEffectDidNotCompile)
{
    EffectResource res("", "", EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes(res, dummyManagedResourceCallback);

    StrictMock<BinaryShaderProviderMock> binaryShaderProvider;

    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash())).Times(1);
    EXPECT_CALL(renderer.deviceMock, uploadShader(_)).Times(1);
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, _)).Times(0);

    ON_CALL(binaryShaderProvider, hasBinaryShader(_)).WillByDefault(Return(false));
    ON_CALL(renderer.deviceMock, uploadShader(_)).WillByDefault(Return(DeviceResourceHandle::Invalid()));

    ResourceUploader uploaderWithBinaryProvider(stats, &binaryShaderProvider);
    EXPECT_FALSE(uploaderWithBinaryProvider.uploadResource(renderer, managedRes, vramSize).isValid());
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

    const ArrayResource res(EResourceType_VertexArray, 0, EDataType_Float, 0, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData()->getRawData(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, managedRes, vramSize));

    EXPECT_CALL(additionalRenderer.deviceMock, allocateVertexBuffer(res.getElementType(), res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(additionalRenderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData()->getRawData(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(additionalRenderer, managedRes, vramSize));
}
