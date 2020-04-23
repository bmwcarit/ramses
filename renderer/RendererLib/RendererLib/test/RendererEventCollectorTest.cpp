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

using namespace testing;

namespace ramses_internal
{
    class ARendererEventCollector : public testing::Test
    {
    protected:
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
        m_rendererEventCollector.addDisplayEvent(ERendererEventType_DisplayCreated, m_displayHandle);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayCreated, resultEvents[0].eventType);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithPixelData)
    {
        UInt8Vector pixelData;
        pixelData.push_back(8);

        m_rendererEventCollector.addReadPixelsEvent(ERendererEventType_ReadPixelsFromFramebuffer, m_displayHandle, std::move(pixelData));
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(ERendererEventType_ReadPixelsFromFramebuffer, resultEvents[0].eventType);
        ASSERT_EQ(1u, resultEvents[0].pixelData.size());
        EXPECT_EQ(8, resultEvents[0].pixelData[0]);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithDisplayConfig)
    {
        DisplayConfig config;
        config.setWarpingEnabled(true);

        m_rendererEventCollector.addDisplayEvent(ERendererEventType_DisplayCreated, m_displayHandle);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayCreated, resultEvents[0].eventType);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithLinkInfo)
    {
        const SceneId providerSceneId(0u);
        const SceneId consumerSceneId(1u);
        const DataSlotId providerdataId(2u);
        const DataSlotId consumerdataId(3u);

        m_rendererEventCollector.addDataLinkEvent(ERendererEventType_SceneDataLinked, providerSceneId, consumerSceneId, providerdataId, consumerdataId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_SceneDataLinked, resultEvents[0].eventType);
        EXPECT_EQ(providerSceneId, resultEvents[0].providerSceneId);
        EXPECT_EQ(consumerSceneId, resultEvents[0].consumerSceneId);
        EXPECT_EQ(providerdataId, resultEvents[0].providerdataId);
        EXPECT_EQ(consumerdataId, resultEvents[0].consumerdataId);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithBufferLinkInfo)
    {
        const OffscreenBufferHandle providerBuffer(11u);
        const SceneId consumerSceneId(1u);
        const DataSlotId consumerdataId(3u);

        m_rendererEventCollector.addOBLinkEvent(ERendererEventType_SceneDataBufferLinked, providerBuffer, consumerSceneId, consumerdataId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_SceneDataBufferLinked, resultEvents[0].eventType);
        EXPECT_EQ(providerBuffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(consumerSceneId, resultEvents[0].consumerSceneId);
        EXPECT_EQ(consumerdataId, resultEvents[0].consumerdataId);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithBufferCreateDestroy)
    {
        const OffscreenBufferHandle buffer(11u);
        const DisplayHandle display(22u);

        m_rendererEventCollector.addOBEvent(ERendererEventType_OffscreenBufferCreated, buffer, display);
        RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferCreated, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addOBEvent(ERendererEventType_OffscreenBufferCreateFailed, buffer, display);
        resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferCreateFailed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addOBEvent(ERendererEventType_OffscreenBufferDestroyed, buffer, display);
        resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferDestroyed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addOBEvent(ERendererEventType_OffscreenBufferDestroyFailed, buffer, display);
        resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferDestroyFailed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);
    }

    TEST_F(ARendererEventCollector, CanAddSceneFlushEvent)
    {
        const SceneId sceneId(123u);
        const SceneVersionTag sceneVersionTag(345u);
        const EResourceStatus resourceStatus(EResourceStatus_Uploaded);

        m_rendererEventCollector.addSceneFlushEvent(ERendererEventType_SceneFlushed, sceneId, sceneVersionTag, resourceStatus);
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_SceneFlushed, resultEvents[0].eventType);
        EXPECT_EQ(sceneId, resultEvents[0].sceneId);
        EXPECT_EQ(sceneVersionTag, resultEvents[0].sceneVersionTag);
        EXPECT_EQ(resourceStatus, resultEvents[0].resourceStatus);
    }

    TEST_F(ARendererEventCollector, CanAddKeyPressEvent)
    {
        const DisplayHandle displayHandle(125u);
        KeyEvent keyEvent;
        keyEvent.modifier = EKeyModifier_Ctrl | EKeyModifier_Alt;
        keyEvent.keyCode = EKeyCode_K;
        keyEvent.type = EKeyEventType_Pressed;

        m_rendererEventCollector.addWindowEvent(ERendererEventType_WindowKeyEvent, displayHandle, keyEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_WindowKeyEvent, resultEvents[0].eventType);
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

        m_rendererEventCollector.addWindowEvent(ERendererEventType_WindowMouseEvent, displayHandle, mouseEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_WindowMouseEvent, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(mouseEvent.type, resultEvents[0].mouseEvent.type);
        EXPECT_EQ(mouseEvent.pos, resultEvents[0].mouseEvent.pos);
    }

    TEST_F(ARendererEventCollector, CanAddWindowClosedEvent)
    {
        const DisplayHandle displayHandle(1287u);

        m_rendererEventCollector.addDisplayEvent(ERendererEventType_WindowClosed, displayHandle);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_WindowClosed, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
    }

    TEST_F(ARendererEventCollector, CanAddStreamSurfaceAvailableEvent)
    {
        const StreamTextureSourceId streamId(294u);

        m_rendererEventCollector.addStreamSourceEvent(ERendererEventType_StreamSurfaceAvailable, streamId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();

        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_StreamSurfaceAvailable, resultEvents[0].eventType);
        EXPECT_EQ(streamId, resultEvents[0].streamSourceId);
    }

    TEST_F(ARendererEventCollector, CanAddStreamSurfaceUnavailableEvent)
    {
        const StreamTextureSourceId streamId(794u);

        m_rendererEventCollector.addStreamSourceEvent(ERendererEventType_StreamSurfaceUnavailable, streamId);
        const RendererEventVector resultEvents = consumeSceneControlEvents();

        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_StreamSurfaceUnavailable, resultEvents[0].eventType);
        EXPECT_EQ(streamId, resultEvents[0].streamSourceId);
    }

    TEST_F(ARendererEventCollector, CanAddAndDispatchWindowResizeEvent)
    {
        const DisplayHandle displayHandle(124u);
        ResizeEvent resizeEvent;
        resizeEvent.width = 567;
        resizeEvent.height = 321;

        m_rendererEventCollector.addWindowEvent(ERendererEventType_WindowResizeEvent, displayHandle, resizeEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_WindowResizeEvent, resultEvents[0].eventType);
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

        m_rendererEventCollector.addWindowEvent(ERendererEventType_WindowMoveEvent, displayHandle, moveEvent);
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_WindowMoveEvent, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(moveEvent.posX, resultEvents[0].moveEvent.posX);
        EXPECT_EQ(moveEvent.posY, resultEvents[0].moveEvent.posY);

    }

    TEST_F(ARendererEventCollector, CanAddAndDispatchObjectsPickedEvent)
    {
        const SceneId sceneId{ 123u };
        const PickableObjectId pickable1{ 567u };
        const PickableObjectId pickable2{ 2567u };

        m_rendererEventCollector.addPickedEvent(ERendererEventType_ObjectsPicked, sceneId, { pickable1, pickable2 });
        const RendererEventVector resultEvents = consumeRendererEvents();
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_ObjectsPicked, resultEvents[0].eventType);
        EXPECT_EQ(sceneId, resultEvents[0].sceneId);

        ASSERT_EQ(2u, resultEvents[0].pickedObjectIds.size());
        EXPECT_EQ(pickable1, resultEvents[0].pickedObjectIds[0]);
        EXPECT_EQ(pickable2, resultEvents[0].pickedObjectIds[1]);
    }

    TEST_F(ARendererEventCollector, QueuesUpInternalSceneEvents)
    {
        constexpr SceneId sceneId1{ 123u };
        constexpr SceneId sceneId2{ 124u };
        constexpr SceneId sceneId3{ 125u };
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType_ScenePublished, sceneId1);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType_SceneMapFailed, sceneId2);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType_SceneShown, sceneId3);

        InternalSceneStateEvents sceneEvts;
        m_rendererEventCollector.dispatchInternalSceneStateEvents(sceneEvts);
        ASSERT_EQ(3u, sceneEvts.size());
        EXPECT_EQ(sceneId1, sceneEvts[0].sceneId);
        EXPECT_EQ(sceneId2, sceneEvts[1].sceneId);
        EXPECT_EQ(sceneId3, sceneEvts[2].sceneId);
        EXPECT_EQ(ERendererEventType_ScenePublished, sceneEvts[0].type);
        EXPECT_EQ(ERendererEventType_SceneMapFailed, sceneEvts[1].type);
        EXPECT_EQ(ERendererEventType_SceneShown, sceneEvts[2].type);

        // for now scene events are pushed to both internal event queue and scene control event queue
        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(3u, resultEvents.size());
        EXPECT_EQ(sceneId1, resultEvents[0].sceneId);
        EXPECT_EQ(sceneId2, resultEvents[1].sceneId);
        EXPECT_EQ(sceneId3, resultEvents[2].sceneId);
        EXPECT_EQ(ERendererEventType_ScenePublished, resultEvents[0].eventType);
        EXPECT_EQ(ERendererEventType_SceneMapFailed, resultEvents[1].eventType);
        EXPECT_EQ(ERendererEventType_SceneShown, resultEvents[2].eventType);
    }

    TEST_F(ARendererEventCollector, QueuesUpSceneEvents)
    {
        constexpr SceneId sceneId1{ 123u };
        constexpr SceneId sceneId2{ 124u };
        constexpr SceneId sceneId3{ 125u };
        m_rendererEventCollector.addSceneEvent(ERendererEventType_ScenePublished, sceneId1, RendererSceneState::Unavailable);
        m_rendererEventCollector.addSceneEvent(ERendererEventType_SceneStateChanged, sceneId2, RendererSceneState::Available);
        m_rendererEventCollector.addSceneEvent(ERendererEventType_SceneStateChanged, sceneId3, RendererSceneState::Ready);

        const RendererEventVector resultEvents = consumeSceneControlEvents();
        ASSERT_EQ(3u, resultEvents.size());
        EXPECT_EQ(sceneId1, resultEvents[0].sceneId);
        EXPECT_EQ(sceneId2, resultEvents[1].sceneId);
        EXPECT_EQ(sceneId3, resultEvents[2].sceneId);
        EXPECT_EQ(ERendererEventType_ScenePublished, resultEvents[0].eventType);
        EXPECT_EQ(ERendererEventType_SceneStateChanged, resultEvents[1].eventType);
        EXPECT_EQ(ERendererEventType_SceneStateChanged, resultEvents[2].eventType);
        EXPECT_EQ(RendererSceneState::Unavailable, resultEvents[0].state);
        EXPECT_EQ(RendererSceneState::Available, resultEvents[1].state);
        EXPECT_EQ(RendererSceneState::Ready, resultEvents[2].state);
    }
}
