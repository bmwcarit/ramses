//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREREVENTTESTHANDLER_H
#define RAMSES_RENDEREREVENTTESTHANDLER_H

#include "ramses-renderer-api/IRendererEventHandler.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/Vector.h"
#include <algorithm>

enum ERendererEventTestType
{
    ERendererEventTestType_Undefined = 0,

    ERendererEventTestType_OffscreenBufferCreated,
    ERendererEventTestType_OffscreenBufferDestroyed,
    ERendererEventTestType_PixelsRead,
    ERendererEventTestType_WarpingUpdated,
    ERendererEventTestType_DisplayCreated,
    ERendererEventTestType_DisplayDestroyed,
    ERendererEventTestType_WindowClosed,
    ERendererEventTestType_WindowResized,
    ERendererEventTestType_WindowMoved,
    ERendererEventTestType_KeyEvent,
    ERendererEventTestType_MouseEvent,
    ERendererEventTestType_RenderThreadPeriodicLoopTimes,
    ERendererEventTestType_ExternalBufferCreated,
    ERendererEventTestType_ExternalBufferDestroyed,
};

struct RendererTestEvent
{
    RendererTestEvent()
    {
    }

    bool operator==(const RendererTestEvent& other) const
    {
        return eventType == other.eventType
            && result == other.result
            && sceneId == other.sceneId
            && displayId == other.displayId
            && bufferId == other.bufferId
            && keyEvent == other.keyEvent
            && keyModifiers == other.keyModifiers
            && keyCode == other.keyCode
            && mouseEvent == other.mouseEvent
            && mousePosX == other.mousePosX
            && mousePosY == other.mousePosY
            && windowWidth == other.windowWidth
            && windowHeight == other.windowHeight
            && externalBufferId == other.externalBufferId
            && externalBufferTextureGlId == other.externalBufferTextureGlId;
    }

    ERendererEventTestType eventType            = ERendererEventTestType_Undefined;
    ramses::ERendererEventResult result         = ramses::ERendererEventResult_FAIL;

    ramses::sceneId_t sceneId                   { 0u };
    ramses::displayId_t displayId;
    ramses::displayBufferId_t bufferId;

    ramses::EKeyEvent keyEvent                  = ramses::EKeyEvent_Invalid;
    uint32_t keyModifiers                       = 0u;
    ramses::EKeyCode keyCode                    = ramses::EKeyCode_Unknown;

    ramses::EMouseEvent mouseEvent              = ramses::EMouseEvent_Invalid;
    int32_t mousePosX                           = 0;
    int32_t mousePosY                           = 0;
    uint32_t windowWidth                        = 0u;
    uint32_t windowHeight                       = 0u;
    int32_t windowPosX                          = 0;
    int32_t windowPosY                          = 0;

    std::chrono::microseconds renderthread_maximumLoopTime{0};
    std::chrono::microseconds renderthread_avg_looptime{0};

    ramses::externalBufferId_t externalBufferId;
    uint32_t externalBufferTextureGlId          { 0u };
};

class RendererEventTestHandler : public ramses::IRendererEventHandler
{
public:
    ~RendererEventTestHandler() override
    {
        EXPECT_TRUE(m_events.empty());
    }

    virtual void offscreenBufferCreated(ramses::displayId_t displayId, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_OffscreenBufferCreated;
        event.displayId = displayId;
        event.bufferId = offscreenBufferId;
        event.result = result;
        m_events.push_back(event);
    }

    virtual void offscreenBufferDestroyed(ramses::displayId_t displayId, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_OffscreenBufferDestroyed;
        event.displayId = displayId;
        event.bufferId = offscreenBufferId;
        event.result = result;
        m_events.push_back(event);
    }

    virtual void framebufferPixelsRead(const uint8_t* /*pixelData*/, const uint32_t /*pixelDataSize*/, ramses::displayId_t displayId, ramses::displayBufferId_t displayBufferId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_PixelsRead;
        event.result = result;
        event.displayId = displayId;
        event.bufferId = displayBufferId;
        m_events.push_back(event);
    }

    virtual void warpingMeshDataUpdated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WarpingUpdated;
        event.result = result;
        event.displayId = displayId;
        m_events.push_back(event);
    }

    virtual void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DisplayCreated;
        event.result = result;
        event.displayId = displayId;
        m_events.push_back(event);
    }

    virtual void displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DisplayDestroyed;
        event.result = result;
        event.displayId = displayId;
        m_events.push_back(event);
    }

    virtual void windowClosed(ramses::displayId_t displayId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WindowClosed;
        event.displayId = displayId;
        m_events.push_back(event);
    }

    virtual void windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WindowResized;
        event.displayId = displayId;
        event.windowWidth = width;
        event.windowHeight = height;
        m_events.push_back(event);
    }

    virtual void windowMoved(ramses::displayId_t displayId, int32_t posX, int32_t posY) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WindowMoved;
        event.displayId = displayId;
        event.windowPosX = posX;
        event.windowPosY = posY;
        m_events.push_back(event);
    }

    virtual void keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent keyEvent, uint32_t keyModifiers, ramses::EKeyCode keyCode) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_KeyEvent;
        event.displayId = displayId;
        event.keyEvent = keyEvent;
        event.keyModifiers = keyModifiers;
        event.keyCode = keyCode;
        m_events.push_back(event);
    }

    virtual void mouseEvent(ramses::displayId_t displayId, ramses::EMouseEvent mouseEvent, int32_t mousePosX, int32_t mousePosY) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_MouseEvent;
        event.mouseEvent = mouseEvent;
        event.displayId = displayId;
        event.mousePosX = mousePosX;
        event.mousePosY = mousePosY;
        m_events.push_back(event);
    }

#ifdef RAMSES_ENABLE_EXTERNAL_BUFFER_EVENTS
    virtual void externalBufferCreated(ramses::displayId_t displayId, ramses::externalBufferId_t externalBufferId, uint32_t textureGlId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_ExternalBufferCreated;
        event.displayId = displayId;
        event.externalBufferId = externalBufferId;
        event.externalBufferTextureGlId = textureGlId;
        event.result = result;
        m_events.push_back(event);
    }

    virtual void externalBufferDestroyed(ramses::displayId_t displayId, ramses::externalBufferId_t externalBufferId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_ExternalBufferDestroyed;
        event.displayId = displayId;
        event.externalBufferId = externalBufferId;
        event.result = result;
        m_events.push_back(event);
    }
#endif

    void expectOffscreenBufferCreated(ramses::displayId_t displayId, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_OffscreenBufferCreated;
        event.displayId = displayId;
        event.bufferId = offscreenBufferId;
        event.result = result;
        expectEvent(event);
    }

    void expectOffscreenBufferDestroyed(ramses::displayId_t displayId, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_OffscreenBufferDestroyed;
        event.displayId = displayId;
        event.bufferId = offscreenBufferId;
        event.result = result;
        expectEvent(event);
    }

    void expectExternalBufferCreated(ramses::displayId_t displayId, ramses::externalBufferId_t externalBufferId, uint32_t textureGlId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_ExternalBufferCreated;
        event.displayId = displayId;
        event.externalBufferId = externalBufferId;
        event.externalBufferTextureGlId = textureGlId;
        event.result = result;
        expectEvent(event);
    }

    void expectExternalBufferDestroyed(ramses::displayId_t displayId, ramses::externalBufferId_t externalBufferId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_ExternalBufferDestroyed;
        event.displayId = displayId;
        event.externalBufferId = externalBufferId;
        event.result = result;
        expectEvent(event);
    }

    void expectFramebufferPixelsRead(const uint8_t* /*pixelData*/, const uint32_t /*pixelDataSize*/, ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_PixelsRead;
        event.result = result;
        event.displayId = displayId;
        expectEvent(event);
    }

    void expectWarpingMeshDataUpdated(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WarpingUpdated;
        event.result = result;
        event.displayId = displayId;
        expectEvent(event);
    }

    void expectDisplayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DisplayCreated;
        event.result = result;
        event.displayId = displayId;
        expectEvent(event);
    }

    void expectDisplayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DisplayDestroyed;
        event.result = result;
        event.displayId = displayId;
        expectEvent(event);
    }

    void expectWindowClosed(ramses::displayId_t displayId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WindowClosed;
        event.displayId = displayId;
        expectEvent(event);
    }

    void expectWindowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WindowResized;
        event.displayId = displayId;
        event.windowWidth = width;
        event.windowHeight = height;
        expectEvent(event);
    }

    void expectWindowMoved(ramses::displayId_t displayId, int32_t posX, int32_t posY)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_WindowMoved;
        event.displayId = displayId;
        event.windowPosX = posX;
        event.windowPosY = posY;
        expectEvent(event);
    }

    void expectKeyEvent(ramses::displayId_t displayId, ramses::EKeyEvent keyEvent, uint32_t keyModifiers, ramses::EKeyCode keyCode)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_KeyEvent;
        event.displayId = displayId;
        event.keyEvent = keyEvent;
        event.keyModifiers = keyModifiers;
        event.keyCode = keyCode;
        expectEvent(event);
    }

    void expectMouseEvent(ramses::displayId_t displayId, ramses::EMouseEvent mouseEvent, int32_t mousePosX, int32_t mousePosY)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_MouseEvent;
        event.displayId = displayId;
        event.mouseEvent = mouseEvent;
        event.mousePosX = mousePosX;
        event.mousePosY = mousePosY;
        expectEvent(event);
    }

    virtual void renderThreadLoopTimings(std::chrono::microseconds maximumLoopTimeMilliseconds, std::chrono::microseconds averageLooptimeMilliseconds) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_RenderThreadPeriodicLoopTimes;
        event.renderthread_maximumLoopTime = maximumLoopTimeMilliseconds;
        event.renderthread_avg_looptime = averageLooptimeMilliseconds;
        expectEvent(event);
    }

    void expectNoEvent()
    {
        EXPECT_TRUE(m_events.empty());
    }

private:
    void expectEvent(const RendererTestEvent& event, const ramses_internal::UInt withinLast = 1u)
    {
        auto it = std::find(m_events.begin(), m_events.begin() + std::min(m_events.size(), withinLast), event);
        EXPECT_TRUE(it != m_events.end()) << "expected event " << event.eventType << " not emitted";

        if (it != m_events.end())
            m_events.erase(it);
    }

    std::vector<RendererTestEvent> m_events;
};

#endif
