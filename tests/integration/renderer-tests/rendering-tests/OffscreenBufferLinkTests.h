//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IRendererTest.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

#include <string>

namespace ramses::internal
{
    class OffscreenBufferLinkTests : public IRendererTest
    {
    public:
        explicit OffscreenBufferLinkTests(bool useInterruptibleBuffers);

        void setUpTestCases(RendererTestsFramework& testFramework) final;
        bool run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase) final;

    private:
        template <typename INTEGRATION_SCENE>
        ramses::sceneId_t createAndShowScene(RendererTestsFramework& testFramework, uint32_t sceneState, const glm::vec3& camPos, uint32_t vpWidth = ramses::internal::IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = ramses::internal::IntegrationScene::DefaultViewportHeight);
        bool renderAndCompareScreenshot(RendererTestsFramework& testFramework, const std::string& expectedImageName, uint32_t testDisplayIdx = 0u, float expectedPixelError = RendererTestUtils::DefaultMaxAveragePercentPerPixel);

        enum
        {
            OffscreenBufferLinkTest_ConsumerLinkedToEmptyBuffer,
            OffscreenBufferLinkTest_ConsumersLinkedToBufferWithOneScene,
            OffscreenBufferLinkTest_OneOfTwoLinksRemoved,
            OffscreenBufferLinkTest_ConsumerLinkedToBufferWithTwoScenes,
            OffscreenBufferLinkTest_ConsumerRelinkedToAnotherBuffer,
            OffscreenBufferLinkTest_ProviderBufferDestroyed,
            OffscreenBufferLinkTest_SourceSceneHidden,
            OffscreenBufferLinkTest_SourceSceneAssignedToFBWhileShown,
            OffscreenBufferLinkTest_OneOfTwoSourceScenesUnmapped,
            OffscreenBufferLinkTest_ProviderSceneUsesDepthTest,
            OffscreenBufferLinkTest_ProviderSceneUsesStencilTest,
            OffscreenBufferLinkTest_SetClearColor,
            OffscreenBufferLinkTest_ConsumerLinkedToMSAABuffer,
            OffscreenBufferLinkTest_ConsumerUnlinkedMSAABuffer
        };

        ramses::sceneId_t m_sceneIdProvider;
        ramses::sceneId_t m_sceneIdProvider2;
        ramses::sceneId_t m_sceneIdConsumer;
        ramses::sceneId_t m_sceneIdConsumer2;

        const glm::vec3 m_cameraLow{ -1.f, 2.f, 8.f };
        const glm::vec3 m_cameraHigh{ 1.f, -2.f, 8.f };
        const glm::vec3 m_cameraMid{ 0.f, 0.f, 8.f };

        const bool m_interruptibleBuffers;

        static constexpr uint32_t OBWidth = 256u;
        static constexpr uint32_t OBHeight = 256u;
    };
}
