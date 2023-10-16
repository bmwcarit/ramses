//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneTest.h"

using namespace testing;

namespace ramses::internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsZeroTotalDataLayoutsUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getDataLayoutCount());
    }

    TYPED_TEST(AScene, CreatingDataLayoutIncreasesDataLayoutCount)
    {
        this->m_scene.allocateDataLayout({}, ResourceContentHash(123u, 0u), {});
        EXPECT_EQ(1u, this->m_scene.getDataLayoutCount());
    }

    TYPED_TEST(AScene, CanGetAndSetEffectHash)
    {
        ResourceContentHash effectHash(123u, 0u);
        //This will internally call setEffectHash
        const DataLayoutHandle dataLayoutHandle = this->m_scene.allocateDataLayout({}, effectHash, {});
        const DataLayout& dataLayout = this->m_scene.getDataLayout(dataLayoutHandle);
        EXPECT_EQ(effectHash, dataLayout.getEffectHash());
    }

    TYPED_TEST(AScene, CanGetNumberOfDataFieldsInLayout)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({}, ResourceContentHash(123u, 0u), {});
        EXPECT_EQ(0u, this->m_scene.getDataLayout(dataLayout).getFieldCount());

        const DataLayoutHandle dataLayout2 = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType::Float), DataFieldInfo(EDataType::Int32) }, ResourceContentHash(456u, 0u), {});
        EXPECT_EQ(2u, this->m_scene.getDataLayout(dataLayout2).getFieldCount());
    }

    TYPED_TEST(AScene, ReturnsCorrectDataFieldType)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash(123u, 0u), {});
        EXPECT_EQ(EDataType::Float, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(0u)).dataType);
    }

    TYPED_TEST(AScene, ReturnsCorrectDataFieldElements)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({DataFieldInfo(EDataType::Float, 3u)}, ResourceContentHash(123u, 0u), {});
        EXPECT_EQ(3u, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(0u)).elementCount);
    }

    TYPED_TEST(AScene, SemanticCanBeSetAndRetrieved)
    {
        const DataLayoutHandle dataLayout = this->m_scene.allocateDataLayout({DataFieldInfo(EDataType::Float), DataFieldInfo(EDataType::Matrix33F, 1u, EFixedSemantics::ModelViewMatrix33)}, ResourceContentHash(123u, 0u), {});

        EXPECT_EQ(EFixedSemantics::Invalid, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(0u)).semantics);
        EXPECT_EQ(EFixedSemantics::ModelViewMatrix33, this->m_scene.getDataLayout(dataLayout).getField(DataFieldHandle(1u)).semantics);
    }
}
