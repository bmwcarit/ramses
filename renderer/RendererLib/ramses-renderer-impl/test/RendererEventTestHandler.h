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
#include "ramses-renderer-api/IRendererSceneControlEventHandler_legacy.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/Vector.h"
#include <algorithm>

enum ERendererEventTestType
{
    ERendererEventTestType_Undefined = 0,

    ERendererEventTestType_ScenePublished,
    ERendererEventTestType_SceneSubscribed,
    ERendererEventTestType_SceneMapped,
    ERendererEventTestType_SceneShown,
    ERendererEventTestType_SceneUnpublished,
    ERendererEventTestType_SceneUnsubscribed,
    ERendererEventTestType_SceneUnmapped,
    ERendererEventTestType_SceneHidden,
    ERendererEventTestType_DataLinked,
    ERendererEventTestType_BufferLinked,
    ERendererEventTestType_DataUnlinked,
    ERendererEventTestType_OffscreenBufferCreated,
    ERendererEventTestType_OffscreenBufferDestroyed,
    ERendererEventTestType_SceneAssignedToDisplayBuffer,
    ERendererEventTestType_PixelsRead,
    ERendererEventTestType_WarpingUpdated,
    ERendererEventTestType_DisplayCreated,
    ERendererEventTestType_DisplayDestroyed,
    ERendererEventTestType_TransformationProviderCreated,
    ERendererEventTestType_TransformationProviderDestroyed,
    ERendererEventTestType_TransformationConsumerCreated,
    ERendererEventTestType_TransformationConsumerDestroyed,
    ERendererEventTestType_DataProviderCreated,
    ERendererEventTestType_DataProviderDestroyed,
    ERendererEventTestType_DataConsumerCreated,
    ERendererEventTestType_DataConsumerDestroyed,
    ERendererEventTestType_TextureProviderCreated,
    ERendererEventTestType_TextureProviderDestroyed,
    ERendererEventTestType_TextureConsumerCreated,
    ERendererEventTestType_TextureConsumerDestroyed,
    ERendererEventTestType_SceneFlushed,
    ERendererEventTestType_SceneExpired,
    ERendererEventTestType_SceneRecoveredAfterExpiration,
    ERendererEventTestType_WindowClosed,
    ERendererEventTestType_WindowResized,
    ERendererEventTestType_WindowMoved,
    ERendererEventTestType_KeyEvent,
    ERendererEventTestType_MouseEvent,
    ERendererEventTestType_StreamAvailabilityChanged,
    ERendererEventTestType_ObjectsPicked,
    ERendererEventTestType_RenderThreadPeriodicLoopTimes,
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
            && providerScene == other.providerScene
            && consumerScene == other.consumerScene
            && dataProviderId == other.dataProviderId
            && dataConsumerId == other.dataConsumerId
            && sceneVersionTag == other.sceneVersionTag
            && resourceStatus == other.resourceStatus
            && keyEvent == other.keyEvent
            && keyModifiers == other.keyModifiers
            && keyCode == other.keyCode
            && mouseEvent == other.mouseEvent
            && mousePosX == other.mousePosX
            && mousePosY == other.mousePosY
            && windowWidth == other.windowWidth
            && windowHeight == other.windowHeight
            && pickedObjects == other.pickedObjects;
    }

    ERendererEventTestType eventType            = ERendererEventTestType_Undefined;
    ramses::ERendererEventResult result         = ramses::ERendererEventResult_FAIL;

    ramses::sceneId_t sceneId                   { 0u };
    ramses::displayId_t displayId;
    ramses::displayBufferId_t bufferId;

    ramses::sceneId_t providerScene             { 0u };
    ramses::sceneId_t consumerScene             { 0u };
    ramses::dataProviderId_t dataProviderId     { 0u };
    ramses::dataConsumerId_t dataConsumerId     { 0u };

    ramses::sceneVersionTag_t sceneVersionTag   = ramses::InvalidSceneVersionTag;
    ramses::ESceneResourceStatus resourceStatus = ramses::ESceneResourceStatus_Pending;

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

    std::chrono::microseconds renderthread_maximumLoopTime;
    std::chrono::microseconds renderthread_avg_looptime;

    std::vector<ramses::pickableObjectId_t> pickedObjects;
};

class RendererEventTestHandler : public ramses::IRendererEventHandler, public ramses::IRendererSceneControlEventHandler_legacy
{
public:
    ~RendererEventTestHandler()
    {
        EXPECT_TRUE(m_events.empty());
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_ScenePublished;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneSubscribed;
        event.result = result;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneMapped;
        event.result = result;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneShown;
        event.result = result;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneUnpublished;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneUnsubscribed;
        event.result = result;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneUnmapped;
        event.result = result;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneHidden;
        event.result = result;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void dataLinked(ramses::sceneId_t providerScene, ramses::dataProviderId_t dataProviderId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataLinked;
        event.result = result;
        event.providerScene = providerScene;
        event.dataProviderId = dataProviderId;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        m_events.push_back(event);
    }

    virtual void offscreenBufferLinkedToSceneData(ramses::displayBufferId_t providerOffscreenBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_BufferLinked;
        event.result = result;
        event.bufferId = providerOffscreenBuffer;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        m_events.push_back(event);
    }

    virtual void dataUnlinked(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataUnlinked;
        event.result = result;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        m_events.push_back(event);
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

    virtual void sceneAssignedToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t displayBufferId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneAssignedToDisplayBuffer;
        event.sceneId = sceneId;
        event.bufferId = displayBufferId;
        event.result = result;
        m_events.push_back(event);
    }

    virtual void framebufferPixelsRead(const uint8_t* /*pixelData*/, const uint32_t /*pixelDataSize*/, ramses::displayId_t displayId, ramses::ERendererEventResult result) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_PixelsRead;
        event.result = result;
        event.displayId = displayId;
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

    virtual void dataProviderCreated(ramses::sceneId_t providerScene, ramses::dataProviderId_t dataProviderId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataProviderCreated;
        event.providerScene = providerScene;
        event.dataProviderId = dataProviderId;
        m_events.push_back(event);
    }

    virtual void dataProviderDestroyed(ramses::sceneId_t providerScene, ramses::dataProviderId_t dataProviderId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataProviderDestroyed;
        event.providerScene = providerScene;
        event.dataProviderId = dataProviderId;
        m_events.push_back(event);
    }

    virtual void dataConsumerCreated(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataConsumerCreated;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        m_events.push_back(event);
    }

    virtual void dataConsumerDestroyed(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataConsumerDestroyed;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        m_events.push_back(event);
    }

    virtual void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag, ramses::ESceneResourceStatus resourceStatus) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneFlushed;
        event.sceneId = sceneId;
        event.sceneVersionTag= sceneVersionTag;
        event.resourceStatus = resourceStatus;
        m_events.push_back(event);
    }

    virtual void sceneExpired(ramses::sceneId_t sceneId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneExpired;
        event.sceneId = sceneId;
        m_events.push_back(event);
    }

    virtual void sceneRecoveredFromExpiration(ramses::sceneId_t sceneId) override
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneRecoveredAfterExpiration;
        event.sceneId = sceneId;
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

    virtual void streamAvailabilityChanged(ramses::streamSource_t /*streamId*/, bool /*available*/) override
    {
        // only add event, content tested elsewhere, no need to test here too
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_StreamAvailabilityChanged;
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

    virtual void objectsPicked(ramses::sceneId_t sceneId, const ramses::pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override
    {
        RendererTestEvent event;
        event.sceneId = sceneId;
        event.pickedObjects.insert(event.pickedObjects.begin(), pickedObjects, pickedObjects + pickedObjectsCount);
        m_events.push_back(event);
    }

    void expectScenePublished(ramses::sceneId_t sceneId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_ScenePublished;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectSceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneSubscribed;
        event.result = result;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectSceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result, const uint32_t withinLast = 1u)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneMapped;
        event.result = result;
        event.sceneId = sceneId;
        expectEvent(event, withinLast);
    }

    void expectSceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneShown;
        event.result = result;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectSceneUnpublished(ramses::sceneId_t sceneId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneUnpublished;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectSceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneUnsubscribed;
        event.result = result;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectSceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneUnmapped;
        event.result = result;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectSceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneHidden;
        event.result = result;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectDataLinked(ramses::sceneId_t providerScene, ramses::dataProviderId_t dataProviderId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataLinked;
        event.result = result;
        event.providerScene = providerScene;
        event.dataProviderId = dataProviderId;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        expectEvent(event);
    }

    void expectOffscreenBufferLinkedToSceneData(ramses::displayBufferId_t providerOffscreenBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_BufferLinked;
        event.result = result;
        event.bufferId = providerOffscreenBuffer;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        expectEvent(event);
    }

    void expectDataUnlinked(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataUnlinked;
        event.result = result;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        expectEvent(event);
    }

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

    void expectSceneAssignedToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t displayBufferId, ramses::ERendererEventResult result)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneAssignedToDisplayBuffer;
        event.sceneId = sceneId;
        event.bufferId = displayBufferId;
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

    void expectDataProviderCreated(ramses::sceneId_t providerScene, ramses::dataProviderId_t dataProviderId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataProviderCreated;
        event.providerScene = providerScene;
        event.dataProviderId = dataProviderId;
        expectEvent(event);
    }

    void expectDataProviderDestroyed(ramses::sceneId_t providerScene, ramses::dataProviderId_t dataProviderId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataProviderDestroyed;
        event.providerScene = providerScene;
        event.dataProviderId = dataProviderId;
        expectEvent(event);
    }

    void expectDataConsumerCreated(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataConsumerCreated;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        expectEvent(event);
    }

    void expectDataConsumerDestroyed(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t dataConsumerId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_DataConsumerDestroyed;
        event.consumerScene = consumerScene;
        event.dataConsumerId = dataConsumerId;
        expectEvent(event);
    }

    void expectSceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag, ramses::ESceneResourceStatus resourceStatus, const uint32_t withinLast = 1u)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneFlushed;
        event.sceneId = sceneId;
        event.sceneVersionTag = sceneVersionTag;
        event.resourceStatus = resourceStatus;
        expectEvent(event, withinLast);
    }

    void expectSceneExpired(ramses::sceneId_t sceneId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneExpired;
        event.sceneId = sceneId;
        expectEvent(event);
    }

    void expectSceneRecoveredFromExpiration(ramses::sceneId_t sceneId)
    {
        RendererTestEvent event;
        event.eventType = ERendererEventTestType_SceneRecoveredAfterExpiration;
        event.sceneId = sceneId;
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

    void expectObjectsPicked(ramses::sceneId_t sceneId, std::initializer_list<ramses::pickableObjectId_t> pickedObjects)
    {
        RendererTestEvent event;
        event.sceneId = sceneId;
        event.pickedObjects.insert(event.pickedObjects.begin(), pickedObjects.begin(), pickedObjects.end());
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
