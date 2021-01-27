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
#include <array>

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
        const ramses::binaryShaderFormatId_t format1{ 123u };
        const ramses::effectId_t effectHash1     = {11u, 0};
        cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData1, shaderDataSize1, format1);

        UInt8                    shaderData2[]   = {13u, 14u, 66u, 7u, 89u, 10u};
        const UInt32             shaderDataSize2 = sizeof(shaderData2) / sizeof(UInt8);
        const ramses::binaryShaderFormatId_t format2{ 112u };
        const ramses::effectId_t effectHash2     = {12u, 0};
        cache.storeBinaryShader(effectHash2, ramses::sceneId_t(2u), shaderData2, shaderDataSize2, format2);

        cache.saveToFile(m_binaryShaderFilePath.c_str());
    }

    void enlargeTestFile()
    {
        ramses_internal::File file(m_binaryShaderFilePath.c_str());
        UInt fileSize(0);
        EXPECT_TRUE(file.getSizeInBytes(fileSize));
        EXPECT_TRUE(file.open(File::Mode::WriteExistingBinary));
        EXPECT_TRUE(file.seek(fileSize, File::SeekOrigin::BeginningOfFile));
        ramses_internal::Char data(33);
        EXPECT_TRUE(file.write(&data, sizeof(data)));
    }

    void truncateTestFile(Int32 size)
    {
        ramses_internal::File file(m_binaryShaderFilePath.c_str());
        EXPECT_TRUE(file.open(File::Mode::ReadOnlyBinary));
        UInt fileSize(0);
        EXPECT_TRUE(file.getSizeInBytes(fileSize));
        std::vector<uint8_t> data(fileSize);

        UInt numReadBytes;
        EXPECT_EQ(EStatus::Ok, file.read(data.data(), fileSize, numReadBytes));

        file.close();
        EXPECT_TRUE(file.open(File::Mode::WriteNewBinary));

        Int32 newSize = (size < 0) ?  static_cast<Int32>(fileSize) + size : size;

        EXPECT_TRUE(file.write(data.data(), newSize));
    }

    void corruptTestFile()
    {
        ramses_internal::File file(m_binaryShaderFilePath);
        UInt fileSize(0);
        EXPECT_TRUE(file.getSizeInBytes(fileSize));
        EXPECT_TRUE(file.open(File::Mode::WriteExistingBinary));
        const Int offset = sizeof(ramses::BinaryShaderCacheImpl::FileHeader);
        EXPECT_TRUE(file.seek(offset, File::SeekOrigin::BeginningOfFile));
        ramses_internal::Char data = 0;
        UInt numBytesRead;
        EXPECT_EQ(EStatus::Ok, file.read(&data, sizeof(data), numBytesRead));
        EXPECT_TRUE(file.seek(offset, File::SeekOrigin::BeginningOfFile));
        data++;
        EXPECT_TRUE(file.write(&data, sizeof(data)));
    }

    void corruptVersionInTestFile()
    {
        ramses_internal::File file(m_binaryShaderFilePath);
        UInt fileSize(0);
        EXPECT_TRUE(file.getSizeInBytes(fileSize));
        EXPECT_TRUE(file.open(File::Mode::WriteExistingBinary));
        const Int transportVersionOffset = offsetof(ramses::BinaryShaderCacheImpl::FileHeader, transportVersion);
        EXPECT_TRUE(file.seek(transportVersionOffset, File::SeekOrigin::BeginningOfFile));
        ramses_internal::Char data = 0;
        UInt                  numBytesRead;
        EXPECT_EQ(EStatus::Ok, file.read(&data, sizeof(data), numBytesRead));
        EXPECT_TRUE(file.seek(transportVersionOffset, File::SeekOrigin::BeginningOfFile));
        data++;
        EXPECT_TRUE(file.write(&data, sizeof(data)));
    }

protected:
    ramses::BinaryShaderCache m_cache;
    const String m_binaryShaderFilePath;
};

TEST_F(ABinaryShaderCache, canStoreAndGetBinaryShader)
{
    UInt8 shaderData[] = { 12u, 34u, 56u, 78u };
    const UInt32 shaderDataSize = sizeof(shaderData) / sizeof(UInt8);
    const ramses::binaryShaderFormatId_t format{ 123u };
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData, shaderDataSize, format);

    m_cache.deviceSupportsBinaryShaderFormats(&format, 1u);
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

TEST_F(ABinaryShaderCache, reportsNoShaderAvailableIfShaderCachedButWithWrongFormat)
{
    std::array<UInt8, 4> shaderData{ 12u, 34u, 56u, 78u };
    const ramses::binaryShaderFormatId_t format{ 123u };
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData.data(), uint32_t(shaderData.size()), format);

    const ramses::binaryShaderFormatId_t supportedFormat{ 124u };
    m_cache.deviceSupportsBinaryShaderFormats(&supportedFormat, 1u);
    EXPECT_FALSE(m_cache.hasBinaryShader(effectHash1));
}

TEST_F(ABinaryShaderCache, reportsNoShaderAvailableIfShaderNotCachedAndDeviceReportsFormatZeroSupported) // zero format is used as invalid format in cache impl
{
    const ramses::binaryShaderFormatId_t supportedFormat{ 0u };
    m_cache.deviceSupportsBinaryShaderFormats(&supportedFormat, 1u);
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    EXPECT_FALSE(m_cache.hasBinaryShader(effectHash1));
}

TEST_F(ABinaryShaderCache, canReturnFalseWhenBinaryShaderNotInCache)
{
    UInt8 shaderData[] = { 12u, 34u, 56u, 78u };
    const UInt32 shaderDataSize = sizeof(shaderData) / sizeof(UInt8);
    const ramses::binaryShaderFormatId_t format{ 123u };
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData, shaderDataSize, format);

    ramses::effectId_t nonExistEffectHash = { 13u, 0 };
    EXPECT_FALSE(m_cache.hasBinaryShader(nonExistEffectHash));
    EXPECT_EQ(ramses::binaryShaderFormatId_t{ 0u }, m_cache.getBinaryShaderFormat(nonExistEffectHash));
    EXPECT_EQ(0u, m_cache.getBinaryShaderSize(nonExistEffectHash));
}

TEST_F(ABinaryShaderCache, canSaveToAndLoadFromBinaryShaderFile)
{
    UInt8 shaderData1[] = { 12u, 34u, 56u, 78u };
    const UInt32 shaderDataSize1 = sizeof(shaderData1) / sizeof(UInt8);
    const ramses::binaryShaderFormatId_t format1{ 123u };
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData1, shaderDataSize1, format1);

    UInt8 shaderData2[] = { 13u, 14u, 66u, 7u, 89u, 10u };
    const UInt32 shaderDataSize2 = sizeof(shaderData2) / sizeof(UInt8);
    const ramses::binaryShaderFormatId_t format2{ 112u };
    const ramses::effectId_t effectHash2 = { 12u, 0 };
    m_cache.storeBinaryShader(effectHash2, ramses::sceneId_t(2u), shaderData2, shaderDataSize2, format2);

    m_cache.saveToFile(m_binaryShaderFilePath.c_str());

    ramses::BinaryShaderCache newCache;
    EXPECT_TRUE(newCache.loadFromFile(m_binaryShaderFilePath.c_str()));

    // need to init with supported formats in order to report cached shaders
    std::array<ramses::binaryShaderFormatId_t, 2> supportedFormats = { format1, format2 };
    newCache.deviceSupportsBinaryShaderFormats(supportedFormats.data(), uint32_t(supportedFormats.size()));

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

TEST_F(ABinaryShaderCache, handlesDoubleStoreProperly)
{
    UInt8 shaderData[] = { 12u, 34u, 56u, 78u };
    const UInt32 shaderDataSize = sizeof(shaderData) / sizeof(UInt8);
    const ramses::binaryShaderFormatId_t format{ 123u };
    const ramses::effectId_t effectHash1 = { 11u, 0 };
    m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData, shaderDataSize, format);
    m_cache.deviceSupportsBinaryShaderFormats(&format, 1u);
    EXPECT_TRUE(m_cache.hasBinaryShader(effectHash1));
    m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData, shaderDataSize, format);
    EXPECT_TRUE(m_cache.hasBinaryShader(effectHash1));
}
