//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Scene/SceneActionCollection.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/BinaryOutputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    class ASceneActionCollectionComplexTypes : public ::testing::Test
    {
    public:
        ASceneActionCollectionComplexTypes()
            : str(
                "People assume that time is a strict progression of cause \
                to effect, but *actually* from a non-linear, non-subjective \
                viewpoint - it's more like a big ball of wibbly wobbly... \
                time-y wimey... stuff.")
            , strEmpty("")
            , strLong(
                "Lorem ipsum dolor sit amet, consectetur adipiscing \
                elit. Aliquam augue tortor, vehicula quis massa eget, viverra \
                eleifend urna. Nunc at odio leo. Ut dignissim ipsum non \
                efficitur placerat. Aliquam condimentum fringilla velit quis \
                aliquam. Nulla sed magna at justo cursus consectetur in a \
                purus. Nulla sed sem magna. Aliquam ut quam malesuada nunc \
                ultricies lacinia. Sed commodo ut lectus nec varius. \
                Suspendisse viverra, orci vel commodo aliquam, sem felis \
                egestas tortor, non sollicitudin lorem ipsum ut nullam.")
            , guid(Guid("9d2aeb99-6eea-4acb-8d93-df619186cff9"))
            , guidData(guid.getGuidData())
            , bufferSize(96u)
            , buffer(bufferSize, 0x71)
            , fval(3.0f)
            , uival(5u)
        {
        }

        SceneActionCollection collection;
        SceneActionCollection otherCollection;

        const String str;
        const String strEmpty;
        const String strLong;
        const Guid guid;
        const generic_uuid_t guidData;

        const UInt32 bufferSize;
        const std::vector<Byte> buffer;
        const float staticBuffer[5] = { 1.f, 2.f, 3.f, 4.f, 5.f };

        const Float fval;
        const UInt32 uival;
    };

    TEST_F(ASceneActionCollectionComplexTypes, WriteStringAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(str);

        const UInt32 expectedSize =
            sizeof(UInt32) + static_cast<UInt32>(str.getLength());
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteEmptyStringAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(strEmpty);

        const UInt32 expectedSize = sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteLongStringAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(strLong);

        const UInt32 expectedSize = sizeof(UInt32) + SceneActionCollection::MaxStringLength;
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteArrayAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(buffer.data(), bufferSize);

        const UInt32 expectedSize = sizeof(UInt32) + bufferSize;
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteEmptyArrayAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(buffer.data(), 0u);

        const UInt32 expectedSize = sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteGuidAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(guid);

        const UInt32 expectedSize = sizeof(generic_uuid_t);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteGuidDataAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(guidData);

        const UInt32 expectedSize = sizeof(generic_uuid_t);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetString)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(str);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        String readString;
        reader.read(readString);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = static_cast<UInt32>(str.getLength()) + sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(str.getLength(), readString.getLength());
        EXPECT_EQ(str, readString);
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetEmptyString)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(strEmpty);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        String readString;
        reader.read(readString);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(0u, readString.getLength());
        EXPECT_EQ(strEmpty, readString);
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetLongString)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(strLong);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        String readString;
        reader.read(readString);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = SceneActionCollection::MaxStringLength + sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(strLong.substr(0, SceneActionCollection::MaxStringLength), readString);
        EXPECT_EQ(SceneActionCollection::MaxStringLength, readString.getLength());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetStaticArray)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(staticBuffer);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        float readBuffer[5] = { 0 };
        reader.read(readBuffer);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = sizeof(staticBuffer);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(0, PlatformMemory::Compare(staticBuffer, readBuffer, sizeof(staticBuffer)));
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetArray)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(buffer.data(), bufferSize);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        const Byte* data = nullptr;
        UInt32 size;
        reader.readWithoutCopy(data, size);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = bufferSize + sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        ASSERT_EQ(bufferSize, size);
        EXPECT_EQ(0, PlatformMemory::Compare(buffer.data(), data, size));
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetEmptyArray)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(buffer.data(), 0u);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        const Byte* data = nullptr;
        UInt32 size;
        reader.readWithoutCopy(data, size);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(size, 0u);
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetGuid)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(guid);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        generic_uuid_t readGuid;
        reader.read(readGuid);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = sizeof(generic_uuid_t);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(0, PlatformMemory::Compare(&readGuid, &guid, sizeof(generic_uuid_t)));
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetMixtureOfTypes)
    {
        collection.beginWriteSceneAction(ESceneActionId_TestAction);
        collection.write(fval);
        collection.write(str);
        collection.write(buffer.data(), bufferSize);
        collection.write(strLong);
        collection.write(guid);
        collection.write(strEmpty);
        collection.write(uival);

        ASSERT_EQ(1u, collection.numberOfActions());

        SceneActionCollection::SceneActionReader reader(collection[0]);
        Float readFval;
        reader.read(readFval);
        EXPECT_EQ(fval, readFval);

        String readStr1;
        reader.read(readStr1);
        EXPECT_EQ(str, readStr1);

        const Byte* data = nullptr;
        UInt32 size;
        reader.readWithoutCopy(data, size);

        ASSERT_EQ(bufferSize, size);
        EXPECT_EQ(0, PlatformMemory::Compare(buffer.data(), data, size));

        String readStr2;
        reader.read(readStr2);
        EXPECT_EQ(strLong.substr(0, SceneActionCollection::MaxStringLength), readStr2);

        generic_uuid_t readGuid;
        reader.read(readGuid);
        EXPECT_EQ(0, PlatformMemory::Compare(&readGuid, &guid, sizeof(generic_uuid_t)));

        String readStr3;
        reader.read(readStr3);
        EXPECT_EQ(strEmpty, readStr3);

        UInt32 readUival;
        reader.read(readUival);
        EXPECT_EQ(uival, readUival);

        EXPECT_TRUE(reader.isFullyRead());
    }
}
