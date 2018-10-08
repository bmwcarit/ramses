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

namespace ramses_internal {
using namespace testing;

template <template<typename> class MOCK_TYPE>
RendererMockWithMockDisplay<MOCK_TYPE>::RendererMockWithMockDisplay(const IPlatformFactory& platformFactory, const RendererScenes& rendererScenes,
    const RendererEventCollector& eventCollector, const LatencyMonitor& latencyMonitor)
    : Renderer(const_cast<IPlatformFactory&>(platformFactory), const_cast<RendererScenes&>(rendererScenes),
        const_cast<RendererEventCollector&>(eventCollector), FrameTimerInstance, const_cast<LatencyMonitor&>(latencyMonitor))
{
}

template <template<typename> class MOCK_TYPE>
void RendererMockWithMockDisplay<MOCK_TYPE>::createDisplayContext(const DisplayConfig& displayConfig, DisplayHandle displayHandle)
{
    UNUSED(displayConfig);
    MOCK_TYPE< RenderBackendMock<MOCK_TYPE> >* renderBackend = new MOCK_TYPE < RenderBackendMock<MOCK_TYPE> > ;
    MOCK_TYPE< EmbeddedCompositingManagerMock >* embeddedCompositingManager = new MOCK_TYPE< EmbeddedCompositingManagerMock >;
    MOCK_TYPE< DisplayControllerMock >* displayController = new MOCK_TYPE < DisplayControllerMock >;
    EXPECT_CALL(*displayController, getDisplayWidth()).Times(AnyNumber());
    EXPECT_CALL(*displayController, getDisplayHeight()).Times(AnyNumber());
    EXPECT_CALL(*displayController, getDisplayBuffer()).Times(AnyNumber());
    EXPECT_CALL(*displayController, getRenderBackend()).Times(AnyNumber());
    EXPECT_CALL(*displayController, getEmbeddedCompositingManager()).Times(AnyNumber());
    ON_CALL(*displayController, getRenderBackend()).WillByDefault(ReturnRef(*renderBackend));
    ON_CALL(*displayController, getEmbeddedCompositingManager()).WillByDefault(ReturnRef(*embeddedCompositingManager));

    Renderer::addDisplayController(*displayController, displayHandle);
    EXPECT_TRUE(hasDisplayController(displayHandle));

    DisplayMockInfo<MOCK_TYPE> displayMock;
    displayMock.m_displayController = displayController;
    displayMock.m_renderBackend = renderBackend;
    displayMock.m_embeddedCompositingManager = embeddedCompositingManager;
    m_displayControllers.put(displayHandle, displayMock);
}

template <template<typename> class MOCK_TYPE>
void RendererMockWithMockDisplay<MOCK_TYPE>::destroyDisplayContext(DisplayHandle handle)
{
    DisplayMockInfo<MOCK_TYPE>& displayMock = *m_displayControllers.get(handle);
    MOCK_TYPE<DisplayControllerMock>& displayController = *displayMock.m_displayController;

    EXPECT_CALL(displayController, validateRenderingStatusHealthy());
    removeDisplayController(handle);
    delete &displayController;
    delete displayMock.m_renderBackend;
    delete displayMock.m_embeddedCompositingManager;
}

template <template<typename> class MOCK_TYPE>
DisplayMockInfo<MOCK_TYPE>& RendererMockWithMockDisplay<MOCK_TYPE>::getDisplayMock(DisplayHandle handle)
{
    return *m_displayControllers.get(handle);
}

template struct DisplayMockInfo < NiceMock >;
template struct DisplayMockInfo < StrictMock >;

template class RendererMockWithMockDisplay < NiceMock >;
template class RendererMockWithMockDisplay < StrictMock >;
}
