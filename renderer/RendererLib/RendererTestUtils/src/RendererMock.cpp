//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererMock.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/RendererScenes.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "RendererLib/RendererStatistics.h"

namespace ramses_internal {
using namespace testing;

const FrameTimer RendererMock::FrameTimerInstance;

RendererMock::RendererMock(DisplayHandle display, const IPlatform& platform, const RendererScenes& rendererScenes,
    const RendererEventCollector& eventCollector, const SceneExpirationMonitor& expirationMonitor, const RendererStatistics& statistics)
    : Renderer(display, const_cast<IPlatform&>(platform), const_cast<RendererScenes&>(rendererScenes),
        const_cast<RendererEventCollector&>(eventCollector), FrameTimerInstance, const_cast<SceneExpirationMonitor&>(expirationMonitor), const_cast<RendererStatistics&>(statistics))
{
    // by default do not track modified scenes, concrete tests will override
    EXPECT_CALL(*this, markBufferWithSceneForRerender(_)).Times(AnyNumber());
    // by default do not track set clear color, concrete tests will override
    EXPECT_CALL(*this, setClearColor(_, _)).Times(AnyNumber());
}

RendererMock::~RendererMock() = default;

template <template<typename> class MOCK_TYPE>
RendererMockWithMockDisplay<MOCK_TYPE>::RendererMockWithMockDisplay(DisplayHandle display, const RendererScenes& rendererScenes,
    const RendererEventCollector& eventCollector, const SceneExpirationMonitor& expirationMonitor, const RendererStatistics& statistics)
    : RendererMock(display, m_platform, const_cast<RendererScenes&>(rendererScenes),
        const_cast<RendererEventCollector&>(eventCollector), const_cast<SceneExpirationMonitor&>(expirationMonitor), const_cast<RendererStatistics&>(statistics))
{
}

template <template<typename> class MOCK_TYPE>
RendererMockWithMockDisplay<MOCK_TYPE>::~RendererMockWithMockDisplay() = default;

template <template<typename> class MOCK_TYPE>
IDisplayController* RendererMockWithMockDisplay<MOCK_TYPE>::createDisplayControllerFromConfig(const DisplayConfig& displayConfig)
{
    assert(m_displayController == nullptr);
    m_displayController = new MOCK_TYPE < DisplayControllerMock >;

    ON_CALL(*m_displayController, getRenderBackend()).WillByDefault(ReturnRef(m_platform.renderBackendMock));
    ON_CALL(*m_displayController, getEmbeddedCompositingManager()).WillByDefault(ReturnRef(m_embeddedCompositingManager));

    EXPECT_CALL(*m_displayController, getDisplayWidth()).Times(AnyNumber()).WillRepeatedly(Return(displayConfig.getDesiredWindowWidth()));
    EXPECT_CALL(*m_displayController, getDisplayHeight()).Times(AnyNumber()).WillRepeatedly(Return(displayConfig.getDesiredWindowHeight()));
    EXPECT_CALL(*m_displayController, getDisplayBuffer()).Times(AnyNumber());
    EXPECT_CALL(*m_displayController, getRenderBackend()).Times(AnyNumber());
    EXPECT_CALL(*m_displayController, getEmbeddedCompositingManager()).Times(AnyNumber());

    EXPECT_CALL(m_platform.renderBackendMock.contextMock, disable()).Times(AtMost(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(m_platform.renderBackendMock.contextMock, enable()).Times(AtMost(1)).WillRepeatedly(Return(true));

    EXPECT_CALL(*this, setClearColor(_, displayConfig.getClearColor()));

    return m_displayController;
}

template <template<typename> class MOCK_TYPE>
void RendererMockWithMockDisplay<MOCK_TYPE>::markBufferWithSceneForRerender(SceneId sceneId)
{
    Renderer::markBufferWithSceneForRerender(sceneId);  // NOLINT clang-tidy: We really mean to call into Renderer
    RendererMock::markBufferWithSceneForRerender(sceneId);
}

template <template<typename> class MOCK_TYPE>
void ramses_internal::RendererMockWithMockDisplay<MOCK_TYPE>::setClearFlags(DeviceResourceHandle bufferDeviceHandle, uint32_t clearFlags)
{
    Renderer::setClearFlags(bufferDeviceHandle, clearFlags);  // NOLINT clang-tidy: We really mean to call into Renderer
    RendererMock::setClearFlags(bufferDeviceHandle, clearFlags);
}

template <template<typename> class MOCK_TYPE>
void ramses_internal::RendererMockWithMockDisplay<MOCK_TYPE>::setClearColor(DeviceResourceHandle bufferDeviceHandle, const Vector4& clearColor)
{
    Renderer::setClearColor(bufferDeviceHandle, clearColor);  // NOLINT clang-tidy: We really mean to call into Renderer
    RendererMock::setClearColor(bufferDeviceHandle, clearColor);
}

template class RendererMockWithMockDisplay < NiceMock >;
template class RendererMockWithMockDisplay < StrictMock >;
}
