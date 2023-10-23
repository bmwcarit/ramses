//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"

namespace ramses::internal
{
    TEST(AArrayResource, hasCorrectArrayType)
    {
        ArrayResource vertArray(EResourceType::VertexArray, 5, EDataType::Vector3F, nullptr, {});
        ArrayResource indArray(EResourceType::IndexArray, 5, EDataType::Vector3F, nullptr, {});

        EXPECT_EQ(EResourceType::VertexArray, vertArray.getTypeID());
        EXPECT_EQ(EResourceType::IndexArray, indArray.getTypeID());
    }

    TEST(AArrayResource, hasCorrectElementCount)
    {
        ArrayResource vertArray(EResourceType::VertexArray, 5, EDataType::Vector3F, nullptr, {});

        EXPECT_EQ(5u, vertArray.getElementCount());
    }

    TEST(AArrayResource, hasCorrectElementType)
    {
        ArrayResource vertArray(EResourceType::VertexArray, 5, EDataType::Vector3F, nullptr, {});

        EXPECT_EQ(EDataType::Vector3F, vertArray.getElementType());
    }
}
