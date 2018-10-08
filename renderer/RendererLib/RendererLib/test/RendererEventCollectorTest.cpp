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
        ARendererEventCollector()
        {
        }

        DisplayHandle m_displayHandle;
        RendererEventCollector m_rendererEventCollector;
    };

    TEST_F(ARendererEventCollector, CanAddRendererEvent)
    {
        m_rendererEventCollector.addEvent(ERendererEventType_DisplayCreated, m_displayHandle);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayCreated, resultEvents[0].eventType);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithPixelData)
    {
        UInt8Vector pixelData;
        pixelData.push_back(8);

        m_rendererEventCollector.addEvent(ERendererEventType_ReadPixelsFromFramebuffer, m_displayHandle, pixelData);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
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

        m_rendererEventCollector.addEvent(ERendererEventType_DisplayCreated, m_displayHandle);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(m_displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(ERendererEventType_DisplayCreated, resultEvents[0].eventType);
    }

    TEST_F(ARendererEventCollector, CanAddSceneStateEvent)
    {
        const SceneId sceneId(6u);
        m_rendererEventCollector.addEvent(ERendererEventType_SceneMapped, sceneId);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(sceneId, resultEvents[0].sceneId);
        EXPECT_EQ(ERendererEventType_SceneMapped, resultEvents[0].eventType);
    }

    TEST_F(ARendererEventCollector, CanAddRendererEventWithLinkInfo)
    {
        const SceneId providerSceneId(0u);
        const SceneId consumerSceneId(1u);
        const DataSlotId providerdataId(2u);
        const DataSlotId consumerdataId(3u);

        m_rendererEventCollector.addEvent(ERendererEventType_SceneDataLinked, providerSceneId, consumerSceneId, providerdataId, consumerdataId);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
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

        m_rendererEventCollector.addEvent(ERendererEventType_SceneDataBufferLinked, providerBuffer, consumerSceneId, consumerdataId);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
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

        m_rendererEventCollector.addEvent(ERendererEventType_OffscreenBufferCreated, buffer, display);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferCreated, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addEvent(ERendererEventType_OffscreenBufferCreateFailed, buffer, display);
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferCreateFailed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addEvent(ERendererEventType_OffscreenBufferDestroyed, buffer, display);
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_OffscreenBufferDestroyed, resultEvents[0].eventType);
        EXPECT_EQ(buffer, resultEvents[0].offscreenBuffer);
        EXPECT_EQ(display, resultEvents[0].displayHandle);

        m_rendererEventCollector.addEvent(ERendererEventType_OffscreenBufferDestroyFailed, buffer, display);
        m_rendererEventCollector.dispatchEvents(resultEvents);
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

        m_rendererEventCollector.addEvent(ERendererEventType_SceneFlushed, sceneId, sceneVersionTag, resourceStatus);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
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

        m_rendererEventCollector.addEvent(ERendererEventType_WindowKeyEvent, displayHandle, keyEvent);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
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

        m_rendererEventCollector.addEvent(ERendererEventType_WindowMouseEvent, displayHandle, mouseEvent);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_WindowMouseEvent, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
        EXPECT_EQ(mouseEvent.type, resultEvents[0].mouseEvent.type);
        EXPECT_EQ(mouseEvent.pos, resultEvents[0].mouseEvent.pos);
    }

    TEST_F(ARendererEventCollector, CanAddWindowClosedEvent)
    {
        const DisplayHandle displayHandle(1287u);

        m_rendererEventCollector.addEvent(ERendererEventType_WindowClosed, displayHandle);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);
        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_WindowClosed, resultEvents[0].eventType);
        EXPECT_EQ(displayHandle, resultEvents[0].displayHandle);
    }

    TEST_F(ARendererEventCollector, CanAddStreamSurfaceAvailableEvent)
    {
        const StreamTextureSourceId streamId(294u);

        m_rendererEventCollector.addEvent(ERendererEventType_StreamSurfaceAvailable, streamId);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);

        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_StreamSurfaceAvailable, resultEvents[0].eventType);
        EXPECT_EQ(streamId, resultEvents[0].streamSourceId);
    }

    TEST_F(ARendererEventCollector, CanAddStreamSurfaceUnavailableEvent)
    {
        const StreamTextureSourceId streamId(794u);

        m_rendererEventCollector.addEvent(ERendererEventType_StreamSurfaceUnavailable, streamId);
        RendererEventVector resultEvents;
        m_rendererEventCollector.dispatchEvents(resultEvents);

        ASSERT_EQ(1u, resultEvents.size());
        EXPECT_EQ(ERendererEventType_StreamSurfaceUnavailable, resultEvents[0].eventType);
        EXPECT_EQ(streamId, resultEvents[0].streamSourceId);
    }
}
