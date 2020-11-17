//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/WarpingMeshData.h"

#include "RamsesFrameworkImpl.h"
#include "RamsesRendererImpl.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/DisplayEventHandler.h"
#include "RendererLib/EKeyModifier.h"
#include "Scene/ClientScene.h"
#include "RendererEventTestHandler.h"
#include "RamsesRendererUtils.h"

//This is needed to abstract from a specific rendering platform
#include "PlatformMock.h"
#include "Platform_Base/Platform_Base.h"

namespace ramses_internal
{
    using namespace testing;

    NiceMock<PlatformNiceMock>* gPlatformMock = nullptr;

    ramses_internal::IPlatform* ramses_internal::Platform_Base::CreatePlatform(const ramses_internal::RendererConfig&)
    {
        gPlatformMock = new ::testing::NiceMock<PlatformNiceMock>();
        return gPlatformMock;
    }

    class ARamsesRendererDispatch : public ::testing::Test
    {
    protected:
        ARamsesRendererDispatch()
            : m_framework()
            , m_renderer(*m_framework.createRenderer(ramses::RendererConfig()))
        {
        }

        ramses::displayId_t addDisplay(bool warpingEnabled = false)
        {
            //Create a display
            ramses::DisplayConfig displayConfig;
            if (warpingEnabled)
            {
                displayConfig.enableWarpingPostEffect();
            }
            EXPECT_EQ(ramses::StatusOK, displayConfig.validate());
            return m_renderer.createDisplay(displayConfig);
        }

        ramses::displayId_t createDisplayAndExpectResult(ramses::ERendererEventResult expectedResult = ramses::ERendererEventResult_OK)
        {
            const ramses::displayId_t displayId = addDisplay();
            updateAndDispatch(m_handler);
            m_handler.expectDisplayCreated(displayId, expectedResult);

            return displayId;
        }

        void doUpdateLoop()
        {
            m_renderer.impl.getRenderer().doOneLoop(ramses_internal::ELoopMode::UpdateOnly);
        }

        void updateAndDispatch(RendererEventTestHandler& eventHandler, uint32_t loops = 1u)
        {
            EXPECT_EQ(ramses::StatusOK, m_renderer.flush());
            for (uint32_t i = 0u; i < loops; ++i)
                doUpdateLoop();
            EXPECT_EQ(ramses::StatusOK, m_renderer.dispatchEvents(eventHandler));
        }

    protected:
        ramses::RamsesFramework m_framework;
        ramses::RamsesRenderer& m_renderer;
        RendererEventTestHandler m_handler;
    };

    TEST_F(ARamsesRendererDispatch, generatesOKEventForDisplayCreation)
    {
        createDisplayAndExpectResult();
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForDisplayCreation)
    {
        ON_CALL(*gPlatformMock, createRenderBackend(_, _)).WillByDefault(Return(static_cast<ramses_internal::IRenderBackend*>(nullptr)));
        createDisplayAndExpectResult(ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForDisplayDestruction)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(displayId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForInvalidDisplayDestruction)
    {
        const ramses::displayId_t displayId(1u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(displayId));

        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWarpingMeshDataUpdate)
    {
        ramses::displayId_t displayId = addDisplay(true);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(displayId, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(displayId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForWarpingMeshDataUpdateOfInvalidDisplay)
    {
        const ramses::displayId_t invalidDisplay(1u);
        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(invalidDisplay, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(invalidDisplay, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWarpingMeshDataUpdateOfDisplayWithoutWarping)
    {
        ramses::displayId_t displayId = addDisplay(false);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(displayId, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForOffscreenBufferCreation)
    {
        ramses::displayId_t displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForOffscreenBufferCreation)
    {
        ramses::displayId_t displayId(0u);
        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForOffscreenBufferDestruction)
    {
        ramses::displayId_t displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyOffscreenBuffer(displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferDestroyed(displayId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForOffscreenBufferDestruction)
    {
        ramses::displayId_t displayId(0u);
        const ramses::displayBufferId_t bufferId(0u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyOffscreenBuffer(displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferDestroyed(displayId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowClosed)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onClose();
        updateAndDispatch(m_handler);
        m_handler.expectWindowClosed(displayId);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowResized)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onResize(100, 200);
        updateAndDispatch(m_handler);
        m_handler.expectWindowResized(displayId, 100, 200);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowMoved)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onWindowMove(100, 200);
        updateAndDispatch(m_handler);
        m_handler.expectWindowMoved(displayId, 100, 200);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyPressed)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onKeyEvent(ramses_internal::EKeyEventType_Pressed,
            ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift, ramses_internal::EKeyCode_E);
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(displayId, ramses::EKeyEvent_Pressed, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_E);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyReleased)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onKeyEvent(ramses_internal::EKeyEventType_Released,
            ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift, ramses_internal::EKeyCode_F);
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(displayId, ramses::EKeyEvent_Released, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_F);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventsForMouseActionsOnDisplay)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_RightButtonDown, 20, 30);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_RightButtonUp, 21, 31);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_LeftButtonDown, 22, 32);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_LeftButtonUp, 23, 33);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_MiddleButtonDown, 24, 34);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_MiddleButtonUp, 25, 35);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WheelDown, 26, 36);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WheelUp, 27, 37);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_Move, 28, 38);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WindowEnter, 29, 39);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WindowLeave, 30, 40);
        updateAndDispatch(m_handler);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_RightButtonDown, 20, 30);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_RightButtonUp, 21, 31);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_LeftButtonDown, 22, 32);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_LeftButtonUp, 23, 33);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_MiddleButtonDown, 24, 34);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_MiddleButtonUp, 25, 35);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WheelDown, 26, 36);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WheelUp, 27, 37);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_Move, 28, 38);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WindowEnter, 29, 39);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WindowLeave, 30, 40);
    }
}
