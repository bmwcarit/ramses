//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IRendererTest.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include <drm_fourcc.h>
#include <gbm.h>
#include <unordered_map>

namespace ramses::internal
{
    class DmaOffscreenBufferTests : public IRendererTest
    {
    public:
        explicit DmaOffscreenBufferTests() = default;

        void setUpTestCases(RendererTestsFramework& testFramework) final;
        bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        enum
        {
            DmaOffscreenBufferTest_BufferRead,
            DmaOffscreenBufferTest_BufferWrite,
            DmaOffscreenBufferTest_BufferReadWrite,
            DmaOffscreenBufferTest_ReadContentWithAlphaBlending,

            DmaOffscreenBufferTest_Format_ARGB8888,
            DmaOffscreenBufferTest_Format_XRGB8888,
            DmaOffscreenBufferTest_Format_XBGR8888,
            DmaOffscreenBufferTest_Format_BGR888,

            DmaOffscreenBufferTest_UsageFlagRendering,
            DmaOffscreenBufferTest_UsageFlagRenderingAndScanOut,

            DmaOffscreenBufferTest_BufferSize_200x200,
            DmaOffscreenBufferTest_BufferSize_256x256,
            DmaOffscreenBufferTest_BufferSize_128x64,
            DmaOffscreenBufferTest_BufferSize_128x256,

            DmaOffscreenBufferTest_TwoBuffers_DoubleBufferedRead,
            DmaOffscreenBufferTest_TwoDisplays,
        };

        enum class EMappingBufferAccessRight
        {
            ReadOnly,
            WriteOnly,
            ReadWrite
        };

        struct TestBufferInfo
        {
            uint32_t displayIdx = 0u;
            ramses::displayBufferId_t displaBufferId;
            int fd = -1;
            uint32_t stride = 0u;
            void* mappedMemory = nullptr;
            EMappingBufferAccessRight accessRights = EMappingBufferAccessRight::ReadOnly;
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t fourccFormat = 0;
        };

        template <typename INTEGRATION_SCENE>
        ramses::sceneId_t createAndShowScene(RendererTestsFramework& testFramework, uint32_t displayIdx, uint32_t sceneState, const glm::vec3& camPos, uint32_t vpWidth = ramses::internal::IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = ramses::internal::IntegrationScene::DefaultViewportHeight);
        template <typename INTEGRATION_SCENE>
        bool renderSceneToBufferAndExpectScreenshot(RendererTestsFramework& testFramework, const TestBufferInfo& testBuffer, uint32_t sceneState, const std::string& screenshotImage);

        static ramses::internal::Image ReadMappedMemoryContentToImage(const TestBufferInfo& bufferInfo);
        static bool WriteTestContentToMappedMemory(const TestBufferInfo& bufferInfo);

        static bool CreateAndMapDmaOffscreenBuffer(RendererTestsFramework& testFramework, uint32_t displayIdx, uint32_t bufferWidth, uint32_t bufferHeight, uint32_t fourccFormat, uint32_t bufferUsageFlags, EMappingBufferAccessRight accessRights, TestBufferInfo& bufferInfo, bool disableClearing = true);
        static bool StartSyncBuffer(const TestBufferInfo& bufferInfo);
        static bool EndSyncBuffer(const TestBufferInfo& bufferInfo);

        bool runBufferWriteTest(RendererTestsFramework& testFramework);
        bool runBufferReadTest(RendererTestsFramework& testFramework);
        bool runContentWithAlphaBlendingTest(RendererTestsFramework& testFramework);
        bool runBufferReadWriteTest(RendererTestsFramework& testFramework,
                                    uint32_t displayIdx = 0u,
                                    uint32_t bufferWidth = DefaultBufferWidth,
                                    uint32_t bufferHeight = DefaultBufferHeight,
                                    uint32_t fourccFormat = DefaultFourccBufferFormat,
                                    uint32_t bufferUsageFlags = DefaultUsageFlags,
                                    const std::string& screenshotImage = "DmaOffscreenBufferTest_BufferReadWrite");
        bool runDoubleBufferedReadTest(RendererTestsFramework& testFramework);
        bool runTwoDisplaysTest(RendererTestsFramework& testFramework);

        static constexpr uint32_t DefaultBufferWidth = 200u;
        static constexpr uint32_t DefaultBufferHeight = 200u;
        static constexpr uint32_t DefaultUsageFlags = GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT;
        static constexpr uint32_t DefaultFourccBufferFormat = DRM_FORMAT_ARGB8888;

        const glm::vec3 m_cameraTranslation{ 0.f, 0.f, 8.f };
        TestBufferInfo testBuffer1;
        TestBufferInfo testBuffer2;
    };
}
