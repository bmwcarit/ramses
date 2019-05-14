//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/BinaryShaderCache.h"
#include "BinaryShaderCacheImpl.h"
#include "gtest/gtest.h"
#include "RendererAPI/Types.h"
#include "Utils/File.h"
#include <sys/stat.h>

using namespace ramses_internal;

class ABinaryShaderCache : public testing::Test
{
public:
    ABinaryShaderCache()
        : m_binaryShaderFilePath("test.binaryshader")
    {

    }

    virtual ~ABinaryShaderCache()
    {
        File binaryShaderFile(m_binaryShaderFilePath);
        if (binaryShaderFile.exists())
        {
            binaryShaderFile.remove();
        }
    }

    void createTestFile()
    {
        ramses::BinaryShaderCache cache;

        UInt8                    shaderData1[]   = {12u, 34u, 56u, 78u};
        const UInt32             shaderDataSize1 = sizeof(shaderData1) / sizeof(UInt8);
        const UInt32             format1         = 123u;
        const ramses::effectId_t effectHash1     = {11u, 0};
        cache.storeBinaryShader(effectHash1, shaderData1, shaderDataSize1, format1);

        UInt8                    shaderData2[]   = {13u, 14u, 66u, 7u, 89u, 10u};
        const UInt32             shaderDataSize2 = sizeof(shaderData2) / sizeof(UInt8);
        const UInt32             format2         = 112u;
        const ramses::effectId_t effectHash2     = {12u, 0};
        cache.storeBinaryShader(effectHash2, shaderData2, shaderDataSize2, format2);

        cache.saveToFile(m_binaryShaderFilePath.c_str());
    }

    void enlargeTestFile()
    {
        ramses_internal::File file(m_binaryShaderFilePath.c_str());
        file.open(EFileMode_WriteExistingBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);
        file.seek(fileSize, EFileSeekOrigin_BeginningOfFile);
        ramses_internal::Char data(33);
        file.write(&data, sizeof(data));
    }

    void truncateTestFile(Int32 size)
    {
        ramses_internal::File            file(m_binaryShaderFilePath.c_str());
        file.open(EFileMode_ReadOnlyBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);
        std::vector<uint8_t> data(fileSize);

        UInt numReadBytes;
        file.read(reinterpret_cast<ramses_internal::Char*>(data.data()), fileSize, numReadBytes);

        file.close();
        file.open(EFileMode_WriteNewBinary);

        Int32 newSize = (size < 0) ?  static_cast<Int32>(fileSize) + size : size;

        file.write(reinterpret_cast<ramses_internal::Char*>(data.data()), newSize);
    }

    void corruptTestFile()
    {
        ramses_internal::File file(m_binaryShaderFilePath);
        file.open(EFileMode_WriteExistingBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);
        const Int offset = sizeof(ramses::BinaryShaderCacheImpl::FileHeader);
        file.seek(offset, EFileSeekOrigin_BeginningOfFile);
        ramses_internal::Char data = 0;
        UInt numBytesRead;
        file.read(&data, sizeof(data), numBytesRead);
        file.seek(offset, EFileSeekOrigin_BeginningOfFile);
        data++;
        file.write(&data, sizeof(data));
    }

    void corruptVersionInTestFile()
    {
        ramses_internal::File file(m_binaryShaderFilePath);
        file.open(EFileMode_WriteExistingBinary);
        UInt fileSize(0);
        file.getSizeInBytes(fileSize);
        const Int transportVersionOffset = offsetof(ramses::BinaryShaderCacheImpl::FileHeader, transportVersion);
        file.seek(transportVersionOffset, EFileSeekOrigin_BeginningOfFile);
        ramses_internal::Char data = 0;
        UInt                  numBytesRead;
        file.read(&data, sizeof(data), numBytesRead);
        file.seek(transportVersionOffset, EFileSeekOrigin_BeginningOfFile);
        data++;
        file.write(&data, sizeof(data));
    }

protected:
    ramses::BinaryShaderCache m_cache;
    const String m_binaryShaderFilePath;
};

TEST_F(ABinaryShaderCache, canStoreAndGetBinaryShader)
{
    UInt8 shaderData[] = { 12u, 34u, 56u, 78u };
    const UInt32 shaderDataSize = sizeof(shaderData) / sizeof(UInt8);
    const UInt32 format = 123u;
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, shaderData, shaderDataSize, format);

    EXPECT_TRUE(m_cache.hasBinaryShader(effectHash1));
    EXPECT_EQ(shaderDataSize, m_cache.getBinaryShaderSize(effectHash1));
    EXPECT_EQ(format, m_cache.getBinaryShaderFormat(effectHash1));

    UInt8Vector dataRead(shaderDataSize);
    m_cache.getBinaryShaderData(effectHash1, &dataRead.front(), shaderDataSize);

    // make sure the data value is the same.
    for (UInt32 index = 0; index < shaderDataSize; index++)
    {
        EXPECT_TRUE(shaderData[index] == dataRead[index]);
    }
}

TEST_F(ABinaryShaderCache, canReturnFalseWhenBinaryShaderNotInCache)
{
    UInt8 shaderData[] = { 12u, 34u, 56u, 78u };
    const UInt32 shaderDataSize = sizeof(shaderData) / sizeof(UInt8);
    const UInt32 format = 123u;
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, shaderData, shaderDataSize, format);

    ramses::effectId_t nonExistEffectHash = { 13u, 0 };
    EXPECT_FALSE(m_cache.hasBinaryShader(nonExistEffectHash));
    EXPECT_EQ(0u, m_cache.getBinaryShaderFormat(nonExistEffectHash));
    EXPECT_EQ(0u, m_cache.getBinaryShaderSize(nonExistEffectHash));
}

TEST_F(ABinaryShaderCache, canSaveToAndLoadFromBinaryShaderFile)
{
    UInt8 shaderData1[] = { 12u, 34u, 56u, 78u };
    const UInt32 shaderDataSize1 = sizeof(shaderData1) / sizeof(UInt8);
    const UInt32 format1 = 123u;
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, shaderData1, shaderDataSize1, format1);

    UInt8 shaderData2[] = { 13u, 14u, 66u, 7u, 89u, 10u };
    const UInt32 shaderDataSize2 = sizeof(shaderData2) / sizeof(UInt8);
    const UInt32 format2 = 112u;
    const ramses::effectId_t effectHash2 = { 12u, 0 };
    m_cache.storeBinaryShader(effectHash2, shaderData2, shaderDataSize2, format2);

    m_cache.saveToFile(m_binaryShaderFilePath.c_str());

    ramses::BinaryShaderCache newCache;
    EXPECT_TRUE(newCache.loadFromFile(m_binaryShaderFilePath.c_str()));

    // check shader data 1
    EXPECT_TRUE(newCache.hasBinaryShader(effectHash1));
    EXPECT_EQ(format1, newCache.getBinaryShaderFormat(effectHash1));

    const UInt32 shaderDataSize1Read = newCache.getBinaryShaderSize(effectHash1);
    EXPECT_EQ(shaderDataSize1, shaderDataSize1Read);

    UInt8Vector shaderData1Read(shaderDataSize1Read);
    newCache.getBinaryShaderData(effectHash1, &shaderData1Read.front(), shaderDataSize1Read);
    for (UInt32 index = 0; index < shaderDataSize1Read; index++)
    {
        EXPECT_EQ(shaderData1[index], shaderData1Read[index]);
    }

    // check shader data 2
    EXPECT_TRUE(newCache.hasBinaryShader(effectHash2));
    EXPECT_EQ(format2, newCache.getBinaryShaderFormat(effectHash2));

    const UInt32 shaderDataSize2Read = newCache.getBinaryShaderSize(effectHash2);
    EXPECT_EQ(shaderDataSize2, shaderDataSize2Read);

    UInt8Vector shaderData2Read(shaderDataSize2Read);
    newCache.getBinaryShaderData(effectHash2, &shaderData2Read.front(), shaderDataSize2Read);
    for (UInt32 index = 0; index < shaderDataSize2Read; index++)
    {
        EXPECT_EQ(shaderData2[index], shaderData2Read[index]);
    }
}

TEST_F(ABinaryShaderCache, canReturnFalseWhenTryToLoadFromNonExistFile)
{
    EXPECT_FALSE(m_cache.loadFromFile("NonExistFile.binaryshader"));
}

TEST_F(ABinaryShaderCache, reportsSuccessOnCorrectFile)
{
    createTestFile();
    EXPECT_TRUE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
}

TEST_F(ABinaryShaderCache, reportsFailOnFileToLarge)
{
    createTestFile();
    enlargeTestFile();
    EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
}

TEST_F(ABinaryShaderCache, reportsFailOnFileToShort)
{
    createTestFile();
    truncateTestFile(-1);
    EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
}

TEST_F(ABinaryShaderCache, reportsFailOnFileShorterThanHeader)
{
    createTestFile();
    truncateTestFile(sizeof(ramses::BinaryShaderCacheImpl::FileHeader) - 1);
    EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
}

TEST_F(ABinaryShaderCache, reportsFailOnNotMatchingChecksum)
{
    createTestFile();
    corruptTestFile();
    EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
}

TEST_F(ABinaryShaderCache, reportsFailOnInvalidFileVersion)
{
    createTestFile();
    corruptVersionInTestFile();
    EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
}
