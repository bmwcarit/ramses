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
            filledDm.setPreviewImagePng(pngLong.data(), pngLong.size());
            filledDm.setPreviewDescription(description);
            filledDm.setWidgetOrder(m_widgetorder);
            filledDm.setWidgetHUDLineID(m_widgethudlineID);
            filledDm.setCarModel(1);
            filledDm.setCarModelView({ 1,2,3,4,5,6,7 }, {8,9});
            filledDm.setCarCameraPlanes({0.1f, 5.f});
            filledDm.setCarModelVisibility(true);
            filledDm.setExclusiveBackground(true);
            filledDm.setStreamID(49);
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
        EXPECT_FALSE(dm.hasCarModelVisibility());
        EXPECT_FALSE(dm.hasExclusiveBackground());
        EXPECT_FALSE(dm.hasStreamID());
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
        constexpr ramses::CarModelViewMetadata values{ 7,6,5,4,3,2,1 };
        constexpr AnimationInformation timing{ 9,8 };
        dm.setCarModelView(values, timing);
        EXPECT_TRUE(dm.hasCarModelView());
        EXPECT_EQ(values, dm.getCarModelView());
        EXPECT_EQ(timing, dm.getCarModelViewAnimationInfo());
    }

    TEST_F(ADcsmMetadata, canSetGetCarCameraPlanes)
    {
        DcsmMetadata dm;
        constexpr ramses::CarCameraPlaneMetadata values{ 2.f, 3.f };
        dm.setCarCameraPlanes(values);
        EXPECT_TRUE(dm.hasCarCameraPlanes());
        EXPECT_EQ(values, dm.getCarCameraPlanes());
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
        EXPECT_TRUE(dm.setCarModelView({ 1,2,3,4,5,6,7 }, { 8,9 }));
        EXPECT_TRUE(dm.setCarModelView({ 7,6,5,4,3,2,1 }, { 9,8 }));
        EXPECT_TRUE(dm.hasCarModelView());
        constexpr ramses::CarModelViewMetadata values{7,6,5,4,3,2,1};
        EXPECT_EQ(values, dm.getCarModelView());
        constexpr AnimationInformation timing{ 9,8 };
        EXPECT_EQ(timing, dm.getCarModelViewAnimationInfo());
    }

    TEST_F(ADcsmMetadata, canSetCarCameraPlanesToNewValue)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarCameraPlanes({1.f, 2.f}));
        EXPECT_TRUE(dm.setCarCameraPlanes({4.f, 5.f}));
        EXPECT_TRUE(dm.hasCarCameraPlanes());
        constexpr ramses::CarCameraPlaneMetadata values{4.f, 5.f};
        EXPECT_EQ(values, dm.getCarCameraPlanes());
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
        EXPECT_TRUE(dm.setCarModelView({ 1,2,3,4,5,6,7 }, { 8,9 }));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setCarModelView({ 7,6,5,4,3,2,1 }, { 9,8 }));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasCarModelView());
        constexpr ramses::CarModelViewMetadata values{7,6,5,4,3,2,1};
        EXPECT_EQ(values, dm.getCarModelView());
        constexpr AnimationInformation timing{ 9,8 };
        EXPECT_EQ(timing, dm.getCarModelViewAnimationInfo());
    }

    TEST_F(ADcsmMetadata, canUpdateCarCameraPlanesLineFromOther)
    {
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setCarCameraPlanes({1.f, 3.f}));

        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setCarCameraPlanes({6.1f, 7.f}));

        dm.updateFromOther(otherDm);
        EXPECT_TRUE(dm.hasCarCameraPlanes());
        constexpr ramses::CarCameraPlaneMetadata values{6.1f, 7.f};
        EXPECT_EQ(values, dm.getCarCameraPlanes());
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
    TEST_F(ADcsmMetadata, canUpdateEmptyWithValues)
    {
        DcsmMetadata otherDm;
        EXPECT_TRUE(otherDm.setPreviewDescription(U"baz"));
        EXPECT_TRUE(otherDm.setPreviewImagePng(pngShort.data(), pngShort.size()));
        EXPECT_TRUE(otherDm.setWidgetOrder(1234));
        EXPECT_TRUE(otherDm.setWidgetBackgroundID(567));
        EXPECT_TRUE(otherDm.setWidgetHUDLineID(789));
        EXPECT_TRUE(otherDm.setCarModel(1234));
        EXPECT_TRUE(otherDm.setCarModelView({ 1,2,3,4,5,6,7 }, { 8,9 }));
        EXPECT_TRUE(otherDm.setCarCameraPlanes({1.f, 2.f}));
        EXPECT_TRUE(otherDm.setCarModelVisibility(true));
        EXPECT_TRUE(otherDm.setExclusiveBackground(true));
        EXPECT_TRUE(otherDm.setStreamID(45));

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
        EXPECT_EQ(ramses::CarModelViewMetadata({1,2,3,4,5,6,7}), dm.getCarModelView());
        EXPECT_EQ(AnimationInformation({ 8,9 }), dm.getCarModelViewAnimationInfo());
        EXPECT_TRUE(dm.hasCarCameraPlanes());
        EXPECT_EQ(ramses::CarCameraPlaneMetadata({1.f, 2.f}), dm.getCarCameraPlanes());
        EXPECT_TRUE(dm.hasCarModelVisibility());
        EXPECT_TRUE(dm.getCarModelVisibility());
        EXPECT_TRUE(dm.hasExclusiveBackground());
        EXPECT_TRUE(dm.getExclusiveBackground());
        EXPECT_TRUE(dm.hasStreamID());
        EXPECT_EQ(45, dm.getStreamID());
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
        EXPECT_TRUE(StringOutputStream::ToString(emptyDm).size() > 0);
        EXPECT_TRUE(StringOutputStream::ToString(filledDm).size() > 0);
    }
}
