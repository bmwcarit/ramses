//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneActionCollection.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    class ASceneActionCollection : public ::testing::Test
    {
    };

    TEST_F(ASceneActionCollection, canBeDefaultConstructed)
    {
        SceneActionCollection c;
        EXPECT_TRUE(c.empty());
        EXPECT_EQ(0u, c.numberOfActions());
        EXPECT_EQ(0u, c.collectionData().size());
    }

    TEST_F(ASceneActionCollection, canBeConstrcutedWithCapacity)
    {
        SceneActionCollection c(1, 1);
        EXPECT_TRUE(c.empty());
        EXPECT_EQ(0u, c.numberOfActions());
        EXPECT_EQ(0u, c.collectionData().size());
    }

    TEST_F(ASceneActionCollection, beginWriteAddsSceneAction)
    {
        SceneActionCollection c;
        EXPECT_EQ(0u, c.numberOfActions());
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        EXPECT_EQ(1u, c.numberOfActions());
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        EXPECT_EQ(2u, c.numberOfActions());
    }

    TEST_F(ASceneActionCollection, beginWriteDoesNotAddData)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        EXPECT_EQ(0u, c.collectionData().size());
    }

    TEST_F(ASceneActionCollection, writeAddsDataToBlob)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        EXPECT_NE(0u, c.collectionData().size());
    }

    TEST_F(ASceneActionCollection, clearRemovesActionsAndData)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.clear();
        EXPECT_EQ(0u, c.numberOfActions());
        EXPECT_EQ(0u, c.collectionData().size());
    }

    TEST_F(ASceneActionCollection, isEqualWhenEmpty)
    {
        SceneActionCollection c;
        SceneActionCollection d;
        EXPECT_TRUE(c == d);
        EXPECT_FALSE(c != d);
    }

    TEST_F(ASceneActionCollection, isEqualWithSameTypeAndData)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_TestAction);
        d.write(123u);
        d.beginWriteSceneAction(ESceneActionId_AllocateNode);

        EXPECT_TRUE(c == d);
        EXPECT_FALSE(c != d);
    }

    TEST_F(ASceneActionCollection, isNotEqualWhenDataDiffers)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_TestAction);
        d.write(234u);

        EXPECT_FALSE(c == d);
        EXPECT_TRUE(c != d);
    }

    TEST_F(ASceneActionCollection, isNotEqualWhenTypesDiffer)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_AllocateNode);
        d.write(123u);

        EXPECT_FALSE(c == d);
        EXPECT_TRUE(c != d);
    }

    TEST_F(ASceneActionCollection, isNotEqualWhenOrderOfActionsDifferent)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_AllocateNode);
        d.beginWriteSceneAction(ESceneActionId_TestAction);

        EXPECT_FALSE(c == d);
        EXPECT_TRUE(c != d);
    }

    TEST_F(ASceneActionCollection, isNotEqualWhenOrderOfDataDifferent)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.write(456u);

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_TestAction);
        d.write(456u);
        d.write(123u);

        EXPECT_FALSE(c == d);
        EXPECT_TRUE(c != d);
    }

    TEST_F(ASceneActionCollection, canAccessSingleActionViaFrontAndback)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        EXPECT_EQ(ESceneActionId_TestAction, c.front().type());
        EXPECT_EQ(ESceneActionId_TestAction, c.back().type());
    }

    TEST_F(ASceneActionCollection, canAccessDifferentActionsViaFrontAndback)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        EXPECT_EQ(ESceneActionId_TestAction, c.front().type());
        EXPECT_EQ(ESceneActionId_AllocateNode, c.back().type());
    }

    TEST_F(ASceneActionCollection, canAccessActionsViaArrayAccessOperator)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        ASSERT_EQ(2u, c.numberOfActions());
        EXPECT_EQ(ESceneActionId_TestAction, c[0].type());
        EXPECT_EQ(ESceneActionId_AllocateNode, c[1].type());
    }

    TEST_F(ASceneActionCollection, memberSwapSwapsActionInfoAndData)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        std::vector<Byte> oldDataC(c.collectionData());

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_AllocateNode);
        d.write(456u);
        d.beginWriteSceneAction(ESceneActionId_ReleaseRenderable);
        std::vector<Byte> oldDataD(d.collectionData());

        c.swap(d);

        ASSERT_EQ(2u, c.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateNode, c[0].type());
        EXPECT_EQ(ESceneActionId_ReleaseRenderable, c[1].type());
        EXPECT_EQ(oldDataD, c.collectionData());

        ASSERT_EQ(1u, d.numberOfActions());
        EXPECT_EQ(ESceneActionId_TestAction, d[0].type());
        EXPECT_EQ(oldDataC, d.collectionData());
    }

    TEST_F(ASceneActionCollection, functionSwapSwapsActionInfoAndData)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        std::vector<Byte> oldDataC(c.collectionData());

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_AllocateNode);
        d.write(456u);
        d.beginWriteSceneAction(ESceneActionId_ReleaseRenderable);
        std::vector<Byte> oldDataD(d.collectionData());

        using ramses_capu::swap;
        swap(c, d);

        ASSERT_EQ(2u, c.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateNode, c[0].type());
        EXPECT_EQ(ESceneActionId_ReleaseRenderable, c[1].type());
        EXPECT_EQ(oldDataD, c.collectionData());

        ASSERT_EQ(1u, d.numberOfActions());
        EXPECT_EQ(ESceneActionId_TestAction, d[0].type());
        EXPECT_EQ(oldDataC, d.collectionData());
    }

    TEST_F(ASceneActionCollection, canBeExplicitlyCopied)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);

        SceneActionCollection d(c.copy());
        EXPECT_EQ(c, d);
    }

    TEST_F(ASceneActionCollection, canBeMoveConstructed)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);

        SceneActionCollection oldC(c.copy());

        SceneActionCollection d(std::move(c));
        EXPECT_EQ(oldC, d);
    }

    TEST_F(ASceneActionCollection, canBeMoveAssigned)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);

        SceneActionCollection oldC(c.copy());

        SceneActionCollection d;
        d = std::move(c);
        EXPECT_EQ(oldC, d);
    }

    TEST_F(ASceneActionCollection, canAppendToAnotherCollection)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.write(124u);

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_AllocateNode);
        d.write(456u);

        std::vector<Byte> oldDataC(c.collectionData());
        c.append(d);

        EXPECT_EQ(oldDataC.size() + d.collectionData().size(), c.collectionData().size());

        ASSERT_EQ(2u, c.numberOfActions());
        EXPECT_EQ(ESceneActionId_TestAction, c[0].type());
        EXPECT_EQ(0u, c[0].offsetInCollection());
        ASSERT_EQ(oldDataC.size(), c[0].size());
        EXPECT_EQ(0, PlatformMemory::Compare(c[0].data(), oldDataC.data(), oldDataC.size()));

        EXPECT_EQ(ESceneActionId_AllocateNode, c[1].type());
        EXPECT_EQ(oldDataC.size(), c[1].offsetInCollection());
        ASSERT_EQ(d[0].size(), c[1].size());
        EXPECT_EQ(0, PlatformMemory::Compare(c[1].data(), d[0].data(), d[0].size()));
    }

    TEST_F(ASceneActionCollection, canAppendEmptyToIncompleteCollection)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.write(124u);
        c.beginWriteSceneAction(ESceneActionId_Incomplete);
        c.write(123u);

        SceneActionCollection copyC(c.copy());

        SceneActionCollection d;
        c.append(d);
        EXPECT_EQ(c, copyC);
    }

    TEST_F(ASceneActionCollection, canAppendCompleteToIncompleteCollection)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_Incomplete);
        c.write(123u);

        const SceneActionCollection::SceneActionReader incompleteReader(c.back());
        const UInt32 offsetIncomplete = incompleteReader.offsetInCollection();
        const UInt32 sizeIncomplete = incompleteReader.size();
        std::vector<Byte> data(c.collectionData());

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_AllocateNode);
        d.write(456u);
        data.insert(data.end(), d.collectionData().begin(), d.collectionData().end());

        c.append(d);

        ASSERT_EQ(2u, c.numberOfActions());
        EXPECT_EQ(offsetIncomplete, c.back().offsetInCollection());
        EXPECT_EQ(ESceneActionId_AllocateNode, c.back().type());
        EXPECT_EQ(sizeIncomplete + d.front().size(), c.back().size());
        EXPECT_EQ(data, c.collectionData());
    }

    TEST_F(ASceneActionCollection, canAppendCompleteWithMultipleActionsToIncompleteCollection)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_Incomplete);
        c.write(123u);

        const UInt32 sizeIncomplete = c.front().size();
        std::vector<Byte> data(c.collectionData());

        SceneActionCollection dTemp;
        dTemp.beginWriteSceneAction(ESceneActionId_TestAction);
        dTemp.write(456u);
        dTemp.beginWriteSceneAction(ESceneActionId_AllocateNode);
        dTemp.write(456u);

        SceneActionCollection d;
        d.appendRawData(d.collectionData().data(), d.collectionData().size());
        d.addRawSceneActionInformation(ESceneActionId_TestAction, 0);
        d.addRawSceneActionInformation(ESceneActionId_AllocateNode, dTemp[0].size() + c[0].size());

        data.insert(data.end(), d.collectionData().begin(), d.collectionData().end());

        c.append(d);

        ASSERT_EQ(2u, c.numberOfActions());
        EXPECT_EQ(ESceneActionId_TestAction, c[0].type());
        EXPECT_EQ(ESceneActionId_AllocateNode, c[1].type());
        EXPECT_EQ(0u, c[0].offsetInCollection());
        EXPECT_EQ(dTemp.front().size() + sizeIncomplete, c[1].offsetInCollection());
        EXPECT_EQ(c[1].offsetInCollection(), c[0].size());
        EXPECT_EQ(data, c.collectionData());
    }

    TEST_F(ASceneActionCollection, canAppendIncompleteActionToIncompleteCollection)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_Incomplete);
        c.write(123u);

        const UInt32 sizeIncomplete = c.front().size();
        std::vector<Byte> data(c.collectionData());

        SceneActionCollection d;
        d.beginWriteSceneAction(ESceneActionId_Incomplete);
        d.write(456u);
        data.insert(data.end(), d.collectionData().begin(), d.collectionData().end());

        c.append(d);

        ASSERT_EQ(1u, c.numberOfActions());
        EXPECT_EQ(ESceneActionId_Incomplete, c[0].type());
        EXPECT_EQ(0u, c[0].offsetInCollection());
        EXPECT_EQ(sizeIncomplete + d[0].size(), c[0].size());
        EXPECT_EQ(data, c.collectionData());
    }

    TEST_F(ASceneActionCollection, isFullyReadIsTrueForEmptyAction)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        EXPECT_TRUE(c[0].isFullyRead());
    }

    TEST_F(ASceneActionCollection, isFullyReadBecomesTrueWhenAllDataRead)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.write(456u);

        unsigned int value;
        SceneActionCollection::SceneActionReader reader(c[0]);
        EXPECT_FALSE(reader.isFullyRead());
        reader.read(value);
        EXPECT_FALSE(reader.isFullyRead());
        reader.read(value);
        EXPECT_TRUE(reader.isFullyRead());
    }

    TEST_F(ASceneActionCollection, isFullyReadAlsoWorksForSecondAction)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        c.write(456u);

        unsigned int value;
        SceneActionCollection::SceneActionReader reader(c[1]);
        EXPECT_FALSE(reader.isFullyRead());
        reader.read(value);
        EXPECT_TRUE(reader.isFullyRead());
    }

    TEST_F(ASceneActionCollection, isFullyReadIsTrueForSecondEmptyAction)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        EXPECT_TRUE(c[1].isFullyRead());
    }

    TEST_F(ASceneActionCollection, beginAndEndIteratorAreEqualForEmptyCollection)
    {
        SceneActionCollection c;
        EXPECT_TRUE(c.begin() == c.end());
        EXPECT_FALSE(c.begin() != c.end());
    }

    TEST_F(ASceneActionCollection, beginAndEndIteratorAreNotEqualForNonEmptyCollection)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        EXPECT_FALSE(c.begin() == c.end());
        EXPECT_TRUE(c.begin() != c.end());
    }

    TEST_F(ASceneActionCollection, iteratorCanBeDereferenced)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        SceneActionCollection::Iterator it = c.begin();
        EXPECT_EQ(ESceneActionId_TestAction, (*it).type());
        EXPECT_EQ(ESceneActionId_TestAction, (*const_cast<const SceneActionCollection::Iterator&>(it)).type());
    }

    TEST_F(ASceneActionCollection, iteratorCanCallReaderViaArrowOperator)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        SceneActionCollection::Iterator it = c.begin();
        EXPECT_EQ(ESceneActionId_TestAction, it->type());
        EXPECT_EQ(ESceneActionId_TestAction, const_cast<const SceneActionCollection::Iterator&>(it)->type());
    }

    TEST_F(ASceneActionCollection, iteratorAdditionResultsInNewOperatorAtChangedPosition)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        c.write(456u);

        EXPECT_EQ(c.end(), c.begin() + 2);
        EXPECT_EQ(c.begin(), c.end() + (-2));

        EXPECT_EQ(ESceneActionId_TestAction, (c.begin() + 0)->type());
        EXPECT_EQ(ESceneActionId_AllocateNode, (c.begin() + 1)->type());
        EXPECT_EQ(ESceneActionId_AllocateNode, (c.end() + (-1))->type());
    }

    TEST_F(ASceneActionCollection, iteratorCanBeUsedInRangeBasedForLoop)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        c.write(456u);

        int idx = 0;
        for (auto r : c)
        {
            EXPECT_EQ(c[idx].type(), r.type());
            ++idx;
        }
    }

    TEST_F(ASceneActionCollection, canBeInitializedWithAppendingRawData)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        c.write(456u);
        c.write(789u);

        SceneActionCollection d;
        d.appendRawData(c.collectionData().data(), c.collectionData().size());
        EXPECT_EQ(c.collectionData(), d.collectionData());

        for (auto r : c)
        {
            d.addRawSceneActionInformation(r.type(), r.offsetInCollection());
        }
        EXPECT_EQ(c, d);
    }

    TEST_F(ASceneActionCollection, canBeInitializedWithDirectlyWritingRawData)
    {
        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        c.write(456u);
        c.write(789u);

        SceneActionCollection d;
        std::vector<Byte>& rawDataD = d.getRawDataForDirectWriting();
        rawDataD.insert(rawDataD.end(), c.collectionData().begin(), c.collectionData().end());
        EXPECT_EQ(c.collectionData(), d.collectionData());

        for (auto r : c)
        {
            d.addRawSceneActionInformation(r.type(), r.offsetInCollection());
        }
        EXPECT_EQ(c, d);
    }

    TEST_F(ASceneActionCollection, hasCorrectActionOffsetsAndSizesAfterWritingMultipleActionsWithDifferentTypes)
    {
        UInt32 bufferSize = 102;
        std::vector<Byte> buffer(bufferSize, 0xFA);
        String str("hello world");

        SceneActionCollection c;
        c.beginWriteSceneAction(ESceneActionId_TestAction);
        c.write(123u);
        UInt32 size_1 = sizeof(unsigned int);

        c.beginWriteSceneAction(ESceneActionId_AllocateNode);
        c.write(buffer.data(), bufferSize);
        c.write(static_cast<UInt8>(10));
        UInt32 size_2 = sizeof(UInt32) + bufferSize + sizeof(UInt8);

        c.beginWriteSceneAction(ESceneActionId_AllocateRenderable);
        c.write(Guid());
        c.write(str);
        UInt32 size_3 = sizeof(generic_uuid_t) + sizeof(UInt32) + static_cast<UInt32>(str.getLength());

        ASSERT_EQ(3u, c.numberOfActions());
        EXPECT_EQ(size_1 + size_2 + size_3, c.collectionData().size());

        SceneActionCollection::SceneActionReader reader_1(c[0]);
        EXPECT_EQ(ESceneActionId_TestAction, reader_1.type());
        EXPECT_EQ(0u, reader_1.offsetInCollection());
        EXPECT_EQ(size_1, reader_1.size());
        EXPECT_EQ(c.collectionData().data() + 0u, reader_1.data());

        SceneActionCollection::SceneActionReader reader_2(c[1]);
        EXPECT_EQ(ESceneActionId_AllocateNode, reader_2.type());
        EXPECT_EQ(size_1, reader_2.offsetInCollection());
        EXPECT_EQ(size_2, reader_2.size());
        EXPECT_EQ(c.collectionData().data() + size_1, reader_2.data());

        SceneActionCollection::SceneActionReader reader_3(c[2]);
        EXPECT_EQ(ESceneActionId_AllocateRenderable, reader_3.type());
        EXPECT_EQ(size_1 + size_2, reader_3.offsetInCollection());
        EXPECT_EQ(size_3, reader_3.size());
        EXPECT_EQ(c.collectionData().data() + size_1 + size_2, reader_3.data());
    }
}
