//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_AWINDOWWAYLANDWITHEVENTHANDLING_H
#define RAMSES_AWINDOWWAYLANDWITHEVENTHANDLING_H

#include "AWindowWayland.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EKeyModifier.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "WindowEventsPollingManager_Wayland/WindowEventsPollingManager_Wayland.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include "wayland-egl.h"
#include <EGL/egl.h>

using namespace testing;

#define EV_RELEASED 0
#define EV_PRESSED  1

namespace ramses_internal
{
    template <typename WINDOWTYPE>
    class AWindowWaylandWithEventHandling : public AWindowWayland<WINDOWTYPE>
    {
    public:
        AWindowWaylandWithEventHandling() {}

        virtual void SetUp()
        {
            openInputDevices();
            setupWaylandWindow();
            initOpenGLContextAndSwapBuffers();

            // move mouse to get mouse focus (this is a workaround for a winston bug)
            // also ensure that the mouse position does not leave the window area by going to screen origin
            sendMouseMoveEvent(-9999, -9999);
            // Mouse click needed to get keyboard input focus.
            sendMouseButtonEvent(BTN_LEFT, true);
            sendMouseButtonEvent(BTN_LEFT, false);
            EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_Move, _, _)).Times(AnyNumber());
            EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonDown, _, _)).Times(AnyNumber());
            EXPECT_CALL(this->m_eventHandlerMock, onMouseEvent(EMouseEventType_LeftButtonUp, _, _)).Times(AnyNumber());
            processAllEvents();
        }

        virtual void TearDown()
        {
            closeOpenGLContext();
            m_windowEventsPollingManager.removeWindow(this->m_window);
            this->destroyWaylandWindow();
            closeInputDevice(keyboardFd);
            closeInputDevice(mouseFd);
        }

        void openInputDevices()
        {
            ASSERT_TRUE(openInputDevice("/dev/input/event1", keyboardFd));
            ASSERT_TRUE(openInputDevice("/dev/input/event2", mouseFd));
        }

        void setupWaylandWindow()
        {
            this->createWaylandWindow();

            ASSERT_TRUE(this->m_window->init());
            m_windowEventsPollingManager.addWindow(this->m_window);

            makeWindowVisible();

            // process window creation related events
            EXPECT_CALL(this->m_eventHandlerMock, onFocusChange(_)).Times(AnyNumber());
            EXPECT_CALL(this->m_eventHandlerMock, onResize(_, _)).Times(AnyNumber());
            EXPECT_CALL(this->m_eventHandlerMock, onMove(_)).Times(AnyNumber());
            processAllEvents();
        }

        void initOpenGLContextAndSwapBuffers()
        {
            // to process and send mouse events correcty to our surface the winston compositor need the final
            // surface size..
            // .. which is commited by an eglSwapBuffers call, so we need an opengl context here :(
            eglDisplay = eglGetDisplay(this->m_window->getNativeDisplayHandle());
            ASSERT_TRUE(eglDisplay != EGL_NO_DISPLAY);

            EGLint iMajorVersion;
            EGLint iMinorVersion;
            ASSERT_TRUE(eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion));
            ASSERT_TRUE(eglBindAPI(EGL_OPENGL_ES_API) != EGL_FALSE);

            EGLint surfaceAttributes[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE};

            EGLConfig eglConfig;
            EGLint    num_config;
            ASSERT_TRUE(eglChooseConfig(eglDisplay, surfaceAttributes, &eglConfig, 1, &num_config));

            ASSERT_TRUE(num_config == 1);


            eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, this->m_window->getNativeWindowHandle(), NULL);
            ASSERT_TRUE(eglSurface != EGL_NO_SURFACE);

            eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, 0);
            ASSERT_TRUE(eglContext != EGL_NO_CONTEXT);

            ASSERT_TRUE(eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) == EGL_TRUE);

            // do a single swapBuffers so the winston compositor knowns the final surface size and
            // is able to compare the mouse cursor position against the surface area
            ASSERT_TRUE(eglSwapBuffers(eglDisplay, eglSurface) == EGL_TRUE);
        }


        void closeOpenGLContext()
        {
            if (eglDisplay != EGL_NO_DISPLAY)
            {
                eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

                if (eglSurface != EGL_NO_SURFACE)
                {
                    eglDestroySurface(eglDisplay, eglSurface);
                }

                if (eglContext != EGL_NO_CONTEXT)
                {
                    eglDestroyContext(eglDisplay, eglContext);
                }

                eglTerminate(eglDisplay);
            }
        }

        Bool openInputDevice(const char* deviceName, int& deviceFd)
        {
            deviceFd = open(deviceName, O_WRONLY);
            return (deviceFd >= 0);
        }

        void closeInputDevice(int& deviceFd)
        {
            close(deviceFd);
            deviceFd = -1;
        }

        Bool sendInputEvent(int inputFd, UInt16 type, UInt16 key, Int32 value)
        {
            struct input_event event;
            memset(&event, 0, sizeof(event));

            event.type                = type;
            event.code                = key;
            event.value               = value;
            const UInt32 writtenBytes = write(inputFd, &event, sizeof(event));
            return (writtenBytes == sizeof(event));
        }

        void sendSyncEvent(int inputFd)
        {
            sendInputEvent(inputFd, EV_SYN, SYN_REPORT, 0);
            sync(); // enforce writing events to disc otherwise they may not be gathered by the system compositor
        }

        void sendKeyEvent(UInt32 key)
        {
            sendInputEvent(keyboardFd, EV_KEY, key, EV_PRESSED);
            sendInputEvent(keyboardFd, EV_KEY, key, EV_RELEASED);
            sendSyncEvent(keyboardFd);
        }

        void sendMouseButtonEvent(UInt32 key, Bool pressed)
        {
            sendInputEvent(mouseFd, EV_KEY, key, pressed ? EV_PRESSED : EV_RELEASED);
            sendSyncEvent(mouseFd);
        }

        void sendMouseMoveEvent(UInt32 numPixelsToMoveX, UInt32 numPixelsToMoveY = 0)
        {
            sendInputEvent(mouseFd, EV_REL, REL_X, numPixelsToMoveX);
            sendInputEvent(mouseFd, EV_REL, REL_Y, numPixelsToMoveY);
            sendSyncEvent(mouseFd);
        }

        void sendMouseWheelEvent(UInt32 wheelDelta)
        {
            sendInputEvent(mouseFd, EV_REL, REL_WHEEL, wheelDelta);
            sendSyncEvent(mouseFd);
        }

        void processAllEvents()
        {
            PlatformThread::Sleep(20); // give some time for event handling inside os / compositor
            for (UInt32 i = 0; i < 10;
                 i++) // enforce handling of all enqueued m_window events which will trigger our event handler
            {
                this->m_windowEventsPollingManager.pollWindowsTillAnyCanRender();
                this->m_window->handleEvents();
            }
        }

        void testKeyCode(UInt32       virtualKeyCode,
                         EKeyCode     expectedRamsesKeyCode,
                         EKeyModifier expectedModifier = EKeyModifier_NoModifier)
        {
            sendKeyEvent(virtualKeyCode);
            EXPECT_CALL(this->m_eventHandlerMock, onKeyEvent(EKeyEventType_Pressed, expectedModifier, expectedRamsesKeyCode))
                .Times(1);
            EXPECT_CALL(this->m_eventHandlerMock,
                        onKeyEvent(EKeyEventType_Released, EKeyModifier_NoModifier, expectedRamsesKeyCode))
                .Times(1);
            processAllEvents();
        }

        void makeWindowVisible()
        {
            // WINDOWTYPE specific code for making the window visible can go on here
            // in template specification
        }

    private:

        int keyboardFd = -1;
        int mouseFd    = -1;

        EGLDisplay eglDisplay = EGL_NO_DISPLAY;
        EGLSurface eglSurface = EGL_NO_SURFACE;
        EGLContext eglContext = EGL_NO_CONTEXT;

        WindowEventsPollingManager_Wayland m_windowEventsPollingManager {std::chrono::microseconds{10000u}};
    };
}

#endif
