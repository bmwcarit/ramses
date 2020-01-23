//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTCASES_H
#define RAMSES_TESTCASES_H

namespace ramses_internal
{

    TYPED_TEST_CASE_P(AWindowWaylandWithEventHandling);

    TYPED_TEST_P(AWindowWaylandWithEventHandling, singleKeyPressEventTriggersKeyPressedEventWithCorrectKeyCode)
    {
        this->testKeyCode(KEY_A,              EKeyCode_A);
        this->testKeyCode(KEY_0,              EKeyCode_0);
        this->testKeyCode(KEY_ESC,            EKeyCode_Escape);
        this->testKeyCode(KEY_ENTER,          EKeyCode_Return);
        this->testKeyCode(KEY_TAB,            EKeyCode_Tab);
        this->testKeyCode(KEY_F1,             EKeyCode_F1);
        this->testKeyCode(KEY_SPACE,          EKeyCode_Space);
        this->testKeyCode(KEY_LEFTSHIFT,      EKeyCode_ShiftLeft,     EKeyModifier_Shift);
        this->testKeyCode(KEY_RIGHTALT,       EKeyCode_AltRight,      EKeyModifier_Alt);
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, leftMouseButtonDownEventTriggersLeftButtonDownEvent)
    {
        this->sendMouseButtonEvent(BTN_LEFT, true);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, _, _)).Times(2);
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonDown, _, _)).Times(1);
        this->processAllEvents();

        // release button to apply default button state for following tests
        this->sendMouseButtonEvent(BTN_LEFT, false);
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, leftMouseButtonUpEventTriggersLeftButtonUpEvent)
    {
        this->sendMouseButtonEvent(BTN_LEFT, true); // to simulate button release event we have to press it first
        this->sendMouseButtonEvent(BTN_LEFT, false);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, _, _)).Times(3);
        Sequence seq;
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonDown, _, _)).Times(1).InSequence(seq);
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonUp, _, _)).Times(1).InSequence(seq);
        this->processAllEvents();
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, rightMouseButtonDownEventTriggersRightButtonDownEvent)
    {
        this->sendMouseButtonEvent(BTN_RIGHT, true);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, _, _)).Times(2);
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_RightButtonDown, _, _)).Times(1);
        this->processAllEvents();

        // release button to apply default button state for following tests
        this->sendMouseButtonEvent(BTN_RIGHT, false);
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, rightMouseButtonUpEventTriggersRightButtonUpEvent)
    {
        this->sendMouseButtonEvent(BTN_RIGHT, true); // to simulate button release event we have to press it first
        this->sendMouseButtonEvent(BTN_RIGHT, false);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, _, _)).Times(3);
        Sequence seq;
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_RightButtonDown, _, _)).Times(1).InSequence(seq);
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_RightButtonUp, _, _)).Times(1).InSequence(seq);
        this->processAllEvents();
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, middleMouseButtonDownEventTriggersMiddleButtonDownEvent)
    {
        this->sendMouseButtonEvent(BTN_MIDDLE, true);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, _, _)).Times(2);
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_MiddleButtonDown, _, _)).Times(1);
        this->processAllEvents();

        // release button to apply default button state for following tests
        this->sendMouseButtonEvent(BTN_MIDDLE, false);
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, middleMouseButtonUpEventTriggersMiddleButtonUpEvent)
    {
        this->sendMouseButtonEvent(BTN_MIDDLE, true); // to simulate button release event we have to press it first
        this->sendMouseButtonEvent(BTN_MIDDLE, false);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, _, _)).Times(3);
        Sequence seq;
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_MiddleButtonDown, _, _)).Times(1).InSequence(seq);
        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_MiddleButtonUp, _, _)).Times(1).InSequence(seq);
        this->processAllEvents();
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, positiveMouseWheelEventTriggersMouseWheelUpEvent)
    {
        this->sendMouseWheelEvent(1);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_WheelUp, _, _)).Times(1);
        this->processAllEvents();
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, negativeMouseWheelEventTriggersMouseWheelDownEvent)
    {
        this->sendMouseWheelEvent(-1);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_WheelDown, _, _)).Times(1);
        this->processAllEvents();
    }

    TYPED_TEST_P(AWindowWaylandWithEventHandling, mouseMoveEventTriggersMouseMoveEvent)
    {
        this->sendMouseMoveEvent(5);

        EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, 5, 0)).Times(1);
        this->processAllEvents();
    }

    REGISTER_TYPED_TEST_CASE_P(AWindowWaylandWithEventHandling,
            singleKeyPressEventTriggersKeyPressedEventWithCorrectKeyCode,
            leftMouseButtonDownEventTriggersLeftButtonDownEvent,
            leftMouseButtonUpEventTriggersLeftButtonUpEvent,
            rightMouseButtonDownEventTriggersRightButtonDownEvent,
            rightMouseButtonUpEventTriggersRightButtonUpEvent,
            middleMouseButtonDownEventTriggersMiddleButtonDownEvent,
            middleMouseButtonUpEventTriggersMiddleButtonUpEvent,
            positiveMouseWheelEventTriggersMouseWheelUpEvent,
            negativeMouseWheelEventTriggersMouseWheelDownEvent,
            mouseMoveEventTriggersMouseMoveEvent
            );

    TYPED_TEST_CASE_P(AWindowWayland);

    TYPED_TEST_P(AWindowWayland, canInitAWindow)
    {
        EXPECT_TRUE(this->m_window->init());
    }

    TYPED_TEST_P(AWindowWayland, IfXdgRuntimeDirIsNotSetInitWillFail)
    {
        WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::XDGRuntimeDir);
        EXPECT_FALSE(this->m_window->init());
    }

    TYPED_TEST_P(AWindowWayland, IfXdgRuntimeDirIsNotCorrectInitWillFail)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, "/this/should/lead/nowhere");
        EXPECT_FALSE(this->m_window->init());
    }

    TYPED_TEST_P(AWindowWayland, IfXdgRuntimeDirIsNotSetButWaylandSocketInitWillSucceed)
    {
        UnixDomainSocket socket = UnixDomainSocket("wayland-0", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));

        WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::XDGRuntimeDir);

        String fileDescriptor = StringOutputStream::ToString(socket.createConnectedFileDescriptor(true));
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.c_str());

        EXPECT_TRUE(this->m_window->init());
    }

    TYPED_TEST_P(AWindowWayland, IfWaylandDisplayIsNotCorrectInitWillFail)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, "SomeFilenameThatDoesNotExist");
        EXPECT_FALSE(this->m_window->init());
    }

    TYPED_TEST_P(AWindowWayland, IfXdgRuntimeDirIsSetAndWaylandSocketIsSetInitWillSucceed)
    {
        UnixDomainSocket socket = UnixDomainSocket("wayland-0", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));

        String fileDescriptor = StringOutputStream::ToString(socket.createConnectedFileDescriptor(true));
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.c_str());

        EXPECT_TRUE(this->m_window->init());
    }

    TYPED_TEST_P(AWindowWayland, IfWaylandSocketIsWrongInitWillFail)
    {
        UnixDomainSocket socket = UnixDomainSocket("wayland-0", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));


        // just set the environment variable to some value, without ever creating the
        // socket
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, "33");
        WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::XDGRuntimeDir);

        EXPECT_FALSE(this->m_window->init());
    }

    TYPED_TEST_P(AWindowWayland, IfWaylandSocketIsSetAndWaylandDisplayIsSetInitWillFail)
    {
        UnixDomainSocket socket = UnixDomainSocket("wayland-0", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));

        String fileDescriptor = StringOutputStream::ToString(socket.createConnectedFileDescriptor(false));
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.c_str());
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, "wayland-0");

        EXPECT_FALSE(this->m_window->init());
    }

    REGISTER_TYPED_TEST_CASE_P(AWindowWayland,
            canInitAWindow,
            IfXdgRuntimeDirIsNotSetInitWillFail,
            IfXdgRuntimeDirIsNotCorrectInitWillFail,
            IfXdgRuntimeDirIsNotSetButWaylandSocketInitWillSucceed,
            IfWaylandDisplayIsNotCorrectInitWillFail,
            IfXdgRuntimeDirIsSetAndWaylandSocketIsSetInitWillSucceed,
            IfWaylandSocketIsWrongInitWillFail,
            IfWaylandSocketIsSetAndWaylandDisplayIsSetInitWillFail
            );
}

#endif
