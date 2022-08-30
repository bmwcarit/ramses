//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "DcsmMetadataUpdateImpl.h"
#include "TestPngHeader.h"
#include "gtest/gtest.h"

namespace ramses
{
    class ADcsmMetadataUpdate : public ::testing::Test
    {
    public:
        ADcsmMetadataUpdate()
            : dmp(*new DcsmMetadataUpdateImpl)
        {
        }

        DcsmMetadataUpdate dmp;
        ramses_internal::DcsmMetadata md;
    };

    TEST_F(ADcsmMetadataUpdate, emptyObjectReturnsEmptyMetadata)
    {
        dmp.impl.setMetadata(md);
        EXPECT_FALSE(dmp.hasPreviewImagePng());
        EXPECT_EQ(0u, dmp.getPreviewImagePng().size());
        EXPECT_FALSE(dmp.hasPreviewDescription());
        EXPECT_EQ(0u, dmp.getPreviewDescription().size());
        EXPECT_FALSE(dmp.hasWidgetOrder());
        EXPECT_FALSE(dmp.hasWidgetBackgroundID());
        EXPECT_FALSE(dmp.hasWidgetHUDLineID());
        EXPECT_FALSE(dmp.hasCarModel());
        EXPECT_FALSE(dmp.hasCarModelView());
        EXPECT_FALSE(dmp.hasCarModelVisibility());
        EXPECT_FALSE(dmp.hasExclusiveBackground());
        EXPECT_FALSE(dmp.hasStreamID());
        EXPECT_FALSE(dmp.hasDisplayedDataFlags());
        EXPECT_FALSE(dmp.hasContentFlippedVertically());
        EXPECT_FALSE(dmp.hasLayoutAvailability());
        EXPECT_FALSE(dmp.hasConfiguratorPriority());
    }

    TEST_F(ADcsmMetadataUpdate, returnsPreviewImagePng)
    {
        const auto data = ramses_internal::TestPngHeader::GetValidHeader();
        EXPECT_TRUE(md.setPreviewImagePng(data.data(), data.size()));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasPreviewImagePng());
        EXPECT_EQ(data, dmp.getPreviewImagePng());
    }

    TEST_F(ADcsmMetadataUpdate, returnsPreviewDescription)
    {
        EXPECT_TRUE(md.setPreviewDescription(U"sometext"));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasPreviewDescription());
        EXPECT_EQ(U"sometext", dmp.getPreviewDescription());
    }

    TEST_F(ADcsmMetadataUpdate, returnsWidgetOrder)
    {
        EXPECT_TRUE(md.setWidgetOrder(1234));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasWidgetOrder());
        EXPECT_EQ(1234, dmp.getWidgetOrder());
    }

    TEST_F(ADcsmMetadataUpdate, returnsWidgetBackgroundID)
    {
        EXPECT_TRUE(md.setWidgetBackgroundID(1234));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasWidgetBackgroundID());
        EXPECT_EQ(1234, dmp.getWidgetBackgroundID());
    }

    TEST_F(ADcsmMetadataUpdate, returnsWidgetHUDLineID)
    {
        EXPECT_TRUE(md.setWidgetHUDLineID(1234));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasWidgetHUDLineID());
        EXPECT_EQ(1234, dmp.getWidgetHUDLineID());
    }

    TEST_F(ADcsmMetadataUpdate, returnsCarModel)
    {
        EXPECT_TRUE(md.setCarModel(1234));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasCarModel());
        EXPECT_EQ(1234, dmp.getCarModel());
    }

    TEST_F(ADcsmMetadataUpdate, returnsCarModelView)
    {
        constexpr CarModelViewMetadata values{ 1,2,3,4,5,6,7,0.1f,0.2f };
        constexpr ramses_internal::AnimationInformation timing{ 8,9 };

        EXPECT_TRUE(md.setCarModelView(values, timing));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasCarModelView());

        EXPECT_EQ(values, dmp.getCarModelView());
        constexpr AnimationInformation timingp{ 8,9 };
        EXPECT_EQ(timingp, dmp.getCarModelViewAnimationInfo());
    }

    TEST_F(ADcsmMetadataUpdate, returnsCarModelViewExtended)
    {
        constexpr CarModelViewMetadataExtended cmExt{ 0.1f,0.2f,9,10 };

        EXPECT_TRUE(md.setCarModelViewExtended(cmExt));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasCarModelViewExtended());

        EXPECT_EQ(cmExt, dmp.getCarModelViewExtended());
    }

    TEST_F(ADcsmMetadataUpdate, returnsCarModelVisibility)
    {
        EXPECT_TRUE(md.setCarModelVisibility(true));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasCarModelVisibility());
        EXPECT_TRUE(dmp.getCarModelVisibility());
    }

    TEST_F(ADcsmMetadataUpdate, returnsExclusiveBackground)
    {
        EXPECT_TRUE(md.setExclusiveBackground(true));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasExclusiveBackground());
        EXPECT_TRUE(dmp.getExclusiveBackground());
    }

    TEST_F(ADcsmMetadataUpdate, returnsStreamID)
    {
        EXPECT_TRUE(md.setStreamID(13));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasStreamID());
        EXPECT_EQ(13, dmp.getStreamID());
    }

    TEST_F(ADcsmMetadataUpdate, returnsContentFlippedVertically)
    {
        EXPECT_TRUE(md.setContentFlippedVertically(true));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasContentFlippedVertically());
        EXPECT_TRUE(dmp.getContentFlippedVertically());
    }

    TEST_F(ADcsmMetadataUpdate, returnsDisplayedDataFlags)
    {
        EXPECT_TRUE(md.setDisplayedDataFlags(13));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasDisplayedDataFlags());
        EXPECT_EQ(13u, dmp.getDisplayedDataFlags());
    }

    TEST_F(ADcsmMetadataUpdate, returnsLayoutAvailability)
    {
        EXPECT_TRUE(md.setLayoutAvailability(5));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasLayoutAvailability());
        EXPECT_EQ(5u, dmp.getLayoutAvailability());
    }

    TEST_F(ADcsmMetadataUpdate, returnsConfiguratorPriority)
    {
        EXPECT_TRUE(md.setConfiguratorPriority(6));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasConfiguratorPriority());
        EXPECT_EQ(6u, dmp.getConfiguratorPriority());
    }
}
