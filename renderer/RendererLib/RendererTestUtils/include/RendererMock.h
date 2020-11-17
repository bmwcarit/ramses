//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERMOCK_H
#define RAMSES_RENDERERMOCK_H

#include "renderer_common_gmock_header.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/FrameTimer.h"
#include "PlatformMock.h"
#include "DisplayControllerMock.h"
#include "RenderBackendMock.h"
#include "EmbeddedCompositingManagerMock.h"

namespace ramses_internal
{
    class DisplayConfig;
    class RendererScenes;
    class SceneExpirationMonitor;

template <template<typename> class MOCK_TYPE>
struct DisplayMockInfo
{
    MOCK_TYPE< DisplayControllerMock >*                         m_displayController;
    MOCK_TYPE< ramses_internal::RenderBackendMock<MOCK_TYPE> >* m_renderBackend;
    MOCK_TYPE< EmbeddedCompositingManagerMock >*                m_embeddedCompositingManager;
};

class RendererMock : public ramses_internal::Renderer
{
public:
    RendererMock(const ramses_internal::IPlatform& platform, const RendererScenes& rendererScenes,
        const RendererEventCollector& eventCollector, const SceneExpirationMonitor& expirationMonitor, const RendererStatistics& statistics);
    virtual ~RendererMock() override;

    MOCK_METHOD(void, markBufferWithSceneAsModified, (SceneId), (override));
    MOCK_METHOD(void, setClearColor, (DisplayHandle, DeviceResourceHandle, const Vector4&), (override));

    static const FrameTimer FrameTimerInstance;
};

template <template<typename> class MOCK_TYPE>
class RendererMockWithMockDisplay : public RendererMock
{
public:
    RendererMockWithMockDisplay(const ramses_internal::IPlatform& platform, const RendererScenes& rendererScenes,
        const RendererEventCollector& eventCollector, const SceneExpirationMonitor& expirationMonitor, const RendererStatistics& statistics);
    virtual ~RendererMockWithMockDisplay() override;

    virtual void createDisplayContext(const ramses_internal::DisplayConfig& displayConfig, ramses_internal::DisplayHandle displayHandle) override;
    virtual void destroyDisplayContext(ramses_internal::DisplayHandle handle) override;

    virtual void markBufferWithSceneAsModified(SceneId sceneId) override;
    virtual void setClearColor(DisplayHandle displayHandle, DeviceResourceHandle bufferDeviceHandle, const Vector4& clearColor) override;

    DisplayMockInfo<MOCK_TYPE>& getDisplayMock(ramses_internal::DisplayHandle handle);

    using ramses_internal::Renderer::getDisplaySetup;

private:
    HashMap< DisplayHandle, DisplayMockInfo<MOCK_TYPE> > m_displayControllers;
};

using RendererMockWithNiceMockDisplay = RendererMockWithMockDisplay< ::testing::NiceMock>;
using RendererMockWithStrictMockDisplay = RendererMockWithMockDisplay< ::testing::StrictMock>;

using DisplayNiceMockInfo = DisplayMockInfo< ::testing::NiceMock>;
using DisplayStrictMockInfo = DisplayMockInfo< ::testing::StrictMock>;
}
#endif
