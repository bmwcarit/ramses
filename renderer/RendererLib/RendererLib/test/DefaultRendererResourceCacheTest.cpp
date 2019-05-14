//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/DefaultRendererResourceCache.h"
#include "DefaultRendererResourceCacheImpl.h"
#include "gtest/gtest.h"
#include "RendererAPI/Types.h"
#include "Utils/File.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/Adler32Checksum.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"

using namespace ramses_internal;

class ADefaultRendererResourceCache : public testing::Test
{
public:
    ADefaultRendererResourceCache()
        : m_saveFilePath("resCacheTest.dat")
    {
    }

    virtual ~ADefaultRendererResourceCache()
    {
        File binaryShaderFile(m_saveFilePath);

        if (binaryShaderFile.exists())
        {
            binaryShaderFile.remove();
        }
    }

    void createTestFile()
    {
        ramses::resourceCacheFlag_t cacheFlag(12345u);
        ramses::sceneId_t           sceneId(0x2288FFFF44);

        uint8_t                      data_1[] = {17u, 37u, 12u, 23u, 123u, 21u};
        ramses::rendererResourceId_t resId_1(0xFFFFFFFF123, 0x321FFFFFFFF);

        uint8_t data_2[] = {18u, 32u, 13u, 22u, 13u, 221u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 123u, 110u};
        ramses::rendererResourceId_t resId_2(0x0, 0x1);

        uint8_t                      data_3[] = {22u};
        ramses::rendererResourceId_t resId_3(0x123456, 0x12345);

        ramses::DefaultRendererResourceCache cache(100);
        cache.storeResource(resId_1, data_1, sizeof(data_1), cacheFlag, sceneId);
        cache.storeResource(resId_2, data_2, sizeof(data_2), cacheFlag, sceneId);
        cache.storeResource(resId_3, data_3, sizeof(data_3), cacheFlag, sceneId);
        cache.saveToFile(m_saveFilePath.c_str());
    }

    void enlargeTestFile()
    {
        ramses_internal::File file(m_saveFilePath);
        file.open(EFileMode_WriteExistingBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);
        file.seek(fileSize, EFileSeekOrigin_BeginningOfFile);
        ramses_internal::Char data(33);
        file.write(&data, sizeof(data));
    }

    void truncateTestFile(Int32 size)
    {
        ramses_internal::File file(m_saveFilePath);
        file.open(EFileMode_ReadOnlyBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);
        std::vector<uint8_t> data(fileSize);

        UInt numReadBytes;
        file.read(reinterpret_cast<ramses_internal::Char*>(data.data()), fileSize, numReadBytes);

        file.close();
        file.open(EFileMode_WriteNewBinary);

        Int32 newSize = static_cast<Int32>(size < 0 ? fileSize + size : size);

        file.write(reinterpret_cast<ramses_internal::Char*>(data.data()), newSize);
    }

    void corruptTestFile(uint32_t offset)
    {
        ramses_internal::File file(m_saveFilePath);
        file.open(EFileMode_WriteExistingBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);

        file.seek(offset, EFileSeekOrigin_BeginningOfFile);
        ramses_internal::Char data = 0;
        UInt                  numBytesRead;
        file.read(&data, sizeof(data), numBytesRead);
        file.seek(offset, EFileSeekOrigin_BeginningOfFile);
        data++;
        file.write(&data, sizeof(data));
    }

    void corruptVersionInTestFile()
    {
        ramses_internal::File file(m_saveFilePath);
        file.open(EFileMode_WriteExistingBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);
        const Int transportVersionOffset = offsetof(ramses::DefaultRendererResourceCacheImpl::FileHeader, transportVersion);
        file.seek(transportVersionOffset, EFileSeekOrigin_BeginningOfFile);
        ramses_internal::Char data = 0;
        UInt                  numBytesRead;
        file.read(&data, sizeof(data), numBytesRead);
        file.seek(transportVersionOffset, EFileSeekOrigin_BeginningOfFile);
        data++;
        file.write(&data, sizeof(data));
    }

protected:

    static void CheckItemInCache(const ramses::DefaultRendererResourceCache& cache, ramses::rendererResourceId_t id, uint8_t* expectedData, uint32_t expectedDataSize)
    {
        uint32_t foundSize;
        EXPECT_TRUE(cache.hasResource(id, foundSize));
        EXPECT_EQ(foundSize, expectedDataSize);

        uint8_t* readBuffer = new uint8_t[expectedDataSize];
        EXPECT_TRUE(cache.getResourceData(id, readBuffer, expectedDataSize));

        for (uint32_t i = 0; i < expectedDataSize; i++)
        {
            EXPECT_EQ(readBuffer[i], expectedData[i]);
        }

        delete[] readBuffer;
    }

    const String m_saveFilePath;
};

TEST_F(ADefaultRendererResourceCache, canStoreAndGetResource)
{
    ramses::DefaultRendererResourceCache cache(100);

    const ramses::rendererResourceId_t resId(123, 123);
    uint8_t resData[50];

    // Add some non-zero data
    for (uint32_t i = 0; i < sizeof(resData); i++)
    {
        resData[i] = i % 9;
    }

    cache.storeResource(resId, resData, sizeof(resData), ramses::resourceCacheFlag_t(1), 7);

    uint32_t loadedSize;
    EXPECT_TRUE(cache.hasResource(resId, loadedSize));
    EXPECT_EQ(loadedSize, sizeof(resData));

    uint8_t readBuffer[200] = {0}; // On purpose this is larger than the result data
    EXPECT_TRUE(cache.getResourceData(resId, readBuffer, sizeof(readBuffer)));

    for (uint32_t i = 0; i < sizeof(resData); i++)
    {
        EXPECT_EQ(resData[i], readBuffer[i]);
    }
}

TEST_F(ADefaultRendererResourceCache, doesNotAttempToStoreItemsTooLargeForCache)
{
    ramses::DefaultRendererResourceCache cache(100);
    EXPECT_FALSE(cache.shouldResourceBeCached(ramses::rendererResourceId_t(123, 123), 200, ramses::resourceCacheFlag_t(1), 7));
}

TEST_F(ADefaultRendererResourceCache, canReturnFalseIfResourceNotInCache)
{
    ramses::DefaultRendererResourceCache cache(100);
    uint32_t size;
    EXPECT_FALSE(cache.hasResource(ramses::rendererResourceId_t(123, 123), size));
}

TEST_F(ADefaultRendererResourceCache, unloadsOldestAddedItemWhenOutOfSpace)
{
    uint8_t inputBuffer[1000];
    uint32_t size;
    ramses::DefaultRendererResourceCache cache(100);

    cache.storeResource(ramses::rendererResourceId_t(1, 0), inputBuffer, 40, ramses::resourceCacheFlag_t(1), 7);
    cache.storeResource(ramses::rendererResourceId_t(2, 0), inputBuffer, 40, ramses::resourceCacheFlag_t(1), 7);
    EXPECT_TRUE(cache.hasResource(ramses::rendererResourceId_t(1, 0), size));
    EXPECT_TRUE(cache.hasResource(ramses::rendererResourceId_t(2, 0), size));

    // Add an item which will exceed the maximum capacity of the cache
    cache.storeResource(ramses::rendererResourceId_t(3, 0), inputBuffer, 40, ramses::resourceCacheFlag_t(1), 7);
    EXPECT_FALSE(cache.hasResource(ramses::rendererResourceId_t(1, 0), size)); // Now gone
    EXPECT_TRUE(cache.hasResource(ramses::rendererResourceId_t(2, 0), size));
    EXPECT_TRUE(cache.hasResource(ramses::rendererResourceId_t(3, 0), size));

    // Same story again
    cache.storeResource(ramses::rendererResourceId_t(4, 0), inputBuffer, 40, ramses::resourceCacheFlag_t(1), 7);
    EXPECT_FALSE(cache.hasResource(ramses::rendererResourceId_t(2, 0), size)); // Now gone
    EXPECT_TRUE(cache.hasResource(ramses::rendererResourceId_t(3, 0), size));
    EXPECT_TRUE(cache.hasResource(ramses::rendererResourceId_t(4, 0), size));

    // Finally add one item which takes up all space
    cache.storeResource(ramses::rendererResourceId_t(5, 0), inputBuffer, 100, ramses::resourceCacheFlag_t(1), 7);
    EXPECT_FALSE(cache.hasResource(ramses::rendererResourceId_t(3, 0), size));
    EXPECT_FALSE(cache.hasResource(ramses::rendererResourceId_t(4, 0), size));
    EXPECT_TRUE(cache.hasResource(ramses::rendererResourceId_t(5, 0), size));
}

TEST_F(ADefaultRendererResourceCache, canSaveAndLoadFromFile)
{
    ramses::resourceCacheFlag_t cacheFlag(12345u);
    ramses::sceneId_t sceneId(0x2288FFFF44);

    uint8_t data_1[] = { 17u, 37u, 12u, 23u, 123u, 21u };
    ramses::rendererResourceId_t resId_1(0xFFFFFFFF123, 0x321FFFFFFFF);

    uint8_t data_2[] = { 18u, 32u, 13u, 22u, 13u, 221u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 123u, 110u };
    ramses::rendererResourceId_t resId_2(0x0, 0x1);

    uint8_t data_3[] = { 22u };
    ramses::rendererResourceId_t resId_3(0x123456, 0x12345);

    ramses::DefaultRendererResourceCache savingCache(100);
    savingCache.storeResource(resId_1, data_1, sizeof(data_1), cacheFlag, sceneId);
    savingCache.storeResource(resId_2, data_2, sizeof(data_2), cacheFlag, sceneId);
    savingCache.storeResource(resId_3, data_3, sizeof(data_3), cacheFlag, sceneId);
    savingCache.saveToFile(m_saveFilePath.c_str());

    ramses::DefaultRendererResourceCache loadedCache(100);
    EXPECT_TRUE(loadedCache.loadFromFile(m_saveFilePath.c_str()));
    CheckItemInCache(loadedCache, resId_1, data_1, sizeof(data_1));
    CheckItemInCache(loadedCache, resId_2, data_2, sizeof(data_2));
    CheckItemInCache(loadedCache, resId_3, data_3, sizeof(data_3));
}

TEST_F(ADefaultRendererResourceCache, reportsFailOnFileNotFound)
{
    ramses::DefaultRendererResourceCache cache(100);
    EXPECT_FALSE(cache.loadFromFile("doesNotExist.dat"));
}

TEST_F(ADefaultRendererResourceCache, reportsSuccessOnCorrectFile)
{
    ramses::DefaultRendererResourceCache cache(100);
    createTestFile();
    EXPECT_TRUE(cache.loadFromFile(m_saveFilePath.c_str()));
}

TEST_F(ADefaultRendererResourceCache, reportsFailOnFileToLarge)
{
    ramses::DefaultRendererResourceCache cache(100);
    createTestFile();
    enlargeTestFile();
    EXPECT_FALSE(cache.loadFromFile(m_saveFilePath.c_str()));
}

TEST_F(ADefaultRendererResourceCache, reportsFailOnFileToShort)
{
    ramses::DefaultRendererResourceCache cache(100);
    createTestFile();
    truncateTestFile(-1);
    EXPECT_FALSE(cache.loadFromFile(m_saveFilePath.c_str()));
}

TEST_F(ADefaultRendererResourceCache, reportsFailOnFileShorterThanHeader)
{
    ramses::DefaultRendererResourceCache cache(100);
    createTestFile();
    truncateTestFile(sizeof(ramses::DefaultRendererResourceCacheImpl::FileHeader) - 1);
    EXPECT_FALSE(cache.loadFromFile(m_saveFilePath.c_str()));
}

TEST_F(ADefaultRendererResourceCache, reportsFailForAnyByteCorruptionInFile)
{
    ramses::DefaultRendererResourceCache cache(100);

    createTestFile();

    ramses_internal::File file(m_saveFilePath);
    UInt                  fileSize(0);
    file.getSizeInBytes(fileSize);

    for (uint32_t offset = 0; offset < fileSize; offset++)
    {
        if (offset > 0)
        {
            createTestFile();
        }
        corruptTestFile(offset);
        EXPECT_FALSE(cache.loadFromFile(m_saveFilePath.c_str()));
    }
}

TEST_F(ADefaultRendererResourceCache, reportsFailOnInvalidFileVersion)
{
    ramses::DefaultRendererResourceCache cache(100);
    createTestFile();
    corruptVersionInTestFile();
    EXPECT_FALSE(cache.loadFromFile(m_saveFilePath.c_str()));
}

TEST_F(ADefaultRendererResourceCache, reportsFailWhenCacheTooSmall)
{
    ramses::DefaultRendererResourceCache cache(1);
    createTestFile();
    EXPECT_FALSE(cache.loadFromFile(m_saveFilePath.c_str()));
}
