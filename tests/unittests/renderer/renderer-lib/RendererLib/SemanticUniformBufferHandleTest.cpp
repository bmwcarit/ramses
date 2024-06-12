//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SemanticUniformBufferHandle.h"
#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>

namespace ramses::internal
{
    using namespace testing;

    class ASemanticUniformBufferHandle : public ::testing::Test
    {
    };

    TEST_F(ASemanticUniformBufferHandle, canConstructHandleForModelUBO)
    {
        constexpr RenderableHandle renderable{ 1u };
        constexpr SemanticUniformBufferHandle uboHandle{ renderable };
        EXPECT_EQ(SemanticUniformBufferHandle::Type::Model, uboHandle.getType());
        EXPECT_EQ(renderable, uboHandle.getRenderable());
        EXPECT_EQ("model(1)", fmt::to_string(uboHandle));
    }

    TEST_F(ASemanticUniformBufferHandle, canConstructHandleForCameraUBO)
    {
        constexpr CameraHandle camera{ 1u };
        constexpr SemanticUniformBufferHandle uboHandle{ camera };
        EXPECT_EQ(SemanticUniformBufferHandle::Type::Camera, uboHandle.getType());
        EXPECT_EQ(camera, uboHandle.getCamera());
        EXPECT_EQ("camera(1)", fmt::to_string(uboHandle));
    }

    TEST_F(ASemanticUniformBufferHandle, canConstructHandleForModelCameraUBO)
    {
        constexpr RenderableHandle renderable{ 1u };
        constexpr CameraHandle camera{ 2u };

        constexpr SemanticUniformBufferHandle uboHandle{ renderable, camera };
        EXPECT_EQ(SemanticUniformBufferHandle::Type::ModelCamera, uboHandle.getType());
        EXPECT_EQ(renderable, uboHandle.getRenderable());
        EXPECT_EQ(camera, uboHandle.getCamera());
        EXPECT_EQ("modelCamera(1:2)", fmt::to_string(uboHandle));
    }

    TEST_F(ASemanticUniformBufferHandle, UBOHandlesConstructedFromDifferentTypesButSameHandleValuesAreNotEqual)
    {
        for (uint32_t val : { 0u, 1u, InvalidMemoryHandle - 3 }) // test with few corner cases
        {
            const RenderableHandle renderable{ val };
            const CameraHandle camera{ val };

            const SemanticUniformBufferHandle uboHandleModel{ renderable };
            const SemanticUniformBufferHandle uboHandleCamera{ camera };
            const SemanticUniformBufferHandle uboHandleModelCamera{ renderable, camera };

            EXPECT_NE(uboHandleModel, uboHandleCamera);
            EXPECT_NE(uboHandleModel, uboHandleModelCamera);
            EXPECT_NE(uboHandleModelCamera, uboHandleCamera);

            // all considered unique
            std::vector<SemanticUniformBufferHandle> vec{ uboHandleModel, uboHandleCamera, uboHandleModelCamera };
            std::sort(vec.begin(), vec.end());
            EXPECT_EQ(vec.end(), std::unique(vec.begin(), vec.end()));

            // all having unique hash
            std::unordered_set<SemanticUniformBufferHandle> set{ uboHandleModel, uboHandleCamera, uboHandleModelCamera };
            EXPECT_EQ(3u, set.size());
        }
    }
}
