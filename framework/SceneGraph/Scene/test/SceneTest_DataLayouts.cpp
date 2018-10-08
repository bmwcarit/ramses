//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneTest.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_CASE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsZeroTotalDataLayoutsUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getDataLayoutCount());
    }

    TYPED_TEST(AScene, CreatingDataLayoutIncreasesDataLayoutCount)
    {
        this->m_scene.allocateDataLayout({});
        EXPECT_EQ(1u, this->m_scene.getDataLayoutCount());
    }

    TYPED_TEST(AScene, CanGetNumberOfDataFieldsInLayout)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({});
        EXPECT_EQ(0u, this->m_scene.getDataLayout(dataLayout).getFieldCount());

        const DataLayoutHandle dataLayout2 = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_Float), DataFieldInfo(EDataType_Int32) });
        EXPECT_EQ(2u, this->m_scene.getDataLayout(dataLayout2).getFieldCount());
    }

    TYPED_TEST(AScene, ReturnsCorrectDataFieldType)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_Float) });
        EXPECT_EQ(EDataType_Float, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(0u)).dataType);
    }

    TYPED_TEST(AScene, ReturnsCorrectDataFieldElements)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_Float, 3u) });
        EXPECT_EQ(3u, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(0u)).elementCount);
    }

    TYPED_TEST(AScene, SemanticCanBeSetAndRetrieved)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType_Float), DataFieldInfo(EDataType_Matrix33F, 1u, EFixedSemantics_ModelViewMatrix33) });

        EXPECT_EQ(EFixedSemantics_Invalid, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(0u)).semantics);
        EXPECT_EQ(EFixedSemantics_ModelViewMatrix33, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(1u)).semantics);
    }
}
