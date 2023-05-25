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
            , bufferSize(96u)
            , buffer(bufferSize, 0x71)
            , fval(3.0f)
            , uival(5u)
        {
        }

        SceneActionCollection collection;
        SceneActionCollection otherCollection;

        const std::string str;
        const std::string strEmpty;
        const std::string strLong;
        const Guid guid;

        const UInt32 bufferSize;
        const std::vector<Byte> buffer;
        const float staticBuffer[5] = { 1.f, 2.f, 3.f, 4.f, 5.f };

        const float fval;
        const UInt32 uival;
    };

    TEST_F(ASceneActionCollectionComplexTypes, WriteStringAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(str);

        const size_t expectedSize =
            sizeof(UInt8) + str.size();
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteEmptyStringAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(strEmpty);

        const size_t expectedSize = sizeof(UInt8);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteLongStringAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(strLong);

        const size_t expectedSize = sizeof(UInt8) + SceneActionCollection::MaxStringLength;
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteArrayAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(buffer.data(), bufferSize);

        const UInt32 expectedSize = sizeof(UInt32) + bufferSize;
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteEmptyArrayAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(buffer.data(), 0u);

        const UInt32 expectedSize = sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteGuidAndCheckBufferSize)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(guid);

        const UInt32 expectedSize = sizeof(Guid::value_type);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetString)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(str);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        std::string readString;
        reader.read(readString);
        EXPECT_TRUE(reader.isFullyRead());

        const size_t expectedSize = str.size() + sizeof(UInt8);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(str.size(), readString.size());
        EXPECT_EQ(str, readString);
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetEmptyString)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(strEmpty);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        std::string readString;
        reader.read(readString);
        EXPECT_TRUE(reader.isFullyRead());

        const size_t expectedSize = sizeof(UInt8);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(0u, readString.size());
        EXPECT_EQ(strEmpty, readString);
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetLongString)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(strLong);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        std::string readString;
        reader.read(readString);
        EXPECT_TRUE(reader.isFullyRead());

        const size_t expectedSize = SceneActionCollection::MaxStringLength + sizeof(UInt8);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(strLong.substr(0, SceneActionCollection::MaxStringLength), readString);
        EXPECT_EQ(SceneActionCollection::MaxStringLength, readString.size());
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetStaticArray)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(staticBuffer);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        float readBuffer[5] = { 0 };
        reader.read(readBuffer);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = sizeof(staticBuffer);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(absl::MakeSpan(staticBuffer), readBuffer);
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetArray)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(buffer.data(), bufferSize);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        const Byte* data = nullptr;
        UInt32 size;
        reader.readWithoutCopy(data, size);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = bufferSize + sizeof(UInt32);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(buffer, absl::MakeSpan(data, size));
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetEmptyArray)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
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
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(guid);

        SceneActionCollection::SceneActionReader reader(collection[0]);
        Guid readGuid;
        reader.read(readGuid);
        EXPECT_TRUE(reader.isFullyRead());

        const UInt32 expectedSize = sizeof(Guid::value_type);
        EXPECT_EQ(expectedSize, collection.collectionData().size());
        EXPECT_EQ(expectedSize, collection[0].size());

        EXPECT_EQ(guid, readGuid);
    }

    TEST_F(ASceneActionCollectionComplexTypes, WriteAndGetMixtureOfTypes)
    {
        collection.beginWriteSceneAction(ESceneActionId::TestAction);
        collection.write(fval);
        collection.write(str);
        collection.write(buffer.data(), bufferSize);
        collection.write(strLong);
        collection.write(guid);
        collection.write(strEmpty);
        collection.write(uival);

        ASSERT_EQ(1u, collection.numberOfActions());

        SceneActionCollection::SceneActionReader reader(collection[0]);
        float readFval;
        reader.read(readFval);
        EXPECT_EQ(fval, readFval);

        std::string readStr1;
        reader.read(readStr1);
        EXPECT_EQ(str, readStr1);

        const Byte* data = nullptr;
        UInt32 size;
        reader.readWithoutCopy(data, size);

        EXPECT_EQ(buffer, absl::MakeSpan(data, size));

        std::string readStr2;
        reader.read(readStr2);
        EXPECT_EQ(strLong.substr(0, SceneActionCollection::MaxStringLength), readStr2);

        Guid readGuid;
        reader.read(readGuid);
        EXPECT_EQ(guid, readGuid);

        std::string readStr3;
        reader.read(readStr3);
        EXPECT_EQ(strEmpty, readStr3);

        UInt32 readUival;
        reader.read(readUival);
        EXPECT_EQ(uival, readUival);

        EXPECT_TRUE(reader.isFullyRead());
    }
}
