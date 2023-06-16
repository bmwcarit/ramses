//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Resource/ArrayResource.h"

namespace ramses_internal
{
    TEST(AArrayResource, hasCorrectArrayType)
    {
        ArrayResource vertArray(EResourceType_VertexArray, 5, EDataType::Vector3F, nullptr, ResourceCacheFlag(0u), {});
        ArrayResource indArray(EResourceType_IndexArray, 5, EDataType::Vector3F, nullptr, ResourceCacheFlag(0u), {});

        EXPECT_EQ(EResourceType_VertexArray, vertArray.getTypeID());
        EXPECT_EQ(EResourceType_IndexArray, indArray.getTypeID());
    }

    TEST(AArrayResource, hasCorrectElementCount)
    {
        ArrayResource vertArray(EResourceType_VertexArray, 5, EDataType::Vector3F, nullptr, ResourceCacheFlag(0u), {});

        EXPECT_EQ(5u, vertArray.getElementCount());
    }

    TEST(AArrayResource, hasCorrectElementType)
    {
        ArrayResource vertArray(EResourceType_VertexArray, 5, EDataType::Vector3F, nullptr, ResourceCacheFlag(0u), {});

        EXPECT_EQ(EDataType::Vector3F, vertArray.getElementType());
    }

    TEST(AArrayResource, hasCorrectCacheFlag)
    {
        const ResourceCacheFlag flag(15u);
        ArrayResource vertArray(EResourceType_VertexArray, 5, EDataType::Vector3F, nullptr, flag, {});
        EXPECT_EQ(flag, vertArray.getCacheFlag());
    }
}
