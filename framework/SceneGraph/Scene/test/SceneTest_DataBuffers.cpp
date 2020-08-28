//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"
#include "SceneAPI/GeometryDataBuffer.h"
#include <array>

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, DataBufferCreated)
    {
        EXPECT_EQ(0u, this->m_scene.getDataBufferCount());

        const DataBufferHandle dataBuffer = this->m_scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u);

        EXPECT_EQ(1u, this->m_scene.getDataBufferCount());
        EXPECT_TRUE(this->m_scene.isDataBufferAllocated(dataBuffer));
    }

    TYPED_TEST(AScene, DataBufferReleased)
    {
        const DataBufferHandle dataBuffer = this->m_scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u);
        this->m_scene.releaseDataBuffer(dataBuffer);

        EXPECT_FALSE(this->m_scene.isDataBufferAllocated(dataBuffer));
    }

    TYPED_TEST(AScene, DoesNotContainDataBufferWhichWasNotCreated)
    {
        EXPECT_FALSE(this->m_scene.isDataBufferAllocated(DataBufferHandle(1u)));
    }

    TYPED_TEST(AScene, CanGetDataBufferType)
    {
        const DataBufferHandle dataBuffer = this->m_scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u);
        EXPECT_EQ(EDataBufferType::IndexBuffer, this->m_scene.getDataBuffer(dataBuffer).bufferType);
    }

    TYPED_TEST(AScene, CanGetDataBufferDataType)
    {
        const DataBufferHandle dataBuffer = this->m_scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u);
        EXPECT_EQ(EDataType::UInt32, this->m_scene.getDataBuffer(dataBuffer).dataType);
    }

    TYPED_TEST(AScene, CanGetDataBufferSize)
    {
        const DataBufferHandle dataBuffer = this->m_scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u);
        EXPECT_EQ(10u, this->m_scene.getDataBuffer(dataBuffer).data.size());
    }

    TYPED_TEST(AScene, CanUpdateDataBuffer)
    {
        const DataBufferHandle dataBuffer = this->m_scene.allocateDataBuffer(EDataBufferType::IndexBuffer, EDataType::UInt32, 10u);
        this->m_scene.updateDataBuffer(dataBuffer, 0u, 4u, std::array<Byte, 4>{{ 0x0A, 0x1B, 0x2C, 0x3D }}.data());
        EXPECT_EQ(0x0A, this->m_scene.getDataBuffer(dataBuffer).data[0]);
        EXPECT_EQ(0x1B, this->m_scene.getDataBuffer(dataBuffer).data[1]);
        EXPECT_EQ(0x2C, this->m_scene.getDataBuffer(dataBuffer).data[2]);
        EXPECT_EQ(0x3D, this->m_scene.getDataBuffer(dataBuffer).data[3]);

        this->m_scene.updateDataBuffer(dataBuffer, 1u, 2u, std::array<Byte, 2>{ { 0x77, 0xAB }}.data());
        EXPECT_EQ(0x0A, this->m_scene.getDataBuffer(dataBuffer).data[0]); //stays same
        EXPECT_EQ(0x77, this->m_scene.getDataBuffer(dataBuffer).data[1]);
        EXPECT_EQ(0xAB, this->m_scene.getDataBuffer(dataBuffer).data[2]);
        EXPECT_EQ(0x3D, this->m_scene.getDataBuffer(dataBuffer).data[3]); //stays same
    }
}
