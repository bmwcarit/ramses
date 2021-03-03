//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererEventCollector.h"
#include "RendererLib/EKeyModifier.h"
#include "Utils/ThreadLocalLog.h"

using namespace testing;

namespace ramses_internal
{
    class ARendererEventCollector : public testing::Test
    {
    protected:
        ARendererEventCollector()
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
        }

        virtual void TearDown() override
        {
            // make sure all test cases dispatch all collected events
            RendererEventVector resultEvents;
            m_rendererEventCollector.appendAndConsumePendingEvents(resultEvents, resultEvents);
            EXPECT_TRUE(resultEvents.empty());
            InternalSceneStateEvents sceneEvts;
            m_rendererEventCollector.dispatchInternalSceneStateEvents(sceneEvts);
            EXPECT_TRUE(sceneEvts.empty());
        }

        RendererEventVector consumeRendererEvents()
        {
            RendererEventVector resultEvents;
            RendererEventVector dummy;
            m_rendererEventCollector.appendAndConsumePendingEvents(resultEvents, dummy);
            return resultEvents;
        }

        RendererEventVector consumeSceneControlEvents()
        {
            RendererEventVector resultEvents;
            RendererEventVector dummy;
            m_rendererEventCollector.appendAndConsumePendingEvents(dummy, resultEvents);
            return resultEvents;
        }

        DisplayHandle m_displayHandle;
        RendererEventCollector m_rendererEventCollector;
    };

    TEST_F(ARendererEventCollector, CanAddRendererEvent)
    {
        m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayCreated, m_displayHandle);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(ERendererEventType::DisplayCreated, resultEvents[0].eventType);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithPixelData)
    {
        UInt8Vector pixelData;
        pixelData.push_back(8);

        const OffscreenBufferHandle bufferHandle{ 123u };
        m_rendererEventCollector.addReadPixelsEvent(ERendererEventType::ReadPixelsFromFramebuffer, m_displayHandle, bufferHandle, std::move(pixelData));
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(bufferHandle, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(ERendererEventType::ReadPixelsFromFramebuffer, resultEvents[0].eventType);
        ASSERT_EQ(1u, resultEvents[0].pixelData.size());
        EXPECT_EQ(8, resultEvents[0].pixelData[0]);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithDisplayConfig)
    {
        DisplayConfig config;
        config.setWarpingEnabled(true);

        m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayCreated, m_displayHandle);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(ERendererEventType::DisplayCreated, resultEvents[0].eventType);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithLinkInfo)
    {
        const SceneId providerSceneId(0u);
        const SceneId consumerSceneId(1u);
        const DataSlotId providerdataId(2u);
        const DataSlotId consumerdataId(3u);

        m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataLinked, providerSceneId, consumerSceneId, providerdataId, consumerdataId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::SceneDataLinked, resultEvents[0].eventType);
        EXPECT_EQ(providerSceneId, resultEvents[0].providerSceneId);
        EXPECT_EQ(consumerSceneId, resultEvents[0].consumerSceneId);
        EXPECT_EQ(providerdataId, resultEvents[0].providerdataId);
        EXPECT_EQ(consumerdataId, resultEvents[0].consumerdataId);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithBufferLinkInfo_OB)
    {
        const OffscreenBufferHandle providerBuffer(11u);
        const SceneId consumerSceneId(1u);
        const DataSlotId consumerdataId(3u);

        m_rendererEventCollector.addBufferLinkEvent(ERendererEventType::SceneDataBufferLinked, providerBuffer, consumerSceneId, consumerdataId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::SceneDataBufferLinked, resultEvents[0].eventType);
        EXPECT_FALSE(resultEvents[0].streamBuffer.isValid());
        EXPECT_EQ(providerBuffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(consumerSceneId, resultEvents[0].consumerSceneId);
        EXPECT_EQ(consumerdataId, resultEvents[0].consumerdataId);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithBufferLinkInfo_SB)
    {
        const StreamBufferHandle providerBuffer(11u);
        const SceneId consumerSceneId(1u);
        const DataSlotId consumerdataId(3u);

        m_rendererEventCollector.addBufferLinkEvent(ERendererEventType::SceneDataBufferLinked, providerBuffer, consumerSceneId, consumerdataId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::SceneDataBufferLinked, resultEvents[0].eventType);
        EXPECT_FALSE(resultEvents[0].offscreenBuffer.isValid());
        EXPECT_EQ(providerBuffer, resultEvents[0].streamBuffer);
        EXPECT_EQ(consumerSceneId, resultEvents[0].consumerSceneId);
        EXPECT_EQ(consumerdataId, resultEvents[0].consumerdataId);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithBufferCreateDestroy)
    {
        const OffscreenBufferHandle buffer(11u);
        const DisplayHandle display(22u);

        m_rendererEventCollector.addOBEvent(ERendererEventType::OffscreenBufferCreated, buffer, display);
        RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::OffscreenBufferCreated, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addOBEvent(ERendererEventType::OffscreenBufferCreateFailed, buffer, display);
        resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::OffscreenBufferCreateFailed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addOBEvent(ERendererEventType::OffscreenBufferDestroyed, buffer, display);
        resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::OffscreenBufferDestroyed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addOBEvent(ERendererEventType::OffscreenBufferDestroyFailed, buffer, display);
        resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::OffscreenBufferDestroyFailed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);
    }

    TEST_F(ARendererEventCollector, CanAddSceneFlushEvent)
    {
        const SceneId sceneId(123u);
        const SceneVersionTag sceneVersionTag(345u);

        m_rendererEventCollector.addSceneFlushEvent(ERendererEventType::SceneFlushed, sceneId, sceneVersionTag);
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::SceneFlushed, resultEvents[0].eventType);
        EXPECT_EQ(sceneId, resultEvents[0].sceneId);
        EXPECT_EQ(sceneVersionTag, resultEvents[0].sceneVersionTag);
    }

    TEST_F(ARendererEventCollector, CanAddKeyPressEvent)
    {
        const DisplayHandle displayHandle(125u);
        KeyEvent keyEvent;
        keyEvent.modifier = EKeyModifier_Ctrl | EKeyModifier_Alt;
        keyEvent.keyCode = EKeyCode_K;
        keyEvent.type = EKeyEventType_Pressed;

        m_rendererEventCollector.addWindowEvent(ERendererEventType::WindowKeyEvent, displayHandle, keyEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::WindowKeyEvent, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(keyEvent.type, resultEvents[0].keyEvent.type);
        EXPECT_EQ(keyEvent.keyCode, resultEvents[0].keyEvent.keyCode);
        EXPECT_EQ(keyEvent.modifier, resultEvents[0].keyEvent.modifier);
    }

    TEST_F(ARendererEventCollector, CanAddMouseEvent)
    {
        const DisplayHandle displayHandle(124u);
        MouseEvent mouseEvent;
        mouseEvent.pos = Vector2i(50u, 60u);
        mouseEvent.type = EMouseEventType_LeftButtonDown;

        m_rendererEventCollector.addWindowEvent(ERendererEventType::WindowMouseEvent, displayHandle, mouseEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::WindowMouseEvent, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(mouseEvent.type, resultEvents[0].mouseEvent.type);
        EXPECT_EQ(mouseEvent.pos, resultEvents[0].mouseEvent.pos);
    }

    TEST_F(ARendererEventCollector, CanAddWindowClosedEvent)
    {
        const DisplayHandle displayHandle(1287u);

        m_rendererEventCollector.addDisplayEvent(ERendererEventType::WindowClosed, displayHandle);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::WindowClosed, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
    }

    TEST_F(ARendererEventCollector, CanAddStreamSurfaceAvailableEvent)
    {
        const WaylandIviSurfaceId streamId(294u);

        m_rendererEventCollector.addStreamSourceEvent(ERendererEventType::StreamSurfaceAvailable, streamId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();

        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::StreamSurfaceAvailable, resultEvents[0].eventType);
        EXPECT_EQ(streamId, resultEvents[0].streamSourceId);
    }

    TEST_F(ARendererEventCollector, CanAddStreamSurfaceUnavailableEvent)
    {
        const WaylandIviSurfaceId streamId(794u);

        m_rendererEventCollector.addStreamSourceEvent(ERendererEventType::StreamSurfaceUnavailable, streamId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();

        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::StreamSurfaceUnavailable, resultEvents[0].eventType);
        EXPECT_EQ(streamId, resultEvents[0].streamSourceId);
    }

    TEST_F(ARendererEventCollector, CanAddAndDispatchWindowResizeEvent)
    {
        const DisplayHandle displayHandle(124u);
        ResizeEvent resizeEvent;
        resizeEvent.width = 567;
        resizeEvent.height = 321;

        m_rendererEventCollector.addWindowEvent(ERendererEventType::WindowResizeEvent, displayHandle, resizeEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::WindowResizeEvent, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(resizeEvent.width, resultEvents[0].resizeEvent.width);
        EXPECT_EQ(resizeEvent.height, resultEvents[0].resizeEvent.height);
    }

    TEST_F(ARendererEventCollector, CanAddAndDispatchWindowMoveEvent)
    {
        const DisplayHandle displayHandle(124u);
        WindowMoveEvent moveEvent;
        moveEvent.posX = 567;
        moveEvent.posY = 321;

        m_rendererEventCollector.addWindowEvent(ERendererEventType::WindowMoveEvent, displayHandle, moveEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::WindowMoveEvent, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(moveEvent.posX, resultEvents[0].moveEvent.posX);
        EXPECT_EQ(moveEvent.posY, resultEvents[0].moveEvent.posY);

    }

    TEST_F(ARendererEventCollector, CanAddAndDispatchObjectsPickedEvent)
    {
        const SceneId sceneId{ 123u };
        const PickableObjectId pickable1{ 567u };
        const PickableObjectId pickable2{ 2567u };

        m_rendererEventCollector.addPickedEvent(ERendererEventType::ObjectsPicked, sceneId, { pickable1, pickable2 });
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::ObjectsPicked, resultEvents[0].eventType);
        EXPECT_EQ(sceneId, resultEvents[0].sceneId);

        ASSERT_EQ(2u, resultEvents[0].pickedObjectIds.size());
        EXPECT_EQ(pickable1, resultEvents[0].pickedObjectIds[0]);
        EXPECT_EQ(pickable2, resultEvents[0].pickedObjectIds[1]);
    }

    TEST_F(ARendererEventCollector, CanAddFrameTimingEvent)
    {
        constexpr std::chrono::microseconds maxTime{ 123 };
        constexpr std::chrono::microseconds avgTime{ 321 };
        m_rendererEventCollector.addFrameTimingReport(maxTime, avgTime);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType::FrameTimingReport, resultEvents[0].eventType);
        EXPECT_EQ(maxTime, resultEvents[0].frameTimings.maximumLoopTimeWithinPeriod);
        EXPECT_EQ(avgTime, resultEvents[0].frameTimings.averageLoopTimeWithinPeriod);
    }

    TEST_F(ARendererEventCollector, QueuesUpInternalSceneEvents)
    {
        constexpr SceneId sceneId1{ 123u };
        constexpr SceneId sceneId2{ 124u };
        constexpr SceneId sceneId3{ 125u };
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::ScenePublished, sceneId1);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneMapFailed, sceneId2);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneShown, sceneId3);

        InternalSceneStateEvents sceneEvts;
        m_rendererEventCollector.dispatchInternalSceneStateEvents(sceneEvts);
        ASSERT_EQ(3u, sceneEvts.size());
        EXPECT_EQ(sceneId1, sceneEvts[0].sceneId);
        EXPECT_EQ(sceneId2, sceneEvts[1].sceneId);
        EXPECT_EQ(sceneId3, sceneEvts[2].sceneId);
        EXPECT_EQ(ERendererEventType::ScenePublished, sceneEvts[0].type);
        EXPECT_EQ(ERendererEventType::SceneMapFailed, sceneEvts[1].type);
        EXPECT_EQ(ERendererEventType::SceneShown, sceneEvts[2].type);
    }

    TEST_F(ARendererEventCollector, QueuesUpSceneEvents)
    {
        constexpr SceneId sceneId1{ 123u };
        constexpr SceneId sceneId2{ 124u };
        constexpr SceneId sceneId3{ 125u };
        m_rendererEventCollector.addSceneEvent(ERendererEventType::ScenePublished, sceneId1, RendererSceneState::Unavailable);
        m_rendererEventCollector.addSceneEvent(ERendererEventType::SceneStateChanged, sceneId2, RendererSceneState::Available);
        m_rendererEventCollector.addSceneEvent(ERendererEventType::SceneStateChanged, sceneId3, RendererSceneState::Ready);

        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(3u, resultEvents.size());
        EXPECT_EQ(sceneId1, resultEvents[0].sceneId);
        EXPECT_EQ(sceneId2, resultEvents[1].sceneId);
        EXPECT_EQ(sceneId3, resultEvents[2].sceneId);
        EXPECT_EQ(ERendererEventType::ScenePublished, resultEvents[0].eventType);
        EXPECT_EQ(ERendererEventType::SceneStateChanged, resultEvents[1].eventType);
        EXPECT_EQ(ERendererEventType::SceneStateChanged, resultEvents[2].eventType);
        EXPECT_EQ(RendererSceneState::Unavailable, resultEvents[0].state);
        EXPECT_EQ(RendererSceneState::Available, resultEvents[1].state);
        EXPECT_EQ(RendererSceneState::Ready, resultEvents[2].state);
    }
}
