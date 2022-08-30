//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/DcsmMetadata.h"
#include "Utils/File.h"
#include "Utils/BinaryOutputStream.h"
#include "TestPngHeader.h"
#include "gtest/gtest.h"
#include "lodepng.h"

namespace ramses_internal
{
    static const ramses::CarModelViewMetadataExtended carModelViewExtA = {11, 12, 13.2f, 14};
    static const ramses::CarModelViewMetadataExtended carModelViewExtB = {21, 20.5f, 19, 18};

    class ADcsmMetadata : public testing::Test
    {
    public:
        ADcsmMetadata()
            : pngHeader(TestPngHeader::GetValidHeader())
            , pngLong(TestPngHeader::GetValidHeaderWithFakeData())
            , pngShort(pngLong.begin(), pngLong.begin() + 200)
            , m_widgetorder(123)
            , m_widgethudlineID(456)
        {
            EXPECT_TRUE(filledDm.setPreviewImagePng(pngLong.data(), pngLong.size()));
            EXPECT_TRUE(filledDm.setPreviewDescription(description));
            EXPECT_TRUE(filledDm.setWidgetOrder(m_widgetorder));
            EXPECT_TRUE(filledDm.setWidgetHUDLineID(m_widgethudlineID));
            EXPECT_TRUE(filledDm.setCarModel(1));
            EXPECT_TRUE(filledDm.setCarModelView({ 1,2,3,4,5,6,7, 0.1f, 5.f }, {8,9}));
            EXPECT_TRUE(filledDm.setCarModelViewExtended(carModelViewExtA));
            EXPECT_TRUE(filledDm.setCarModelVisibility(true));
            EXPECT_TRUE(filledDm.setExclusiveBackground(true));
            EXPECT_TRUE(filledDm.setStreamID(49));
            EXPECT_TRUE(filledDm.setDisplayedDataFlags(22));
            EXPECT_TRUE(filledDm.setContentFlippedVertically(true));
            EXPECT_TRUE(filledDm.setLayoutAvailability(3));
            EXPECT_TRUE(filledDm.setConfiguratorPriority(6));
        }

        DcsmMetadata serializeDeserialize(const DcsmMetadata& ref)
        {
            const auto vec = ref.toBinary();
            EXPECT_TRUE(vec.size() > 0);
            return DcsmMetadata(vec);
        }

        static std::vector<unsigned char> ReadFile(const char* fname)
        {
            File f(fname);
            EXPECT_TRUE(f.open(File::Mode::ReadOnlyBinary));
            UInt fileSize = 0;
            EXPECT_TRUE(f.getSizeInBytes(fileSize));
            std::vector<unsigned char> data(fileSize);
            UInt readBytes = 0;
            EXPECT_EQ(EStatus::Ok, f.read(data.data(), fileSize, readBytes));
            assert(readBytes > 0);
            return data;
        }

        static std::vector<unsigned char> CreateSizedPngBuffer(uint32_t width, uint32_t height)
        {
            std::vector<unsigned char> inData(width*height*4);
            std::vector<unsigned char> outData;
            const auto res = lodepng::encode(outData, inData, width, height);
            EXPECT_EQ(res, 0u);
            assert(res == 0);
            return outData;
        }

        std::vector<unsigned char> pngHeader;
        std::vector<unsigned char> pngLong;
        std::vector<unsigned char> pngShort;
        std::u32string description = U"test243243242";
        int32_t m_widgetorder;
        int32_t m_widgethudlineID;

        DcsmMetadata emptyDm;
        DcsmMetadata filledDm;
    };

    TEST_F(ADcsmMetadata, initiallyEmpty)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.empty());
        EXPECT_FALSE(dm.hasPreviewImagePng());
        EXPECT_FALSE(dm.hasPreviewDescription());
        EXPECT_FALSE(dm.hasWidgetOrder());
        EXPECT_FALSE(dm.hasWidgetHUDLineID());
        EXPECT_FALSE(dm.hasWidgetBackgroundID());
        EXPECT_FALSE(dm.hasCarModel());
        EXPECT_FALSE(dm.hasCarModelView());
        EXPECT_FALSE(dm.hasCarModelViewExtended());
        EXPECT_FALSE(dm.hasCarModelVisibility());
        EXPECT_FALSE(dm.hasExclusiveBackground());
        EXPECT_FALSE(dm.hasStreamID());
        EXPECT_FALSE(dm.hasDisplayedDataFlags());
        EXPECT_FALSE(dm.hasContentFlippedVertically());
        EXPECT_FALSE(dm.hasLayoutAvailability());
        EXPECT_FALSE(dm.hasConfiguratorPriority());
    }

    TEST_F(ADcsmMetadata, canSetGetPreviewImagePngHeader)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewImagePng(pngHeader.data(), pngHeader.size()));
        EXPECT_TRUE(dm.hasPreviewImagePng());
        EXPECT_FALSE(dm.hasPreviewDescription());

        const auto png = dm.getPreviewImagePng();
        EXPECT_EQ(pngHeader, png);
    }

    TEST_F(ADcsmMetadata, canSetGetPreviewImagePngShort)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewImagePng(pngShort.data(), pngShort.size()));
        const auto png = dm.getPreviewImagePng();
        EXPECT_EQ(pngShort, png);
    }

    TEST_F(ADcsmMetadata, canSetGetPreviewImagePngLong)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewImagePng(pngLong.data(), pngLong.size()));
        EXPECT_EQ(pngLong, dm.getPreviewImagePng());
    }

    TEST_F(ADcsmMetadata, cannotSetInvalidPng)
    {
        DcsmMetadata dm;
        EXPECT_FALSE(dm.setPreviewImagePng(nullptr, 0));
        const std::vector<unsigned char> wrongData{1, 2, 3};
        EXPECT_FALSE(dm.setPreviewImagePng(wrongData.data(), wrongData.size()));
        std::vector<unsigned char> wrongHeader(pngHeader);
        wrongHeader.back() = 0;
        EXPECT_FALSE(dm.setPreviewImagePng(wrongHeader.data(), wrongHeader.size()));
    }

    TEST_F(ADcsmMetadata, canHandleTooShortPngData)
    {
        DcsmMetadata dm;
        EXPECT_FALSE(dm.setPreviewImagePng(pngHeader.data(), 1));
    }

    TEST_F(ADcsmMetadata, canSetGetRealPngImage)
    {
        std::vector<unsigned char> img = ReadFile("res/sampleImage.png");
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewImagePng(img.data(), img.size()));
        EXPECT_TRUE(dm.hasPreviewImagePng());
        EXPECT_EQ(img, dm.getPreviewImagePng());
    }

    TEST_F(ADcsmMetadata, worksIfImageDataExactlyMaximumSize)
    {
        std::vector<unsigned char> img = ReadFile("res/sampleImage.png");
        img.resize(DcsmMetadata::MaxPreviewImageSize);
        EXPECT_TRUE(DcsmMetadata().setPreviewImagePng(img.data(), img.size()));
    }

    TEST_F(ADcsmMetadata, failsIfImageDataTooLarge)
    {
        std::vector<unsigned char> img = ReadFile("res/sampleImage.png");
        img.resize(DcsmMetadata::MaxPreviewImageSize + 1);
        EXPECT_FALSE(DcsmMetadata().setPreviewImagePng(img.data(), img.size()));
    }

    TEST_F(ADcsmMetadata, worksIfImageSizeExactlyMaximumSize)
    {
        const std::vector<unsigned char> img = CreateSizedPngBuffer(DcsmMetadata::MaxPreviewImageWidth,
                                                                    DcsmMetadata::MaxPreviewImageHeight);
        EXPECT_TRUE(DcsmMetadata().setPreviewImagePng(img.data(), img.size()));
    }

    TEST_F(ADcsmMetadata, failsIfImageWidthTooLarge)
    {
        const std::vector<unsigned char> img = CreateSizedPngBuffer(DcsmMetadata::MaxPreviewImageWidth + 1,
                                                                    DcsmMetadata::MaxPreviewImageHeight);
        EXPECT_FALSE(DcsmMetadata().setPreviewImagePng(img.data(), img.size()));
    }

    TEST_F(ADcsmMetadata, failsIfImageHeightTooLarge)
    {
        const std::vector<unsigned char> img = CreateSizedPngBuffer(DcsmMetadata::MaxPreviewImageWidth,
                                                                    DcsmMetadata::MaxPreviewImageHeight + 1);
        EXPECT_FALSE(DcsmMetadata().setPreviewImagePng(img.data(), img.size()));
    }

    TEST_F(ADcsmMetadata, canSetGetPreviewDescription)
    {
        DcsmMetadata dm;
        std::u32string str = U"foobar";
        dm.setPreviewDescription(str);
        EXPECT_TRUE(dm.hasPreviewDescription());
        EXPECT_EQ(str, dm.getPreviewDescription());
    }

    TEST_F(ADcsmMetadata, canSetGetWidgetOrder)
    {
        DcsmMetadata dm;
        dm.setWidgetOrder(12345);
        EXPECT_TRUE(dm.hasWidgetOrder());
        EXPECT_EQ(12345, dm.getWidgetOrder());
    }

    TEST_F(ADcsmMetadata, canSetGetWidgetHUDline)
    {
        DcsmMetadata dm;
        dm.setWidgetHUDLineID(12345);
        EXPECT_TRUE(dm.hasWidgetHUDLineID());
        EXPECT_EQ(12345, dm.getWidgetHUDLineID());
    }

    TEST_F(ADcsmMetadata, canSetGetCarModel)
    {
        DcsmMetadata dm;
        dm.setCarModel(123);
        EXPECT_TRUE(dm.hasCarModel());
        EXPECT_EQ(123, dm.getCarModel());
    }

    TEST_F(ADcsmMetadata, canSetGetCarModelView)
    {
        DcsmMetadata dm;
        constexpr ramses::CarModelViewMetadata values{ 7,6,5,4,3,2,1,0.1f,11.f };
        constexpr AnimationInformation timing{ 9,8 };
        dm.setCarModelView(values, timing);
        EXPECT_TRUE(dm.hasCarModelView());
        EXPECT_EQ(values, dm.getCarModelView());
        EXPECT_EQ(timing, dm.getCarModelViewAnimationInfo());
    }

    TEST_F(ADcsmMetadata, canSetGetCarModelViewExtended)
    {
        DcsmMetadata dm;
        dm.setCarModelViewExtended(carModelViewExtB);
        EXPECT_TRUE(dm.hasCarModelViewExtended());
        EXPECT_EQ(carModelViewExtB, dm.getCarModelViewExtended());
    }

    TEST_F(ADcsmMetadata, canSetGetCarModelVisibility)
    {
        DcsmMetadata dm;
        dm.setCarModelVisibility(true);
        EXPECT_TRUE(dm.hasCarModelVisibility());
        EXPECT_TRUE(dm.getCarModelVisibility());
    }

    TEST_F(ADcsmMetadata, canSetGetExclusiveBackground)
    {
        DcsmMetadata dm;
        dm.setExclusiveBackground(true);
        EXPECT_TRUE(dm.hasExclusiveBackground());
        EXPECT_TRUE(dm.getExclusiveBackground());
    }

    TEST_F(ADcsmMetadata, canSetGetStreamID)
    {
        DcsmMetadata dm;
        dm.setStreamID(123);
        EXPECT_TRUE(dm.hasStreamID());
        EXPECT_EQ(123, dm.getStreamID());
    }

    TEST_F(ADcsmMetadata, canSetGeContentFlippedVertically)
    {
        DcsmMetadata dm;
        dm.setContentFlippedVertically(true);
        EXPECT_TRUE(dm.hasContentFlippedVertically());
        EXPECT_EQ(true, dm.getContentFlippedVertically());
    }

    TEST_F(ADcsmMetadata, canSetGetDisplayedDataFlags)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setDisplayedDataFlags(123));
        EXPECT_TRUE(dm.hasDisplayedDataFlags());
        EXPECT_EQ(123u, dm.getDisplayedDataFlags());
    }

    TEST_F(ADcsmMetadata, canSetGetLayoutAvailability)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setLayoutAvailability(6u));
        EXPECT_TRUE(dm.hasLayoutAvailability());
        EXPECT_EQ(6u, dm.getLayoutAvailability());
    }

    TEST_F(ADcsmMetadata, canSetGetConfiguratorPriority)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setConfiguratorPriority(7u));
        EXPECT_TRUE(dm.hasConfiguratorPriority());
        EXPECT_EQ(7u, dm.getConfiguratorPriority());
    }

    TEST_F(ADcsmMetadata, canCompare)
    {
        EXPECT_TRUE(emptyDm == emptyDm);
        EXPECT_TRUE(filledDm == filledDm);

        EXPECT_FALSE(emptyDm != emptyDm);
        EXPECT_FALSE(filledDm != filledDm);

        EXPECT_FALSE(emptyDm == filledDm);
        EXPECT_TRUE(emptyDm != filledDm);
    }

    TEST_F(ADcsmMetadata, canCopyConstruct)
    {
        DcsmMetadata emptyCopy(emptyDm);
        EXPECT_EQ(emptyDm, emptyCopy);

        DcsmMetadata filledCopy(filledDm);
        EXPECT_EQ(filledDm, filledCopy);
    }

    TEST_F(ADcsmMetadata, canCopyAssign)
    {
        DcsmMetadata emptyCopy;
        emptyCopy = emptyDm;
        EXPECT_EQ(emptyDm, emptyCopy);

        DcsmMetadata filledCopy;
        filledCopy = filledDm;
        EXPECT_EQ(filledDm, filledCopy);
    }

    TEST_F(ADcsmMetadata, canMoveContruct)
    {
        DcsmMetadata emptyCopy(emptyDm);
        DcsmMetadata emptyMoved(std::move(emptyCopy));
        EXPECT_EQ(emptyDm, emptyMoved);

        DcsmMetadata filledCopy(filledDm);
        DcsmMetadata filledMoved(std::move(filledCopy));
        EXPECT_EQ(filledDm, filledMoved);
    }

    TEST_F(ADcsmMetadata, canMoveAssign)
    {
        DcsmMetadata emptyCopy(emptyDm);
        DcsmMetadata emptyMoved;
        emptyMoved = std::move(emptyCopy);
        EXPECT_EQ(emptyDm, emptyMoved);

        DcsmMetadata filledCopy(filledDm);
        DcsmMetadata filledMoved;
        filledMoved = std::move(filledCopy);
        EXPECT_EQ(filledDm, filledMoved);
    }

    TEST_F(ADcsmMetadata, canSerializeDeserializeEmpty)
    {
        EXPECT_EQ(emptyDm, serializeDeserialize(emptyDm));
    }

    TEST_F(ADcsmMetadata, canSerializeDeserializeFilled)
    {
        EXPECT_EQ(filledDm, serializeDeserialize(filledDm));
    }

    TEST_F(ADcsmMetadata, canSerializeDeserializeSomeSet)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewDescription(U"fkdsjflkdsjflkdshfls111111110"));
        EXPECT_EQ(dm, serializeDeserialize(dm));
    }

    TEST_F(ADcsmMetadata, canSetPngToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewImagePng(pngLong.data(), pngLong.size()));
        EXPECT_TRUE(dm.setPreviewImagePng(pngShort.data(), pngShort.size()));
        EXPECT_TRUE(dm.hasPreviewImagePng());
        EXPECT_EQ(pngShort, dm.getPreviewImagePng());
    }

    TEST_F(ADcsmMetadata, canSetDescriptionToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewDescription(U"foobar"));
        EXPECT_TRUE(dm.setPreviewDescription(U"baz"));
        EXPECT_TRUE(dm.hasPreviewDescription());
        EXPECT_EQ(U"baz", dm.getPreviewDescription());
    }

    TEST_F(ADcsmMetadata, canSetWidgetOrderToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setWidgetOrder(321));
        EXPECT_TRUE(dm.setWidgetOrder(123));
        EXPECT_TRUE(dm.hasWidgetOrder());
        EXPECT_EQ(123, dm.getWidgetOrder());
    }

    TEST_F(ADcsmMetadata, canSetWidgetbackgroundIDToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setWidgetBackgroundID(321));
        EXPECT_TRUE(dm.setWidgetBackgroundID(123));
        EXPECT_TRUE(dm.hasWidgetBackgroundID());
        EXPECT_EQ(123, dm.getWidgetBackgroundID());
    }

    TEST_F(ADcsmMetadata, zeroValueStillIsValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setWidgetBackgroundID(0));
        EXPECT_TRUE(dm.hasWidgetBackgroundID());

        EXPECT_TRUE(dm.setPreviewDescription({}));
        EXPECT_TRUE(dm.hasPreviewDescription());

        EXPECT_TRUE(dm.setWidgetOrder(0));
        EXPECT_TRUE(dm.hasWidgetOrder());

        EXPECT_TRUE(dm.setWidgetHUDLineID(0));
        EXPECT_TRUE(dm.hasWidgetHUDLineID());

        EXPECT_TRUE(dm.setCarModel(0));
        EXPECT_TRUE(dm.hasCarModel());

        EXPECT_TRUE(dm.setStreamID(0));
        EXPECT_TRUE(dm.hasStreamID());

        EXPECT_TRUE(dm.setDisplayedDataFlags(0));
        EXPECT_TRUE(dm.hasDisplayedDataFlags());

        EXPECT_TRUE(dm.setLayoutAvailability(0));
        EXPECT_TRUE(dm.hasLayoutAvailability());

        EXPECT_TRUE(dm.setConfiguratorPriority(0));
        EXPECT_TRUE(dm.hasConfiguratorPriority());
    }

    TEST_F(ADcsmMetadata, canSetWidgetHUDlineIDToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setWidgetHUDLineID(321));
        EXPECT_TRUE(dm.setWidgetHUDLineID(123));
        EXPECT_TRUE(dm.hasWidgetHUDLineID());
        EXPECT_EQ(123, dm.getWidgetHUDLineID());
    }

    TEST_F(ADcsmMetadata, canSetCarModelToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModel(321));
        EXPECT_TRUE(dm.setCarModel(123));
        EXPECT_TRUE(dm.hasCarModel());
        EXPECT_EQ(123, dm.getCarModel());
    }

    TEST_F(ADcsmMetadata, canSetCarModelViewToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModelView({ 1,2,3,4,5,6,7,0.8f,0.9f }, { 8,9 }));
        EXPECT_TRUE(dm.setCarModelView({ 7,6,5,4,3,2,1,0.1f,0.2f }, { 9,8 }));
        EXPECT_TRUE(dm.hasCarModelView());
        constexpr ramses::CarModelViewMetadata values{7,6,5,4,3,2,1,0.1f,0.2f};
        EXPECT_EQ(values, dm.getCarModelView());
        constexpr AnimationInformation timing{ 9,8 };
        EXPECT_EQ(timing, dm.getCarModelViewAnimationInfo());
    }

    TEST_F(ADcsmMetadata, canSetCarModelViewExtendedToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModelViewExtended(carModelViewExtA));
        EXPECT_TRUE(dm.setCarModelViewExtended(carModelViewExtB));
        EXPECT_TRUE(dm.hasCarModelViewExtended());
        EXPECT_EQ(carModelViewExtB, dm.getCarModelViewExtended());
    }

    TEST_F(ADcsmMetadata, canSetCarModelVisibilityToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModelVisibility(false));
        EXPECT_TRUE(dm.setCarModelVisibility(true));
        EXPECT_TRUE(dm.hasCarModelVisibility());
        EXPECT_TRUE(dm.getCarModelVisibility());
    }

    TEST_F(ADcsmMetadata, canSetExclusiveBackgroundToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setExclusiveBackground(false));
        EXPECT_TRUE(dm.setExclusiveBackground(true));
        EXPECT_TRUE(dm.hasExclusiveBackground());
        EXPECT_TRUE(dm.getExclusiveBackground());
    }

    TEST_F(ADcsmMetadata, canUpdatePngFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewImagePng(pngLong.data(), pngLong.size()));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setPreviewImagePng(pngShort.data(), pngShort.size()));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasPreviewImagePng());
        EXPECT_EQ(pngShort, dm.getPreviewImagePng());
    }

    TEST_F(ADcsmMetadata, canUpdateDescriptionFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewDescription(U"foobar"));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setPreviewDescription(U"baz"));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasPreviewDescription());
        EXPECT_EQ(U"baz", dm.getPreviewDescription());
    }

    TEST_F(ADcsmMetadata, canUpdateWidgetOrderFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setWidgetOrder(1234));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setWidgetOrder(567));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasWidgetOrder());
        EXPECT_EQ(567, dm.getWidgetOrder());
    }

    TEST_F(ADcsmMetadata, canUpdateWidgetbackgroundIDFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setWidgetBackgroundID(1234));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setWidgetBackgroundID(567));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasWidgetBackgroundID());
        EXPECT_EQ(567, dm.getWidgetBackgroundID());
    }

    TEST_F(ADcsmMetadata, canUpdateWidgetHUDLineFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setWidgetHUDLineID(1234));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setWidgetHUDLineID(567));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasWidgetHUDLineID());
        EXPECT_EQ(567, dm.getWidgetHUDLineID());
    }

    TEST_F(ADcsmMetadata, canUpdateCarModelFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModel(1234));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setCarModel(567));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasCarModel());
        EXPECT_EQ(567, dm.getCarModel());
    }

    TEST_F(ADcsmMetadata, canUpdateCarModelViewLineFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModelView({ 1,2,3,4,5,6,7,0.8f,0.9f }, { 8,9 }));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setCarModelView({ 7,6,5,4,3,2,1,0.1f,0.2f }, { 9,8 }));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasCarModelView());
        constexpr ramses::CarModelViewMetadata values{7,6,5,4,3,2,1,0.1f,0.2f};
        EXPECT_EQ(values, dm.getCarModelView());
        constexpr AnimationInformation timing{ 9,8 };
        EXPECT_EQ(timing, dm.getCarModelViewAnimationInfo());
    }

    TEST_F(ADcsmMetadata, canUpdateCarModelViewExtendedFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModelViewExtended(carModelViewExtA));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setCarModelViewExtended(carModelViewExtB));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasCarModelViewExtended());
        EXPECT_EQ(carModelViewExtB, dm.getCarModelViewExtended());
    }

    TEST_F(ADcsmMetadata, canUpdateCarModelVisibilityFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarModelVisibility(false));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setCarModelVisibility(true));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasCarModelVisibility());
        EXPECT_TRUE(dm.getCarModelVisibility());
    }

    TEST_F(ADcsmMetadata, canUpdateExclusiveBackgroundFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setExclusiveBackground(false));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setExclusiveBackground(true));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasExclusiveBackground());
        EXPECT_TRUE(dm.getExclusiveBackground());
    }

    TEST_F(ADcsmMetadata, canUpdateStreamIDFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setStreamID(456));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setStreamID(1));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasStreamID());
        EXPECT_EQ(1, dm.getStreamID());
    }

    TEST_F(ADcsmMetadata, canUpdateContentFlippedVerticallyFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setContentFlippedVertically(false));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setContentFlippedVertically(true));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasContentFlippedVertically());
        EXPECT_TRUE(dm.getContentFlippedVertically());
    }

    TEST_F(ADcsmMetadata, canUpdateDisplayedDataFlagsFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setDisplayedDataFlags(22));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setDisplayedDataFlags(11));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasDisplayedDataFlags());
        EXPECT_EQ(11u, dm.getDisplayedDataFlags());
    }

    TEST_F(ADcsmMetadata, canUpdateLayoutAvailabilityFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setLayoutAvailability(23u));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setLayoutAvailability(14u));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasLayoutAvailability());
        EXPECT_EQ(14u, dm.getLayoutAvailability());
    }

    TEST_F(ADcsmMetadata, canUpdateConfiguratorPriorityFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setConfiguratorPriority(24u));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setConfiguratorPriority(15u));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasConfiguratorPriority());
        EXPECT_EQ(15u, dm.getConfiguratorPriority());
    }

    TEST_F(ADcsmMetadata, canUpdateEmptyWithValues)
    {
        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setPreviewDescription(U"baz"));
        EXPECT_TRUE(otherDm.setPreviewImagePng(pngShort.data(), pngShort.size()));
        EXPECT_TRUE(otherDm.setWidgetOrder(1234));
        EXPECT_TRUE(otherDm.setWidgetBackgroundID(567));
        EXPECT_TRUE(otherDm.setWidgetHUDLineID(789));
        EXPECT_TRUE(otherDm.setCarModel(1234));
        EXPECT_TRUE(otherDm.setCarModelView({ 1,2,3,4,5,6,7,0.1f,0.2f }, { 8,9 }));
        EXPECT_TRUE(otherDm.setCarModelViewExtended(carModelViewExtA));
        EXPECT_TRUE(otherDm.setCarModelVisibility(true));
        EXPECT_TRUE(otherDm.setExclusiveBackground(true));
        EXPECT_TRUE(otherDm.setStreamID(45));
        EXPECT_TRUE(otherDm.setDisplayedDataFlags(22));
        EXPECT_TRUE(otherDm.setContentFlippedVertically(true));
        EXPECT_TRUE(otherDm.setLayoutAvailability(15));
        EXPECT_TRUE(otherDm.setConfiguratorPriority(16));

        DcsmMetadata dm;
        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasPreviewImagePng());
        EXPECT_EQ(pngShort, dm.getPreviewImagePng());
        EXPECT_TRUE(dm.hasPreviewDescription());
        EXPECT_EQ(U"baz", dm.getPreviewDescription());
        EXPECT_TRUE(dm.hasWidgetOrder());
        EXPECT_EQ(1234, dm.getWidgetOrder());
        EXPECT_TRUE(dm.hasWidgetBackgroundID());
        EXPECT_EQ(567, dm.getWidgetBackgroundID());
        EXPECT_TRUE(dm.hasWidgetHUDLineID());
        EXPECT_EQ(789, dm.getWidgetHUDLineID());
        EXPECT_TRUE(dm.hasCarModel());
        EXPECT_EQ(1234, dm.getCarModel());
        EXPECT_TRUE(dm.hasCarModelView());
        EXPECT_EQ(ramses::CarModelViewMetadata({1,2,3,4,5,6,7,0.1f,0.2f}), dm.getCarModelView());
        EXPECT_TRUE(dm.hasCarModelViewExtended());
        EXPECT_EQ(carModelViewExtA, dm.getCarModelViewExtended());
        EXPECT_EQ(AnimationInformation({ 8,9 }), dm.getCarModelViewAnimationInfo());
        EXPECT_TRUE(dm.hasCarModelVisibility());
        EXPECT_TRUE(dm.getCarModelVisibility());
        EXPECT_TRUE(dm.hasExclusiveBackground());
        EXPECT_TRUE(dm.getExclusiveBackground());
        EXPECT_TRUE(dm.hasStreamID());
        EXPECT_EQ(45, dm.getStreamID());
        EXPECT_TRUE(dm.hasDisplayedDataFlags());
        EXPECT_EQ(22u, dm.getDisplayedDataFlags());
        EXPECT_TRUE(dm.hasContentFlippedVertically());
        EXPECT_TRUE(dm.getContentFlippedVertically());
        EXPECT_TRUE(dm.hasLayoutAvailability());
        EXPECT_EQ(15u, dm.getLayoutAvailability());
        EXPECT_TRUE(dm.hasConfiguratorPriority());
        EXPECT_EQ(16u, dm.getConfiguratorPriority());
    }

    TEST_F(ADcsmMetadata, canSkipDeserializeUnknownTypes)
    {
        BinaryOutputStream os;
        os << static_cast<uint32_t>(1) // version
           << static_cast<uint32_t>(2) // entries

           << static_cast<uint32_t>(55) // unknown type
           << static_cast<uint32_t>(16) // unknown size
           << static_cast<uint64_t>(2) // unknown data
           << static_cast<uint64_t>(3)

           << static_cast<uint32_t>(1) // png type
           << static_cast<uint32_t>(2) // png size
           << static_cast<uint8_t>(123) // png data
           << static_cast<uint8_t>(65);

        DcsmMetadata dm(os.release());
        EXPECT_TRUE(dm.hasPreviewImagePng());
        EXPECT_FALSE(dm.hasPreviewDescription());
        EXPECT_EQ(U"", dm.getPreviewDescription());
        EXPECT_EQ(std::vector<unsigned char>({123, 65}), dm.getPreviewImagePng());
    }

    TEST_F(ADcsmMetadata, ignoresUnexpectedMetadataVersion)
    {
        BinaryOutputStream os;
        os << static_cast<uint32_t>(100) // unsupported version
           << static_cast<uint32_t>(1) // entries

           << static_cast<uint32_t>(1) // png type
           << static_cast<uint32_t>(1) // png size
           << static_cast<uint8_t>(123); // png data
        DcsmMetadata dm(os.release());
        EXPECT_FALSE(dm.hasPreviewImagePng());
    }

    TEST_F(ADcsmMetadata, canUseInLogs)
    {
        EXPECT_TRUE(fmt::to_string(emptyDm).size() > 0);
        EXPECT_TRUE(fmt::to_string(filledDm).size() > 0);
    }

    TEST_F(ADcsmMetadata, containsStreamIDwhenLogged)
    {
        // ensure log contains streamid
        const std::string s = fmt::to_string(filledDm);
        EXPECT_NE(s.find("49"), std::string::npos);
    }

    TEST_F(ADcsmMetadata, containsContentFlippedVerticallywhenLogged)
    {
        const std::string s = fmt::to_string(filledDm);
        EXPECT_NE(s.find("contentFlippedVertically:true"), std::string::npos);
    }

    TEST_F(ADcsmMetadata, containsDisplayedDataFlagsWhenLogged)
    {
        EXPECT_NE(fmt::to_string(filledDm).find("22"), std::string::npos);
    }

    TEST_F(ADcsmMetadata, containsLayoutAvailabilityWhenLogged)
    {
        EXPECT_NE(fmt::to_string(filledDm).find("layoutAvailability:3"), std::string::npos);
    }

    TEST_F(ADcsmMetadata, containsConfiguratorPriorityWhenLogged)
    {
        EXPECT_NE(fmt::to_string(filledDm).find("configuratorPriority:6"), std::string::npos);
    }

    TEST_F(ADcsmMetadata, containsFullSetOfCarviewWhenLogged)
    {
        // ensure log contains expected values
        const std::string s = fmt::to_string(filledDm);
        EXPECT_NE(s.find('1'), std::string::npos);  //pitch
        EXPECT_NE(s.find('2'), std::string::npos);  //yaw
        EXPECT_NE(s.find('3'), std::string::npos);  //distance
        EXPECT_NE(s.find('4'), std::string::npos);  //x
        EXPECT_NE(s.find('5'), std::string::npos);  //y
        EXPECT_NE(s.find('6'), std::string::npos);  //z
        EXPECT_NE(s.find('7'), std::string::npos);  //fov
        EXPECT_NE(s.find('8'), std::string::npos);  //animation start
        EXPECT_NE(s.find('9'), std::string::npos);  //animation end
    }

    TEST_F(ADcsmMetadata, containsCarModelViewExtendedWhenLogged)
    {
        const std::string s = fmt::to_string(filledDm);
        EXPECT_NE(s.find("carViewExt:r:11,t(12,13.2,14)"), std::string::npos);
    }

    TEST_F(ADcsmMetadata, canLogPreviewDescription)
    {
        DcsmMetadata dm;
        dm.setPreviewDescription(U"asd");
        const std::string s = fmt::to_string(dm);
        EXPECT_NE(s.find("desc:3[61;73;64;]"), std::string::npos);
    }
}
