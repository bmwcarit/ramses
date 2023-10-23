//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/DisplayEventHandler.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "internal/RendererLib/Enums/EKeyEvent.h"
#include "internal/RendererLib/Enums/EKeyModifier.h"
#include "internal/Core/Utils/ThreadLocalLog.h"

namespace ramses::internal
{
    using namespace testing;

    class ADisplayEventHandler : public ::testing::Test
    {
    protected:
        ADisplayEventHandler()
            : m_displayHandle(5110u)
            , m_displayEventHandler(m_displayHandle, m_eventCollector)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
        }

        [[nodiscard]] RendererEvent getRendererEvent(uint32_t index) const
        {
            const auto events = m_eventCollector.getRendererEvents();
            if (events.size() <= index)
            {
                return RendererEvent(ERendererEventType::Invalid);
            }
            return events[index];
        }

        RendererEventCollector m_eventCollector;
        DisplayHandle m_displayHandle;
        DisplayEventHandler m_displayEventHandler;
    };

    TEST_F(ADisplayEventHandler, createsRendererEventOnKeyPressEvent)
    {
        KeyModifiers modifier = EKeyModifier::Alt | EKeyModifier::Shift;
        EKeyCode keyCode = ramses::EKeyCode_4;
        EKeyEvent keyEventType = EKeyEvent::Pressed;
        m_displayEventHandler.onKeyEvent(keyEventType, modifier, keyCode);
        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowKeyEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(keyEventType, event.keyEvent.type);
        EXPECT_EQ(modifier, event.keyEvent.modifier);
        EXPECT_EQ(keyCode, event.keyEvent.keyCode);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventOnKeyReleasedEvent)
    {
        KeyModifiers modifier = EKeyModifier::Alt | EKeyModifier::Shift;
        EKeyCode keyCode = ramses::EKeyCode_A;
        EKeyEvent keyEventType = EKeyEvent::Released;
        m_displayEventHandler.onKeyEvent(keyEventType, modifier, keyCode);
        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowKeyEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(keyEventType, event.keyEvent.type);
        EXPECT_EQ(modifier, event.keyEvent.modifier);
        EXPECT_EQ(keyCode, event.keyEvent.keyCode);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventOnMouseMoveEvent)
    {
        const glm::ivec2 mousePos(5, 20);
        m_displayEventHandler.onMouseEvent(EMouseEvent::Move, mousePos.x, mousePos.y);
        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowMouseEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(EMouseEvent::Move, event.mouseEvent.type);
        EXPECT_EQ(mousePos, event.mouseEvent.pos);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventAndInvokesTouchHandlerOnMouseEvent)
    {
        glm::ivec2 mousePosition(90u, 250u);
        EMouseEvent mouseEventType = EMouseEvent::LeftButtonDown;
        m_displayEventHandler.onMouseEvent(mouseEventType, mousePosition.x, mousePosition.y);

        // check renderer event collector
        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowMouseEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(mouseEventType, event.mouseEvent.type);
        EXPECT_EQ(mousePosition, event.mouseEvent.pos);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventOnMouseEventWindowEnter)
    {
        m_displayEventHandler.onMouseEvent(EMouseEvent::WindowEnter, 5, 10);
        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowMouseEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(EMouseEvent::WindowEnter, event.mouseEvent.type);
        EXPECT_EQ(glm::ivec2(5, 10), event.mouseEvent.pos);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventOnMouseEventWindowLeave)
    {
        m_displayEventHandler.onMouseEvent(EMouseEvent::WindowLeave, 10, 20);
        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowMouseEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(EMouseEvent::WindowLeave, event.mouseEvent.type);
        EXPECT_EQ(glm::ivec2(10, 20), event.mouseEvent.pos);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventOnWindowClosedEvent)
    {
        m_displayEventHandler.onClose();

        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowClosed, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventOnWindowResizeEvent)
    {
        m_displayEventHandler.onResize(1280u, 480u);

        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowResizeEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(1280u, event.resizeEvent.width);
        EXPECT_EQ(480u, event.resizeEvent.height);
    }

    TEST_F(ADisplayEventHandler, createsRendererEventOnWindowMoveEvent)
    {
        m_displayEventHandler.onWindowMove(1280, 480);

        const RendererEvent event = getRendererEvent(0u);
        EXPECT_EQ(ERendererEventType::WindowMoveEvent, event.eventType);
        EXPECT_EQ(m_displayHandle, event.displayHandle);
        EXPECT_EQ(1280, event.moveEvent.posX);
        EXPECT_EQ(480, event.moveEvent.posY);
    }
}
