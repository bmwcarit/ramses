//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DmaOffscreenBufferTests.h"
#include "TestScenes/TextureLinkScene.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/LogMacros.h"
#include <linux/dma-buf.h>
#include <linux/ioctl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace ramses_internal;

void DmaOffscreenBufferTests::setUpTestCases(RendererTestsFramework& testFramework)
{
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_BufferRead, *this, "DmaOffscreenBufferTest_BufferRead");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_BufferWrite, *this, "DmaOffscreenBufferTest_BufferWrite");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_BufferReadWrite, *this, "DmaOffscreenBufferTest_BufferReadWrite");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_ReadContentWithAlphaBlending, *this, "DmaOffscreenBufferTest_ContentWithAlphaBlending");

    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_Format_XBGR8888, *this, "DmaOffscreenBufferTest_Format_XBGR8888");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_Format_XRGB8888, *this, "DmaOffscreenBufferTest_Format_XRGB8888");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_Format_ARGB8888, *this, "DmaOffscreenBufferTest_Format_ARGB8888");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_Format_BGR888, *this, "DmaOffscreenBufferTest_Format_BGR888");

    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_UsageFlagRendering, *this, "DmaOffscreenBufferTest_UsageFlagRendering");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_UsageFlagRenderingAndScanOut, *this, "DmaOffscreenBufferTest_UsageFlagRenderingAndScanOut");

    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_BufferSize_200x200, *this, "DmaOffscreenBufferTest_BufferSize_200x200");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_BufferSize_256x256, *this, "DmaOffscreenBufferTest_BufferSize_256x256");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_BufferSize_128x64, *this, "DmaOffscreenBufferTest_BufferSize_128x64");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_BufferSize_128x256, *this, "DmaOffscreenBufferTest_BufferSize_128x256");

    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_TwoBuffers_DoubleBufferedRead, *this, "DmaOffscreenBufferTest_TwoBuffers_DoubleBufferedRead");
    testFramework.createTestCaseWithoutRenderer(DmaOffscreenBufferTest_TwoDisplays, *this, "DmaOffscreenBufferTest_TwoDisplays");
}

bool DmaOffscreenBufferTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
{
    //Set big frame callback poll time, to make sure no frames are skipped due to late frame callbacks
    auto rendererConfig = RendererTestUtils::CreateTestRendererConfig();
    rendererConfig.setFrameCallbackMaxPollTime(1000000u);
    testFramework.initializeRenderer(rendererConfig);

    ramses::DisplayConfig dmaDisplayConfig = RendererTestUtils::CreateTestDisplayConfig(1u);
    dmaDisplayConfig.setWindowRectangle(0, 0, ramses_internal::IntegrationScene::DefaultViewportWidth, ramses_internal::IntegrationScene::DefaultViewportHeight);
    dmaDisplayConfig.setPlatformRenderNode("/dev/dri/renderD128");
    testFramework.createDisplay(dmaDisplayConfig);

    if(testCase.m_id == DmaOffscreenBufferTest_TwoDisplays)
    {
        ramses::DisplayConfig dmaDisplayConfig2 = RendererTestUtils::CreateTestDisplayConfig(2u);
        dmaDisplayConfig2.setWindowRectangle(ramses_internal::IntegrationScene::DefaultViewportWidth, 0, ramses_internal::IntegrationScene::DefaultViewportWidth, ramses_internal::IntegrationScene::DefaultViewportHeight);
        dmaDisplayConfig2.setPlatformRenderNode("/dev/dri/renderD128");
        testFramework.createDisplay(dmaDisplayConfig2);
    }

    switch (testCase.m_id)
    {
    case DmaOffscreenBufferTest_BufferRead:
        return runBufferReadTest(testFramework);
    case DmaOffscreenBufferTest_BufferWrite:
        return runBufferWriteTest(testFramework);
    case DmaOffscreenBufferTest_BufferReadWrite:
        return runBufferReadWriteTest(testFramework);
    case DmaOffscreenBufferTest_ReadContentWithAlphaBlending:
        return runContentWithAlphaBlendingTest(testFramework);

    case DmaOffscreenBufferTest_Format_ARGB8888:
        return runBufferReadWriteTest(testFramework,0u, DefaultBufferWidth, DefaultBufferHeight, DRM_FORMAT_ARGB8888);
    case DmaOffscreenBufferTest_Format_XRGB8888:
        return runBufferReadWriteTest(testFramework,0u, DefaultBufferWidth, DefaultBufferHeight, DRM_FORMAT_XRGB8888);
    case DmaOffscreenBufferTest_Format_XBGR8888:
        return runBufferReadWriteTest(testFramework,0u, DefaultBufferWidth, DefaultBufferHeight, DRM_FORMAT_XBGR8888);
    case DmaOffscreenBufferTest_Format_BGR888:
        return runBufferReadWriteTest(testFramework,0u, DefaultBufferWidth, DefaultBufferHeight, DRM_FORMAT_BGR888);

    case DmaOffscreenBufferTest_UsageFlagRendering:
        return runBufferReadWriteTest(testFramework,0u, DefaultBufferWidth, DefaultBufferHeight, DefaultFourccBufferFormat, GBM_BO_USE_RENDERING);
    case DmaOffscreenBufferTest_UsageFlagRenderingAndScanOut:
        return runBufferReadWriteTest(testFramework,0u, DefaultBufferWidth, DefaultBufferHeight, DefaultFourccBufferFormat, GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT);

    case DmaOffscreenBufferTest_BufferSize_200x200:
        return runBufferReadWriteTest(testFramework,0u, 200u, 200u, DefaultFourccBufferFormat, DefaultUsageFlags, "DmaOffscreenBufferTest_BufferReadWrite");
    case DmaOffscreenBufferTest_BufferSize_256x256:
        return runBufferReadWriteTest(testFramework,0u, 256u, 256u, DefaultFourccBufferFormat, DefaultUsageFlags, "DmaOffscreenBufferTest_BufferWrite_256x256");
    case DmaOffscreenBufferTest_BufferSize_128x64:
        return runBufferReadWriteTest(testFramework,0u, 128u, 64u, DefaultFourccBufferFormat, DefaultUsageFlags, "DmaOffscreenBufferTest_BufferWrite_128x64");
    case DmaOffscreenBufferTest_BufferSize_128x256:
        return runBufferReadWriteTest(testFramework,0u, 128u, 256u, DefaultFourccBufferFormat, DefaultUsageFlags, "DmaOffscreenBufferTest_BufferWrite_128x256");

    case DmaOffscreenBufferTest_TwoBuffers_DoubleBufferedRead:
        return runDoubleBufferedReadTest(testFramework);
    case DmaOffscreenBufferTest_TwoDisplays:
        return runBufferReadWriteTest(testFramework,0u, DefaultBufferWidth, DefaultBufferHeight, DefaultFourccBufferFormat, DefaultUsageFlags)
                && runBufferReadWriteTest(testFramework,1u, DefaultBufferWidth, DefaultBufferHeight, DefaultFourccBufferFormat, DefaultUsageFlags);
    default:
        assert(false && "undefined test case");
    }

    return false;
}

template <typename INTEGRATION_SCENE>
ramses::sceneId_t DmaOffscreenBufferTests::createAndShowScene(RendererTestsFramework& testFramework, uint32_t displayIdx, uint32_t sceneState, const Vector3& camPos, uint32_t vpWidth, uint32_t vpHeight)
{
    const auto sceneId = testFramework.getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState, camPos, vpWidth, vpHeight);
    testFramework.publishAndFlushScene(sceneId);
    testFramework.getSceneToRendered(sceneId, displayIdx);

    return sceneId;
}

template <typename INTEGRATION_SCENE>
bool DmaOffscreenBufferTests::renderSceneToBufferAndExpectScreenshot(RendererTestsFramework& testFramework, const TestBufferInfo& testBuffer, uint32_t sceneState, const std::string& screenshotImage)
{
    //render a scene into OB, then read mapped memory for that OB and compare it with screenshot image
    const auto sceneId = createAndShowScene<INTEGRATION_SCENE>(testFramework, testBuffer.displayIdx, sceneState, Vector3{0.f}, testBuffer.width, testBuffer.height);
    testFramework.assignSceneToDisplayBuffer(sceneId, testBuffer.displaBufferId);

    //make two loops to make sure swap buffers was called twice after scene was assigned to OB
    testFramework.flushRendererAndDoOneLoop();
    testFramework.flushRendererAndDoOneLoop();

    //check content rendered into OB from two loops ago
    const Image obMemImageResult = readMappedMemoryContentToImage(testBuffer);
    return RendererTestUtils::CompareBitmapToImageInFile(obMemImageResult, screenshotImage.c_str(), RendererTestUtils::DefaultMaxAveragePercentPerPixel, true);
}

bool DmaOffscreenBufferTests::startSyncBuffer(const TestBufferInfo& bufferInfo)
{
    dma_buf_sync syncStartFlags = { 0 };
    switch (bufferInfo.accessRights)
    {
    case EMappingBufferAccessRight::ReadOnly:
        syncStartFlags.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ;
        break;
    case EMappingBufferAccessRight::WriteOnly:
        syncStartFlags.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
        break;
    case EMappingBufferAccessRight::ReadWrite:
        syncStartFlags.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
        break;
    }
    assert(syncStartFlags.flags != 0);

    const auto syncStartResult = ioctl(bufferInfo.fd, DMA_BUF_IOCTL_SYNC, &syncStartFlags);
    if(syncStartResult != 0)
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Failed to start sync!");
        return false;
    }

    return true;
}

bool DmaOffscreenBufferTests::endSyncBuffer(const TestBufferInfo& bufferInfo)
{
    dma_buf_sync syncEndFlags = { 0 };
    switch (bufferInfo.accessRights)
    {
    case EMappingBufferAccessRight::ReadOnly:
        syncEndFlags.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
        break;
    case EMappingBufferAccessRight::WriteOnly:
        syncEndFlags.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_WRITE;
        break;
    case EMappingBufferAccessRight::ReadWrite:
        syncEndFlags.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
        break;
    }
    assert(syncEndFlags.flags != 0);

    const auto syncEndResult = ioctl(bufferInfo.fd, DMA_BUF_IOCTL_SYNC, &syncEndFlags);
    if(syncEndResult != 0)
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Failed to end sync!");
        return false;
    }

    return true;
}

Image DmaOffscreenBufferTests::readMappedMemoryContentToImage(const TestBufferInfo& bufferInfo)
{
    if(!startSyncBuffer(bufferInfo))
        return {};

    std::vector<uint8_t> mappedMemImageData;
    mappedMemImageData.reserve(bufferInfo.height * bufferInfo.width * 4u);

    auto readPixelARGB8888 = [&bufferInfo, &mappedMemImageData](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * (bufferInfo.height - row - 1) + 4u * col;
        const uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //R=p[2], G=p[1], B=p[0], A=p[3]
        mappedMemImageData.push_back(pixelData[2]);
        mappedMemImageData.push_back(pixelData[1]);
        mappedMemImageData.push_back(pixelData[0]);
        mappedMemImageData.push_back(pixelData[3]);
    };

    auto readPixelXRGB8888 = [&bufferInfo, &mappedMemImageData](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * (bufferInfo.height - row - 1) + 4u * col;
        const uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //R=p[2], G=p[1], B=p[0], A=255
        mappedMemImageData.push_back(pixelData[2]);
        mappedMemImageData.push_back(pixelData[1]);
        mappedMemImageData.push_back(pixelData[0]);
        mappedMemImageData.push_back(255u);
    };

    auto readPixelXBGR8888 = [&bufferInfo, &mappedMemImageData](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * (bufferInfo.height - row - 1) + 4u * col;
        const uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //R=p[0], G=p[1], B=p[2], A=255
        mappedMemImageData.push_back(pixelData[0]);
        mappedMemImageData.push_back(pixelData[1]);
        mappedMemImageData.push_back(pixelData[2]);
        mappedMemImageData.push_back(255u);
    };

    auto readPixelBGR888 = [&bufferInfo, &mappedMemImageData](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * (bufferInfo.height - row - 1) + 3u * col;
        const uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //R=p[0], G=p[1], B=p[2], A=255
        mappedMemImageData.push_back(pixelData[0]);
        mappedMemImageData.push_back(pixelData[1]);
        mappedMemImageData.push_back(pixelData[2]);
        mappedMemImageData.push_back(255u);
    };

    switch(bufferInfo.fourccFormat)
    {
    case DRM_FORMAT_ARGB8888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                readPixelARGB8888(row, col);
        break;
    case DRM_FORMAT_XRGB8888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                readPixelXRGB8888(row, col);
        break;
    case DRM_FORMAT_XBGR8888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                readPixelXBGR8888(row, col);
        break;
    case DRM_FORMAT_BGR888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                readPixelBGR888(row, col);
        break;
    default:
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Trying to read content from unknown buffer format!");
        assert(false);
        return {};
    }

    if(!endSyncBuffer(bufferInfo))
        return {};

    return Image(bufferInfo.width, bufferInfo.height, std::move(mappedMemImageData));
}

bool DmaOffscreenBufferTests::writeTestContentToMappedMemory(const DmaOffscreenBufferTests::TestBufferInfo& bufferInfo)
{
    if(!startSyncBuffer(bufferInfo))
        return false;

    auto calculatePixelMagicValue = [&bufferInfo](uint32_t row, uint32_t col)
    {
        const uint8_t r = static_cast<uint8_t>((1.f * col / bufferInfo.width) * 255);
        const uint8_t g = static_cast<uint8_t>((1.f * row / bufferInfo.height) * 255);
        const uint8_t b = 128u;
        const uint8_t a = 255u;

        return std::array<uint8_t, 4>{r, g, b, a};
    };

    auto writePixelARGB8888 = [&bufferInfo, &calculatePixelMagicValue](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * row + 4u * col;
        uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //p[0]=B, p[1]=G, p[2]=R, p[3]=A
        const auto val = calculatePixelMagicValue(row, col);
        pixelData[0] = val[2];
        pixelData[1] = val[1];
        pixelData[2] = val[0];
        pixelData[3] = val[3];
    };

    auto writePixelXRGB8888 = [&bufferInfo, &calculatePixelMagicValue](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * row + 4u * col;
        uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //p[0]=B, p[1]=G, p[2]=R, p[3]=X
        const auto val = calculatePixelMagicValue(row, col);
        pixelData[0] = val[2];
        pixelData[1] = val[1];
        pixelData[2] = val[0];
        pixelData[3] = 0u;
    };

    auto writePixelXBGR8888 = [&bufferInfo, &calculatePixelMagicValue](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * row + 4u * col;
        uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //p[0]=R, p[1]=G, p[2]=B, p[3]=X
        const auto val = calculatePixelMagicValue(row, col);
        pixelData[0] = val[0];
        pixelData[1] = val[1];
        pixelData[2] = val[2];
        pixelData[3] = 0u;
    };

    auto writePixelBGR8888 = [&bufferInfo, &calculatePixelMagicValue](uint32_t row, uint32_t col)
    {
        const std::size_t pixelDataOffset = bufferInfo.stride * row + 3u * col;
        uint8_t* pixelData = reinterpret_cast<uint8_t*>(bufferInfo.mappedMemory) + pixelDataOffset;

        //p[0]=R, p[1]=G, p[2]=B
        const auto val = calculatePixelMagicValue(row, col);
        pixelData[0] = val[0];
        pixelData[1] = val[1];
        pixelData[2] = val[2];
    };

    switch(bufferInfo.fourccFormat)
    {
    case DRM_FORMAT_ARGB8888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                writePixelARGB8888(row, col);
        break;
    case DRM_FORMAT_XRGB8888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                writePixelXRGB8888(row, col);
        break;
    case DRM_FORMAT_XBGR8888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                writePixelXBGR8888(row, col);
        break;
    case DRM_FORMAT_BGR888:
        for(uint32_t row = 0u; row < bufferInfo.height; ++row)
            for(uint32_t col = 0u; col < bufferInfo.width; ++col)
                writePixelBGR8888(row, col);
        break;
    default:
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Trying to write test content to unknown buffer format!");
        assert(false);
        return false;
    }

    if(!endSyncBuffer(bufferInfo))
        return false;

    return true;
}

bool DmaOffscreenBufferTests::createAndMapDmaOffscreenBuffer(RendererTestsFramework& testFramework, uint32_t displayIdx, uint32_t bufferWidth, uint32_t bufferHeight, uint32_t fourccFormat, uint32_t bufferUsageFlags, EMappingBufferAccessRight accessRights, TestBufferInfo& bufferInfo, bool disableClearing)
{
    bufferInfo.displayIdx = displayIdx;
    bufferInfo.width = bufferWidth;
    bufferInfo.height = bufferHeight;
    bufferInfo.fourccFormat = fourccFormat;

    bufferInfo.displaBufferId = testFramework.createDmaOffscreenBuffer(displayIdx, bufferWidth, bufferHeight, fourccFormat, bufferUsageFlags, DRM_FORMAT_MOD_INVALID);
    if(!bufferInfo.displaBufferId.isValid())
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Invalid offscreen buffer handle!");
        return false;
    }

    if(disableClearing)
    {
        testFramework.setClearFlags(displayIdx, bufferInfo.displaBufferId, ramses::EClearFlags_None);
        testFramework.flushRendererAndDoOneLoop();
    }

    if(!testFramework.getDmaOffscreenBufferFDAndStride(displayIdx, bufferInfo.displaBufferId, bufferInfo.fd, bufferInfo.stride))
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Could not get FD and stride for DMA offscreen buffer!");
        return false;
    }

    bufferInfo.accessRights = accessRights;
    int accessRightsFlags = 0;
    switch(accessRights)
    {
    case EMappingBufferAccessRight::ReadOnly:
        accessRightsFlags = PROT_READ;
        break;
    case EMappingBufferAccessRight::WriteOnly:
        accessRightsFlags = PROT_WRITE;
        break;
    case EMappingBufferAccessRight::ReadWrite:
        accessRightsFlags = PROT_READ | PROT_WRITE;
        break;
    }
    assert(accessRightsFlags != 0);

    const auto fileSize = lseek(bufferInfo.fd, 0u, SEEK_END);
    bufferInfo.mappedMemory = mmap(nullptr, fileSize, accessRightsFlags, MAP_SHARED, bufferInfo.fd, 0u);
    if(bufferInfo.mappedMemory == MAP_FAILED)
    {
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Failed to map memory!");
        return false;
    }

    return true;
}

bool DmaOffscreenBufferTests::runBufferWriteTest(RendererTestsFramework& testFramework)
{
    constexpr uint32_t bufferWidth = 256u;
    constexpr uint32_t bufferHeight = 256u;
    constexpr uint32_t fourccFormat = DefaultFourccBufferFormat;
    constexpr uint32_t bufferUsageFlags = DefaultUsageFlags;

    //write some content into mapped memory of DMA OB, then link to tex consumer and make screenshot test
    if(!createAndMapDmaOffscreenBuffer(testFramework, 0u, bufferWidth, bufferHeight, fourccFormat, bufferUsageFlags, EMappingBufferAccessRight::WriteOnly, testBuffer1))
        return false;

    if(!writeTestContentToMappedMemory(testBuffer1))
        return false;

    const auto sceneIdConsumer = createAndShowScene<TextureLinkScene>(testFramework, 0u, TextureLinkScene::DATA_CONSUMER, m_cameraTranslation);
    testFramework.createBufferDataLink(testBuffer1.displaBufferId, sceneIdConsumer, TextureLinkScene::DataConsumerId);

    return testFramework.renderAndCompareScreenshot("DmaOffscreenBufferTest_BufferWrite");
}

bool DmaOffscreenBufferTests::runBufferReadTest(RendererTestsFramework &testFramework)
{
    constexpr uint32_t bufferWidth = 200u;
    constexpr uint32_t bufferHeight = 200u;
    constexpr uint32_t fourccFormat = DefaultFourccBufferFormat;
    constexpr uint32_t bufferUsageFlags = DefaultUsageFlags;

    if(!createAndMapDmaOffscreenBuffer(testFramework, 0u, bufferWidth, bufferHeight, fourccFormat, bufferUsageFlags, EMappingBufferAccessRight::ReadOnly, testBuffer1, false))
        return false;

    return renderSceneToBufferAndExpectScreenshot<MultipleTrianglesScene>(testFramework, testBuffer1, MultipleTrianglesScene::THREE_TRIANGLES, "MultipleTrianglesScene_ThreeTriangles");
}

bool DmaOffscreenBufferTests::runContentWithAlphaBlendingTest(RendererTestsFramework &testFramework)
{
    constexpr uint32_t bufferWidth = 200u;
    constexpr uint32_t bufferHeight = 200u;
    constexpr uint32_t fourccFormat = DefaultFourccBufferFormat;
    constexpr uint32_t bufferUsageFlags = DefaultUsageFlags;

    if(!createAndMapDmaOffscreenBuffer(testFramework, 0u, bufferWidth, bufferHeight, fourccFormat, bufferUsageFlags, EMappingBufferAccessRight::ReadOnly, testBuffer1, false))
        return false;

    return renderSceneToBufferAndExpectScreenshot<MultipleTrianglesScene>(testFramework, testBuffer1, MultipleTrianglesScene::ALPHA_BLENDING, "MultipleTrianglesScene_AlphaBlending");
}

bool DmaOffscreenBufferTests::runBufferReadWriteTest(RendererTestsFramework &testFramework, uint32_t displayIdx, uint32_t bufferWidth, uint32_t bufferHeight, uint32_t fourccFormat, uint32_t bufferUsageFlags, const std::string& screenshotImage)
{
    if(!createAndMapDmaOffscreenBuffer(testFramework, displayIdx, bufferWidth, bufferHeight, fourccFormat, bufferUsageFlags, EMappingBufferAccessRight::ReadWrite, testBuffer1))
        return false;

    //write color gradient to OB
    if(!writeTestContentToMappedMemory(testBuffer1))
        return false;

    //render triangles scene above (since clearing is disabled) the written color gradient
    return renderSceneToBufferAndExpectScreenshot<MultipleTrianglesScene>(testFramework, testBuffer1, MultipleTrianglesScene::THREE_TRIANGLES, screenshotImage);
}

bool DmaOffscreenBufferTests::runDoubleBufferedReadTest(RendererTestsFramework& testFramework)
{
    //The test creates a scene and 2 DMA OBs, then keeps alternating the scene between both OBs each frame
    //The contents of the DMA OB are checked against a screenshot from 2 frames ago to make sure the content was already rendered
    constexpr uint32_t bufferWidth = 200u;
    constexpr uint32_t bufferHeight = 200u;
    constexpr uint32_t fourccFormat = DefaultFourccBufferFormat;
    constexpr uint32_t bufferUsageFlags = DefaultUsageFlags;
    const float expectedPixelError = RendererTestUtils::DefaultMaxAveragePercentPerPixel;

    //Create a dummy scene and assign to FB to make sure swap buffers is called every frame
    createAndShowScene<MultipleTrianglesScene>(testFramework, 0u, MultipleTrianglesScene::THREE_TRIANGLES, Vector3{0.f}, bufferWidth, bufferHeight);

    const auto sceneIdProvider = createAndShowScene<MultipleTrianglesScene>(testFramework, 0u, MultipleTrianglesScene::THREE_TRIANGLES, Vector3{0.f}, bufferWidth, bufferHeight);

    if(!createAndMapDmaOffscreenBuffer(testFramework, 0, bufferWidth, bufferHeight, fourccFormat, bufferUsageFlags, EMappingBufferAccessRight::ReadOnly, testBuffer1))
        return false;
    if(!createAndMapDmaOffscreenBuffer(testFramework, 0, bufferWidth, bufferHeight, fourccFormat, bufferUsageFlags, EMappingBufferAccessRight::ReadOnly, testBuffer2))
        return false;

    testFramework.assignSceneToDisplayBuffer(sceneIdProvider, testBuffer1.displaBufferId);
    testFramework.flushRendererAndDoOneLoop();

    //change scene state and render to OB2
    testFramework.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneIdProvider, MultipleTrianglesScene::TRIANGLES_REORDERED);
    testFramework.getScenesRegistry().getScene(sceneIdProvider).flush();
    testFramework.assignSceneToDisplayBuffer(sceneIdProvider, testBuffer2.displaBufferId);
    testFramework.flushRendererAndDoOneLoop();

    //check result of OB1 from two loops ago
    const Image ob1MemImageResult = readMappedMemoryContentToImage(testBuffer1);
    if(!RendererTestUtils::CompareBitmapToImageInFile(ob1MemImageResult, "MultipleTrianglesScene_ThreeTriangles", expectedPixelError, true))
        return false;

    testFramework.getScenesRegistry().destroyScenes();
    testFramework.flushRendererAndDoOneLoop();

    //check result of OB2 from two loops ago
    const Image ob2MemImageResult = readMappedMemoryContentToImage(testBuffer2);
    if(!RendererTestUtils::CompareBitmapToImageInFile(ob2MemImageResult, "MultipleTrianglesScene_RenderingOrderChanged", expectedPixelError, true))
        return false;

    return true;
}

bool DmaOffscreenBufferTests::runTwoDisplaysTest(RendererTestsFramework &testFramework)
{
    if(!createAndMapDmaOffscreenBuffer(testFramework, 0u, 256u, 256u, DefaultFourccBufferFormat, DefaultUsageFlags, EMappingBufferAccessRight::ReadWrite, testBuffer1))
        return false;
    if(!createAndMapDmaOffscreenBuffer(testFramework, 1u, 128u, 64u, DefaultFourccBufferFormat, DefaultUsageFlags, EMappingBufferAccessRight::ReadWrite, testBuffer2))
        return false;

    //write color gradient to OBs
    if(!writeTestContentToMappedMemory(testBuffer1))
        return false;
    if(!writeTestContentToMappedMemory(testBuffer2))
        return false;

    if(!renderSceneToBufferAndExpectScreenshot<MultipleTrianglesScene>(testFramework, testBuffer1, MultipleTrianglesScene::THREE_TRIANGLES, "DmaOffscreenBufferTest_BufferWrite_256x256"))
        return false;
    return renderSceneToBufferAndExpectScreenshot<MultipleTrianglesScene>(testFramework, testBuffer2, MultipleTrianglesScene::THREE_TRIANGLES, "DmaOffscreenBufferTest_BufferWrite_128x64");
}
