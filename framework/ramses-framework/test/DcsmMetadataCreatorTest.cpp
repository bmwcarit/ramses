//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmMetadataCreator.h"
#include "DcsmMetadataCreatorImpl.h"
#include "TestPngHeader.h"
#include "gtest/gtest.h"

namespace ramses
{
    class ADcsmMetadataCreator : public ::testing::Test
    {
    };

    TEST_F(ADcsmMetadataCreator, initiallyEmpty)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(ramses_internal::DcsmMetadata{}, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetPreviewImagePng)
    {
        const auto data = ramses_internal::TestPngHeader::GetValidHeader();
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setPreviewImagePng(data.data(), data.size()));

        ramses_internal::DcsmMetadata dm;
        dm.setPreviewImagePng(data.data(), data.size());
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, failsForInvalidPreviewImagePng)
    {
        const auto data = ramses_internal::TestPngHeader::GetInvalidHeader();
        DcsmMetadataCreator dmf;
        EXPECT_NE(StatusOK, dmf.setPreviewImagePng(data.data(), data.size()));
    }

    TEST_F(ADcsmMetadataCreator, canSetPreviewDescription)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setPreviewDescription(U"testtext"));

        ramses_internal::DcsmMetadata dm;
        dm.setPreviewDescription(U"testtext");
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetWidgetOrder)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setWidgetOrder(12345));

        ramses_internal::DcsmMetadata dm;
        dm.setWidgetOrder(12345);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetWidgetBackgroundID)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setWidgetBackgroundID(12345));

        ramses_internal::DcsmMetadata dm;
        dm.setWidgetBackgroundID(12345);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetWidgetHUDLineID)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setWidgetHUDLineID(12345));

        ramses_internal::DcsmMetadata dm;
        dm.setWidgetHUDLineID(12345);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetCarModel)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setCarModel(12345));

        ramses_internal::DcsmMetadata dm;
        dm.setCarModel(12345);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetCarModelView)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setCarModelView({ 1,2,3,4,5,6,7,0.1f,0.2f }, { 8,9 }));

        ramses_internal::DcsmMetadata dm;
        dm.setCarModelView({ 1,2,3,4,5,6,7,0.1f,0.2f }, { 8,9 });
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetCarModelViewExtended)
    {
        const CarModelViewMetadataExtended cm{10, 11, 0.1f, 0.2f};
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setCarModelViewExtended(cm));

        ramses_internal::DcsmMetadata dm;
        dm.setCarModelViewExtended(cm);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetCarModelVisibility)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setCarModelVisibility(true));

        ramses_internal::DcsmMetadata dm;
        dm.setCarModelVisibility(true);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetExclusiveBackground)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setExclusiveBackground(true));

        ramses_internal::DcsmMetadata dm;
        dm.setExclusiveBackground(true);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetStreamID)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setStreamID(123));

        ramses_internal::DcsmMetadata dm;
        dm.setStreamID(123);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetContentFlippedVertically)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setContentFlippedVertically(true));

        ramses_internal::DcsmMetadata dm;
        dm.setContentFlippedVertically(true);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetDisplayedDataFlags)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setDisplayedDataFlags(123));

        ramses_internal::DcsmMetadata dm;
        dm.setDisplayedDataFlags(123);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }

    TEST_F(ADcsmMetadataCreator, canSetLayoutAvailability)
    {
        DcsmMetadataCreator dmf;
        EXPECT_EQ(StatusOK, dmf.setLayoutAvailability(0x31));

        ramses_internal::DcsmMetadata dm;
        dm.setLayoutAvailability(0x31);
        EXPECT_EQ(dm, dmf.impl.getMetadata());
    }
}
