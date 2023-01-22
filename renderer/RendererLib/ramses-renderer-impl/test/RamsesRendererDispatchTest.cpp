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

    // ugly hack to replace platform with mock via overriding a static factory helper
    NiceMock<PlatformNiceMock>* gPlatformMock = nullptr;
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig&)
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
            m_renderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
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

        ramses::displayId_t createDisplayAndExpectResult()
        {
            const ramses::displayId_t displayId = addDisplay();
            updateAndDispatch(m_handler);
            m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

            return displayId;
        }

        void doUpdateLoop()
        {
            m_renderer.doOneLoop();
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
        constexpr ramses::displayId_t displayId{ 123u };
        RendererEvent evt{ ramses_internal::ERendererEventType::DisplayCreateFailed };
        evt.displayHandle = DisplayHandle{ displayId.getValue() };
        m_renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(evt));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult::ERendererEventResult_FAIL);
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

#ifdef RAMSES_ENABLE_EXTERNAL_BUFFER_EVENTS
    TEST_F(ARamsesRendererDispatch, generatesEventForExternalBufferCreation)
    {
        ramses::displayId_t displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const ramses::externalBufferId_t bufferId = m_renderer.createExternalBuffer(displayId);
        updateAndDispatch(m_handler);
        m_handler.expectExternalBufferCreated(displayId, bufferId, DeviceMock::FakeExternalTextureGlId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForExternalBufferCreation)
    {
        ramses::displayId_t displayId(0u);
        const ramses::externalBufferId_t bufferId = m_renderer.createExternalBuffer(displayId);
        updateAndDispatch(m_handler);
        m_handler.expectExternalBufferCreated(displayId, bufferId, 0u, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForExternalBufferDestruction)
    {
        ramses::displayId_t displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const ramses::externalBufferId_t bufferId = m_renderer.createExternalBuffer(displayId);
        updateAndDispatch(m_handler);
        m_handler.expectExternalBufferCreated(displayId, bufferId, DeviceMock::FakeExternalTextureGlId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyExternalBuffer(displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectExternalBufferDestroyed(displayId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForExternalBufferDestruction)
    {
        ramses::displayId_t displayId(0u);
        const ramses::externalBufferId_t bufferId(0u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyExternalBuffer(displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectExternalBufferDestroyed(displayId, bufferId, ramses::ERendererEventResult_FAIL);
    }
#endif

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowClosed)
    {
        ramses_internal::RendererEvent evt{ ramses_internal::ERendererEventType::WindowClosed };
        evt.displayHandle = DisplayHandle{ 2u };
        m_renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(evt));
        updateAndDispatch(m_handler);
        m_handler.expectWindowClosed(ramses::displayId_t{ 2u });
    }
    TEST_F(ARamsesRendererDispatch, generatesEventForWindowResized)
    {
        ramses_internal::RendererEvent evt{ ramses_internal::ERendererEventType::WindowResizeEvent };
        evt.displayHandle = DisplayHandle{ 2u };
        evt.resizeEvent.width = 100;
        evt.resizeEvent.height = 200;
        m_renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(evt));
        updateAndDispatch(m_handler);
        m_handler.expectWindowResized(ramses::displayId_t{ 2u }, 100, 200);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowMoved)
    {
        ramses_internal::RendererEvent evt{ ramses_internal::ERendererEventType::WindowMoveEvent };
        evt.displayHandle = DisplayHandle{ 2u };
        evt.moveEvent.posX = 100;
        evt.moveEvent.posY = 200;
        m_renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(evt));
        updateAndDispatch(m_handler);
        m_handler.expectWindowMoved(ramses::displayId_t{ 2u }, 100, 200);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyPressed)
    {
        ramses_internal::RendererEvent evt{ ramses_internal::ERendererEventType::WindowKeyEvent };
        evt.displayHandle = DisplayHandle{ 2u };
        evt.keyEvent.type = ramses_internal::EKeyEventType_Pressed;
        evt.keyEvent.modifier = ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift;
        evt.keyEvent.keyCode = ramses_internal::EKeyCode_E;
        m_renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(evt));
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(ramses::displayId_t{ 2u }, ramses::EKeyEvent_Pressed, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_E);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyReleased)
    {
        ramses_internal::RendererEvent evt{ ramses_internal::ERendererEventType::WindowKeyEvent };
        evt.displayHandle = DisplayHandle{ 2u };
        evt.keyEvent.type = ramses_internal::EKeyEventType_Released;
        evt.keyEvent.modifier = ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift;
        evt.keyEvent.keyCode = ramses_internal::EKeyCode_F;
        m_renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(evt));
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(ramses::displayId_t{ 2u }, ramses::EKeyEvent_Released, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_F);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventsForMouseActionsOnDisplay)
    {
        ramses_internal::RendererEvent evt{ ramses_internal::ERendererEventType::WindowMouseEvent };
        evt.displayHandle = DisplayHandle{ 2u };
        evt.mouseEvent.type = ramses_internal::EMouseEventType_RightButtonDown;
        evt.mouseEvent.pos = { 20, 30 };
        m_renderer.impl.getDisplayDispatcher().injectRendererEvent(std::move(evt));
        updateAndDispatch(m_handler);
        m_handler.expectMouseEvent(ramses::displayId_t{ 2u }, ramses::EMouseEvent_RightButtonDown, 20, 30);
    }
}
