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
        EXPECT_TRUE(md.setCarModelView({1,2,3,4,5,6}));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasCarModelView());

        ramses::CarModelViewMetadata values{ 1,2,3,4,5,6 };
        EXPECT_EQ(values, dmp.getCarModelView());
    }

    TEST_F(ADcsmMetadataUpdate, returnsCarModelVisibility)
    {
        EXPECT_TRUE(md.setCarModelVisibility(true));
        dmp.impl.setMetadata(md);
        EXPECT_TRUE(dmp.hasCarModelVisibility());
        EXPECT_TRUE(dmp.getCarModelVisibility());
    }
}
