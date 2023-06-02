//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererAPI/IBinaryShaderCache.h"
#include "RenderBackendMock.h"
#include "ResourceMock.h"
#include "Resource/EffectResource.h"
#include "Resource/TextureResource.h"
#include "Resource/EffectResource.h"
#include "Resource/ArrayResource.h"
#include "RendererLib/ResourceDescriptor.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Utils/ThreadBarrier.h"
#include "Utils/ThreadLocalLog.h"
#include <thread>

using namespace ramses_internal;

class BinaryShaderProviderMock : public IBinaryShaderCache
{
public:
    MOCK_METHOD(void, deviceSupportsBinaryShaderFormats, (const std::vector<BinaryShaderFormatID>&), (override));
    MOCK_METHOD(bool, hasBinaryShader, (ResourceContentHash), (const, override));
    MOCK_METHOD(UInt32, getBinaryShaderSize, (ResourceContentHash), (const, override));
    MOCK_METHOD(BinaryShaderFormatID, getBinaryShaderFormat, (ResourceContentHash), (const, override));
    MOCK_METHOD(bool, shouldBinaryShaderBeCached, (ResourceContentHash, SceneId), (const, override));
    MOCK_METHOD(void, getBinaryShaderData, (ResourceContentHash, uint8_t*, UInt32), (const, override));
    MOCK_METHOD(void, storeBinaryShader, (ResourceContentHash, SceneId, const uint8_t*, UInt32, BinaryShaderFormatID), (override));
    MOCK_METHOD(void, binaryShaderUploaded, (ResourceContentHash, bool), (const, override));
    MOCK_METHOD(std::once_flag&, binaryShaderFormatsReported, (), (override));
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
        ON_CALL(*this, binaryShaderFormatsReported()).WillByDefault(ReturnRef(m_binaryShaderFormatReported));
    }

    [[nodiscard]] bool hasBinaryShader(ResourceContentHash effectHash) const override
    {
        BinaryShaderProviderMock::hasBinaryShader(effectHash);
        if (effectHash == m_effectHash)
        {
            return true;
        }

        return false;
    }

    [[nodiscard]] UInt32 getBinaryShaderSize(ResourceContentHash effectHash) const override
    {
        BinaryShaderProviderMock::getBinaryShaderSize(effectHash);
        if (effectHash == m_effectHash)
        {
            return m_binaryShaderDataSize;
        }

        return 0;
    }

    [[nodiscard]] BinaryShaderFormatID getBinaryShaderFormat(ResourceContentHash effectHash) const override
    {
        BinaryShaderProviderMock::getBinaryShaderFormat(effectHash);
        return (effectHash == m_effectHash ? m_binaryShaderFormat : BinaryShaderFormatID::Invalid());
    }

    ResourceContentHash m_effectHash;
    const uint8_t* m_binaryShaderData;
    UInt32 m_binaryShaderDataSize;
    BinaryShaderFormatID m_binaryShaderFormat;
    std::once_flag m_binaryShaderFormatReported;
};

class AResourceUploader : public ::testing::Test
{
public:
    AResourceUploader()
        : uploader(true)
        , dummyManagedResourceCallback(managedResourceDeleter)
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);
    }

    StrictMock<RenderBackendStrictMock> renderer;
    ResourceUploader uploader;

    StrictMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
    ResourceDeleterCallingCallback dummyManagedResourceCallback;
    UInt32 vramSize = 0;
};

TEST_F(AResourceUploader, uploadsVertexArrayResource)
{
    const ArrayResource res(EResourceType_VertexArray, 1, EDataType::Float, nullptr, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData().data(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(res.getDecompressedDataSize(), vramSize);
}

TEST_F(AResourceUploader, uploadsIndexArrayResource16)
{
    const ArrayResource res(EResourceType_IndexArray, 1, EDataType::UInt16, nullptr, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
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
    const TextureMetaInfo texDesc(2u, 2u, 1u, ETextureFormat::R8, false, DefaultTextureSwizzleArray, { 4, 1 });
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(2u, 2u, ETextureFormat::R8, DefaultTextureSwizzleArray, mipCount, 5u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 1u, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, 0u, 1u, 1u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(2u * 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture2DResourceWithNonDefaultTextureSwizzleArray)
{
    const UInt32 mipCount = 1u;
    const TextureSwizzleArray swizzle {ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Green, ETextureChannelColor::Red};
    const TextureMetaInfo texDesc(2u, 2u, 1u, ETextureFormat::R8, false, swizzle, { 4 });
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(2u, 2u, ETextureFormat::R8, swizzle, mipCount, 4u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(2u * 2, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture2DResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 1u, ETextureFormat::R8, true, DefaultTextureSwizzleArray, { 4 });
    TextureResource res(EResourceType_Texture2D, texDesc, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture2D(4u, 1u, ETextureFormat::R8, DefaultTextureSwizzleArray, 3u, 7u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 4u, 1u, 1u, _, _));
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(4u + 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture3DResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 2u, 2u, ETextureFormat::R8, false, DefaultTextureSwizzleArray, { 8, 1 });
    TextureResource res(EResourceType_Texture3D, texDesc, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture3D(2u, 2u, 2u, ETextureFormat::R8, mipCount, 9u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 2u, 2u, 2u, _, _));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 1u, 0u, 0u, 0u, 1u, 1u, 1u, _, _));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(2u * 2 * 2 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTexture3DResourceWithMipGen)
{
    const TextureMetaInfo texDesc(4u, 1u, 2u, ETextureFormat::R8, true, DefaultTextureSwizzleArray, { 8 });
    TextureResource res(EResourceType_Texture3D, texDesc, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateTexture3D(4u, 1u, 2u, ETextureFormat::R8, 3u, 11u)).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, 0u, 4u, 1u, 2u, _, _));
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(4u * 1 * 2 + 2 * 1 * 1 + 1, vramSize);
}

TEST_F(AResourceUploader, uploadsTextureCubeResource)
{
    const UInt32 mipCount = 2u;
    const TextureMetaInfo texDesc(2u, 1u, 1u, ETextureFormat::R8, false, DefaultTextureSwizzleArray, { 4, 1 });
    TextureResource res(EResourceType_TextureCube, texDesc, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, allocateTextureCube(2u, ETextureFormat::R8, DefaultTextureSwizzleArray, mipCount, 6 * 5)).WillOnce(Return(DeviceResourceHandle(123)));
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
    const TextureMetaInfo texDesc(4u, 1u, 1u, ETextureFormat::R8, true, DefaultTextureSwizzleArray, { 16 });
    TextureResource res(EResourceType_TextureCube, texDesc, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    InSequence seq;
    EXPECT_CALL(renderer.deviceMock, allocateTextureCube(4u, ETextureFormat::R8, DefaultTextureSwizzleArray, 3u, 6 * (16 + 4 + 1))).WillOnce(Return(DeviceResourceHandle(123)));
    for (UInt32 i = 0u; i < 6u; ++i)
    {
        EXPECT_CALL(renderer.deviceMock, uploadTextureData(DeviceResourceHandle(123), 0u, 0u, 0u, i, 4u, 4u, 1u, _, _));
    }
    EXPECT_CALL(renderer.deviceMock, generateMipmaps(DeviceResourceHandle(123)));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));
    EXPECT_EQ(6u * (4 * 4 + 2 * 2 + 1), vramSize);
}

TEST_F(AResourceUploader, canStoreBinaryShader)
{
    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    const SceneId sceneUsingResource(14);

    BinaryShaderProviderFake binaryShaderProvider;
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash(), _)).WillOnce(Return(true));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(DeviceMock::FakeShaderDeviceHandle, _, _)).WillOnce(DoAll(SetArgReferee<1>(UInt8Vector(10)), Return(true)));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(_, sceneUsingResource, _, _, _));

    ResourceUploader uploaderWithBinaryProvider(true, &binaryShaderProvider);
    uploaderWithBinaryProvider.storeShaderInBinaryShaderCache(renderer, DeviceMock::FakeShaderDeviceHandle, res.getHash(), sceneUsingResource);
}

TEST_F(AResourceUploader, doesNoStoreBinaryShaderIfFailedToGetBinaryShaderFromDevice)
{
    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    const SceneId sceneUsingResource(14);

    BinaryShaderProviderFake binaryShaderProvider;

    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash(), _)).WillOnce(Return(true));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(DeviceMock::FakeShaderDeviceHandle, _, _)).WillOnce(Return(false));
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(_, sceneUsingResource, _, _, _)).Times(0);

    ResourceUploader uploaderWithBinaryProvider(true, &binaryShaderProvider);
    uploaderWithBinaryProvider.storeShaderInBinaryShaderCache(renderer, DeviceMock::FakeShaderDeviceHandle, res.getHash(), sceneUsingResource);
}

TEST_F(AResourceUploader, ifShaderShouldNotBeCachedNoDownloadWillHappen)
{
    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    const SceneId sceneUsingResource(14);

    BinaryShaderProviderFake binaryShaderProvider;
    EXPECT_CALL(binaryShaderProvider, shouldBinaryShaderBeCached(res.getHash(), _)).WillOnce(Return(false));
    EXPECT_CALL(renderer.deviceMock, getBinaryShader(DeviceMock::FakeShaderDeviceHandle, _, _)).Times(0);
    EXPECT_CALL(binaryShaderProvider, storeBinaryShader(_, sceneUsingResource, _, _, _)).Times(0);

    ResourceUploader uploaderWithBinaryProvider(true, &binaryShaderProvider);
    uploaderWithBinaryProvider.storeShaderInBinaryShaderCache(renderer, DeviceMock::FakeShaderDeviceHandle, res.getHash(), sceneUsingResource);
}

TEST_F(AResourceUploader, doesNotTryToStoreBinaryShaderIfNoBinaryShaderCacheAvailable)
{
    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    const SceneId sceneUsingResource(14);

    EXPECT_CALL(renderer.deviceMock, getBinaryShader(_, _, _)).Times(0);
    uploader.storeShaderInBinaryShaderCache(renderer, DeviceMock::FakeShaderDeviceHandle, res.getHash(), sceneUsingResource);
}

TEST_F(AResourceUploader, doesNotReturnDeviceHandleForEffectResourceWithoutBinaryShaderCache)
{
    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);
    EXPECT_FALSE(uploader.uploadResource(renderer, resourceObject, vramSize).has_value());
}

TEST_F(AResourceUploader, uploadsShaderWithoutBinaryShaderCacheIfAsyncUploadDisabled)
{
    ResourceUploader uploaderWithoutCacheOrAsyncUpload(false);

    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    EXPECT_CALL(renderer.deviceMock, registerShader(_));
    EXPECT_EQ(DeviceMock::FakeShaderDeviceHandle, uploaderWithoutCacheOrAsyncUpload.uploadResource(renderer, resourceObject, vramSize));
}

TEST_F(AResourceUploader, doesNotReturnDeviceHandleIfShaderIsNotCached)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithBinaryProvider(true, &binaryShaderProvider);

    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, binaryShaderFormatsReported());
    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash())).WillOnce(Return(false));

    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_FALSE(uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize).has_value());
}

TEST_F(AResourceUploader, uploadShaderIfShaderIsNotCachedAndAsyncUploadDisabled)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithCacheButNoAsyncUpload(false, &binaryShaderProvider);

    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, binaryShaderFormatsReported());
    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash())).WillOnce(Return(false));

    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(renderer.deviceMock, uploadShader(_));
    EXPECT_CALL(renderer.deviceMock, registerShader(_));
    EXPECT_EQ(DeviceMock::FakeShaderDeviceHandle, uploaderWithCacheButNoAsyncUpload.uploadResource(renderer, resourceObject, vramSize));
}

TEST_F(AResourceUploader, uploadsEffectResourceFromBinaryShaderCacheWhenCacheHit)
{
    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };

    uint8_t binaryShaderData[] = { 1u, 2u, 3u, 4u };

    BinaryShaderProviderFake binaryShaderProvider;
    binaryShaderProvider.m_effectHash = res.getHash();
    binaryShaderProvider.m_binaryShaderData = binaryShaderData;
    binaryShaderProvider.m_binaryShaderDataSize = sizeof(binaryShaderData) / sizeof(uint8_t);
    binaryShaderProvider.m_binaryShaderFormat = BinaryShaderFormatID{ 12u };

    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    EXPECT_CALL(binaryShaderProvider, binaryShaderFormatsReported());
    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderSize(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderFormat(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderData(res.getHash(), _, _));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, true)).Times(1);

    EXPECT_CALL(renderer.deviceMock, uploadBinaryShader(_, _, _, _)).WillOnce(Return(DeviceResourceHandle(123)));

    ResourceUploader uploaderWithBinaryProvider(true, &binaryShaderProvider);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_EQ(123u, uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize));
}

TEST_F(AResourceUploader, doesNotReturnDeviceHandleForEffectResourceWithBrokenBinaryShaderCache)
{
    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    SceneId sceneUsingResource(14);

    uint8_t binaryShaderData[] = { 1u, 2u, 3u, 4u };

    BinaryShaderProviderFake binaryShaderProvider;
    binaryShaderProvider.m_effectHash = res.getHash();
    binaryShaderProvider.m_binaryShaderData = binaryShaderData;
    binaryShaderProvider.m_binaryShaderDataSize = sizeof(binaryShaderData) / sizeof(uint8_t);
    binaryShaderProvider.m_binaryShaderFormat = BinaryShaderFormatID{ 12u };

    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(1);

    // Set up so it looks like there is a valid binary shader cache
    EXPECT_CALL(binaryShaderProvider, binaryShaderFormatsReported());
    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat }));
    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderSize(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderFormat(res.getHash()));
    EXPECT_CALL(binaryShaderProvider, getBinaryShaderData(res.getHash(), _, _));

    // Make the upload from cache fail and expect the callback telling that the cache was broken
    EXPECT_CALL(renderer.deviceMock, uploadBinaryShader(_, _, _, _)).WillOnce(Return(DeviceResourceHandle::Invalid()));
    EXPECT_CALL(binaryShaderProvider, binaryShaderUploaded(_, false)).Times(1);

    ResourceUploader uploaderWithBinaryProvider(true, &binaryShaderProvider);
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    resourceObject.sceneUsage = { sceneUsingResource };
    EXPECT_FALSE(uploaderWithBinaryProvider.uploadResource(renderer, resourceObject, vramSize).has_value());
}

TEST_F(AResourceUploader, providesSupportedBinaryShaderFormatsOnlyOnce)
{
    BinaryShaderProviderFake binaryShaderProvider;
    ResourceUploader uploaderWithBinaryProvider(true, &binaryShaderProvider);

    EffectResource res("", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(Ref(res))).Times(3);

    EXPECT_CALL(binaryShaderProvider, binaryShaderFormatsReported()).Times(3);
    EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat })).Times(1);

    EXPECT_CALL(binaryShaderProvider, hasBinaryShader(res.getHash())).Times(3);

    ResourceDescriptor resourceObject1;
    resourceObject1.resource = ManagedResource{ &res, dummyManagedResourceCallback };
    resourceObject1.sceneUsage = { SceneId{1u} };
    ResourceDescriptor resourceObject2 = resourceObject1;
    resourceObject2.resource = ManagedResource{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject3 = resourceObject1;
    resourceObject3.resource = ManagedResource{ &res, dummyManagedResourceCallback };

    EXPECT_FALSE(uploaderWithBinaryProvider.uploadResource(renderer, resourceObject1, vramSize).has_value());
    EXPECT_FALSE(uploaderWithBinaryProvider.uploadResource(renderer, resourceObject2, vramSize).has_value());
    EXPECT_FALSE(uploaderWithBinaryProvider.uploadResource(renderer, resourceObject3, vramSize).has_value());
}

TEST_F(AResourceUploader, providesSupportedBinaryShaderFormatsOnlyOnceFromMultipleThreadsBeforeFirstHasBinaryShader)
{
    BinaryShaderProviderFake binaryShaderProvider;

    ResourceUploader uploaderWithBinaryProvider1(true, &binaryShaderProvider);
    ResourceUploader uploaderWithBinaryProvider2(true, &binaryShaderProvider);
    ResourceUploader uploaderWithBinaryProvider3(true, &binaryShaderProvider);

    EffectResource res1("1", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EffectResource res2("2", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EffectResource res3("3", "", "", {}, EffectInputInformationVector(), EffectInputInformationVector(), "", ResourceCacheFlag_DoNotCache);
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(3);
    EXPECT_CALL(binaryShaderProvider, binaryShaderFormatsReported()).Times(3);

    {
        InSequence inseq; // it would be wrong if any hasBinaryShader from "the other two" threads get called before deviceSupportsBinaryShaderFormats
        EXPECT_CALL(binaryShaderProvider, deviceSupportsBinaryShaderFormats(std::vector<BinaryShaderFormatID>{ DeviceMock::FakeSupportedBinaryShaderFormat })).Times(1).WillOnce(
            [](auto) { std::this_thread::sleep_for(std::chrono::milliseconds(2)); });
        EXPECT_CALL(binaryShaderProvider, hasBinaryShader(_)).Times(3);
    }

    ResourceDescriptor resourceObject1;
    ResourceDeleterCallingCallback dummyManagedResourceCallback1(managedResourceDeleter);
    resourceObject1.resource = ManagedResource{ &res1, dummyManagedResourceCallback1 };
    resourceObject1.sceneUsage = { SceneId{1u} };
    ResourceDescriptor resourceObject2;
    ResourceDeleterCallingCallback dummyManagedResourceCallback2(managedResourceDeleter);
    resourceObject2.resource = ManagedResource{ &res2, dummyManagedResourceCallback2 };
    resourceObject2.sceneUsage = { SceneId{2u} };
    ResourceDescriptor resourceObject3;
    ResourceDeleterCallingCallback dummyManagedResourceCallback3(managedResourceDeleter);
    resourceObject3.resource = ManagedResource{ &res3, dummyManagedResourceCallback3 };
    resourceObject3.sceneUsage = { SceneId{3u} };

    ThreadBarrier startBarrier(3);
    std::thread t1([&]() {
            ThreadLocalLog::SetPrefix(2);
            startBarrier.wait();
            uint32_t ramSize = 0;
            EXPECT_FALSE(uploaderWithBinaryProvider1.uploadResource(renderer, resourceObject1, ramSize).has_value());
        });
    std::thread t2([&]() {
            ThreadLocalLog::SetPrefix(3);
            startBarrier.wait();
            uint32_t ramSize = 0;
            EXPECT_FALSE(uploaderWithBinaryProvider2.uploadResource(renderer, resourceObject2, ramSize).has_value());
        });
    std::thread t3([&]() {
            ThreadLocalLog::SetPrefix(4);
            startBarrier.wait();
            uint32_t ramSize = 0;
            EXPECT_FALSE(uploaderWithBinaryProvider3.uploadResource(renderer, resourceObject3, ramSize).has_value());
        });

    t1.join();
    t2.join();
    t3.join();
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

    const ArrayResource res(EResourceType_VertexArray, 1, EDataType::Float, nullptr, ResourceCacheFlag_DoNotCache, {});
    ManagedResource managedRes{ &res, dummyManagedResourceCallback };
    ResourceDescriptor resourceObject;
    resourceObject.resource = managedRes;
    EXPECT_CALL(managedResourceDeleter, managedResourceDeleted(_)).Times(1);

    EXPECT_CALL(renderer.deviceMock, allocateVertexBuffer(res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(renderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData().data(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(renderer, resourceObject, vramSize));

    EXPECT_CALL(additionalRenderer.deviceMock, allocateVertexBuffer(res.getDecompressedDataSize())).WillOnce(Return(DeviceResourceHandle(123)));
    EXPECT_CALL(additionalRenderer.deviceMock, uploadVertexBufferData(DeviceResourceHandle(123), res.getResourceData().data(), res.getDecompressedDataSize()));
    EXPECT_EQ(123u, uploader.uploadResource(additionalRenderer, resourceObject, vramSize));
}
