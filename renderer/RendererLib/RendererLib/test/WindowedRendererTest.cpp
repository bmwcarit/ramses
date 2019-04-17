//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererLib/WindowedRenderer.h"
#include "ResourceProviderMock.h"
#include "PlatformFactoryMock.h"
#include "RenderBackendMock.h"
#include "ResourceUploaderMock.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "ComponentMocks.h"
#include "WindowMock.h"
#include "Utils/Image.h"

namespace ramses_internal {

class AWindowedRenderer : public ::testing::Test
{
public:
    AWindowedRenderer()
        : m_renderer(m_commandBuffer, m_sceneGraphConsumerComponent, m_platformFactoryMock, m_rendererStatistics)
    {
        ON_CALL(m_platformFactoryMock.renderBackendMock.surfaceMock, canRenderNewFrame()).WillByDefault(Return(true));
    }

    void update()
    {
        m_renderer.update();
        m_renderer.finishFrameStatistics(std::chrono::microseconds{ 0u });
    }

    void updateAndRender()
    {
        m_renderer.update();
        m_renderer.render();
        m_renderer.finishFrameStatistics(std::chrono::microseconds{ 0u });
    }

    void createDisplay(const DisplayHandle& displayHandle)
    {
        const DisplayConfig dummyConfig;

        m_commandBuffer.createDisplay(dummyConfig, m_resourceProvider, m_resourceUploader, displayHandle);
        EXPECT_CALL(m_platformFactoryMock, createRenderBackend(_, _));
        update();

        RendererEventVector events;
        m_renderer.dispatchRendererEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(displayHandle, events[0].displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayCreated, events[0].eventType);
    }

    void destroyDisplay(const DisplayHandle& displayHandle)
    {
        m_commandBuffer.destroyDisplay(displayHandle);
        EXPECT_CALL(m_platformFactoryMock, destroyRenderBackend(_));
        update();

        RendererEventVector events;
        m_renderer.dispatchRendererEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(displayHandle, events.front().displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayDestroyed, events.front().eventType);
    }

protected:
    PlatformFactoryNiceMock m_platformFactoryMock;
    StrictMock<SceneGraphConsumerComponentMock> m_sceneGraphConsumerComponent;
    NiceMock<ResourceProviderMock> m_resourceProvider;
    NiceMock<ResourceUploaderMock> m_resourceUploader;
    RendererCommandBuffer m_commandBuffer;
    RendererStatistics m_rendererStatistics;
    WindowedRenderer m_renderer;
};

class AWindowedRendererWithDisplay : public AWindowedRenderer
{
public:
    AWindowedRendererWithDisplay()
        : displayHandle(1u)
    {
    }

    virtual void SetUp()
    {
        createDisplay(displayHandle);
    }

    virtual void TearDown()
    {
        destroyDisplay(displayHandle);
    }

protected:
    const DisplayHandle displayHandle;
};

TEST_F(AWindowedRenderer, doesNotCrashWhenCreatedAndDestroyed)
{
}

TEST_F(AWindowedRendererWithDisplay, doesNotCrashWhenCreatedAndDestroyed)
{
}

TEST_F(AWindowedRenderer, canCreateAndDestroyDisplayInSingleLoop)
{
    const DisplayHandle displayHandle(1u);
    const DisplayConfig dummyConfig;
    NiceMock<ResourceProviderMock> resourceProvider;
    NiceMock<ResourceUploaderMock> resourceUploader;

    RendererCommandBuffer& rendererCommandBuffer = m_renderer.getRendererCommandBuffer();
    rendererCommandBuffer.createDisplay(dummyConfig, resourceProvider, resourceUploader, displayHandle);
    rendererCommandBuffer.destroyDisplay(displayHandle);

    EXPECT_CALL(m_platformFactoryMock, createRenderBackend(_, _));
    EXPECT_CALL(m_platformFactoryMock, destroyRenderBackend(_));
    update();

    RendererEventVector events;
    m_renderer.dispatchRendererEvents(events);
    ASSERT_EQ(2u, events.size());
    EXPECT_EQ(displayHandle, events[0].displayHandle);
    EXPECT_EQ(ERendererEventType_DisplayCreated, events[0].eventType);
    EXPECT_EQ(displayHandle, events[1].displayHandle);
    EXPECT_EQ(ERendererEventType_DisplayDestroyed, events[1].eventType);
}

TEST_F(AWindowedRendererWithDisplay, takeScreenshotGeneratesSuccessEvent)
{
    m_commandBuffer.readPixels(displayHandle, "", false, 0u, 0u, WindowMock::FakeWidth, WindowMock::FakeHeight);
    updateAndRender();

    RendererEventVector events;
    m_renderer.dispatchRendererEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(displayHandle, events[0].displayHandle);
    EXPECT_EQ(WindowMock::FakeWidth * WindowMock::FakeHeight * 4u, events[0].pixelData.size());
    EXPECT_EQ(ERendererEventType_ReadPixelsFromFramebuffer, events[0].eventType);
}

TEST_F(AWindowedRendererWithDisplay, takeScreenshotGeneratesFailEvent)
{
    // take screenshot bigger than display!
    m_commandBuffer.readPixels(displayHandle, "", false, 0u, 0u, WindowMock::FakeWidth, WindowMock::FakeHeight + 1u);
    updateAndRender();

    RendererEventVector events;
    m_renderer.dispatchRendererEvents(events);
    ASSERT_EQ(1u, events.size());
    EXPECT_EQ(displayHandle, events[0].displayHandle);
    EXPECT_EQ(0u, events[0].pixelData.size());
    EXPECT_EQ(ERendererEventType_ReadPixelsFromFramebufferFailed, events[0].eventType);
}

TEST_F(AWindowedRendererWithDisplay, saveScreenshotToFileWithFullscreenDoesNotGenerateEvent)
{
    const String filename = "testScreenshot.png";

    m_commandBuffer.readPixels(displayHandle, filename, true, 0u, 0u, 1u, 1u);
    updateAndRender();

    // expect file has been written
    ramses_capu::File screenshotFile(filename.stdRef());
    EXPECT_TRUE(screenshotFile.exists());

    // expect correct fullScreen size
    {
        Image bitmap;
        bitmap.loadFromFilePNG(filename);
        const UInt32 displayWidth = WindowMock::FakeWidth;
        const UInt32 displayHeight = WindowMock::FakeHeight;
        EXPECT_EQ(displayWidth, bitmap.getWidth());
        EXPECT_EQ(displayHeight, bitmap.getHeight());
    }
    EXPECT_EQ(ramses_capu::CAPU_OK, screenshotFile.remove());

    // Do not expect a renderer event
    RendererEventVector events;
    m_renderer.dispatchRendererEvents(events);
    EXPECT_EQ(0u, events.size());
}

TEST_F(AWindowedRendererWithDisplay, saveScreenshotToFileOnNotExistentDisplayDoesNotGenerateEvent)
{
    const String filename = "failedTestScreenshot.png";

    const DisplayHandle wrongDisplayHandle(23u);

    m_commandBuffer.readPixels(wrongDisplayHandle, filename, false, 0u, 0u, WindowMock::FakeWidth, WindowMock::FakeHeight);
    updateAndRender();

    // expect no file has been written
    ramses_capu::File screenshotFile(filename.stdRef());
    EXPECT_FALSE(screenshotFile.exists());

    RendererEventVector events;
    m_renderer.dispatchRendererEvents(events);
    EXPECT_EQ(0u, events.size());
}

TEST_F(AWindowedRendererWithDisplay, saveScreenshotToFileFailsAndDoesNotGenerateEvent)
{
    const String filename = "failedTestScreenshot.png";

    m_commandBuffer.readPixels(displayHandle, filename, false, 0u, 0u, WindowMock::FakeWidth, WindowMock::FakeHeight + 1u);
    updateAndRender();

    // expect no file has been written
    ramses_capu::File screenshotFile(filename.stdRef());
    EXPECT_FALSE(screenshotFile.exists());

    // Do not expect a renderer event
    RendererEventVector events;
    m_renderer.dispatchRendererEvents(events);
    EXPECT_EQ(0u, events.size());
}

TEST_F(AWindowedRenderer, updatesSystemCompositorControllerInUpdate)
{
    EXPECT_CALL(m_platformFactoryMock.systemCompositorControllerMock, update());
    update();
}
}
