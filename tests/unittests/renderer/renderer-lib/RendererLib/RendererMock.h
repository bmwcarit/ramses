//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Renderer.h"
#include "internal/RendererLib/FrameTimer.h"
#include "PlatformMock.h"
#include "DisplayControllerMock.h"
#include "RenderBackendMock.h"
#include "EmbeddedCompositingManagerMock.h"

namespace ramses::internal
{
    class DisplayConfigData;
    class RendererScenes;
    class SceneExpirationMonitor;

class RendererMock : public Renderer
{
public:
    RendererMock(DisplayHandle display, const IPlatform& platform, const RendererScenes& rendererScenes,
        const RendererEventCollector& eventCollector, const SceneExpirationMonitor& expirationMonitor, const RendererStatistics& statistics);
    ~RendererMock() override;

    MOCK_METHOD(void, markBufferWithSceneForRerender, (SceneId), (override));
    MOCK_METHOD(void, setClearFlags, (DeviceResourceHandle, ClearFlags), (override));
    MOCK_METHOD(void, setClearColor, (DeviceResourceHandle, const glm::vec4&), (override));

    static const FrameTimer FrameTimerInstance;
};

template <template<typename> class MOCK_TYPE>
class RendererMockWithMockDisplay : public RendererMock
{
public:
    RendererMockWithMockDisplay(DisplayHandle display, const RendererScenes& rendererScenes,
        const RendererEventCollector& eventCollector, const SceneExpirationMonitor& expirationMonitor, const RendererStatistics& statistics);
    ~RendererMockWithMockDisplay() override;

    IDisplayController* createDisplayControllerFromConfig(const ramses::internal::DisplayConfigData& displayConfig) override;

    void markBufferWithSceneForRerender(SceneId sceneId) override;
    void setClearFlags(DeviceResourceHandle bufferDeviceHandle, ClearFlags clearFlags) override;
    void setClearColor(DeviceResourceHandle bufferDeviceHandle, const glm::vec4& clearColor) override;

    MOCK_TYPE< PlatformMock<MOCK_TYPE> > m_platform;
    MOCK_TYPE< DisplayControllerMock >* m_displayController = nullptr;
    MOCK_TYPE< EmbeddedCompositingManagerMock > m_embeddedCompositingManager;
};

using RendererMockWithNiceMockDisplay = RendererMockWithMockDisplay< ::testing::NiceMock>;
using RendererMockWithStrictMockDisplay = RendererMockWithMockDisplay< ::testing::StrictMock>;
}

