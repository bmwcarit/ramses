//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "DisplayManager/IDisplayManager.h"
#include <unordered_set>

using namespace ramses;
using namespace testing;

class RendererEventTracker final : public RendererEventHandlerEmpty
{
public:
    explicit RendererEventTracker(sceneId_t sceneId)
        : m_sceneId(sceneId) {}
    virtual void displayCreated(displayId_t displayId, ERendererEventResult result) override
    {
        if (result == ERendererEventResult_OK)
            m_displaysCreated.insert(displayId);
    }
    virtual void displayDestroyed(displayId_t displayId, ERendererEventResult result) override
    {
        if (result == ERendererEventResult_OK)
            m_displaysCreated.erase(displayId);
    }
    virtual void scenePublished(sceneId_t sceneId) override
    {
        if (sceneId == m_sceneId)
            m_lastState = ramses_display_manager::SceneState::Unavailable;
    }
    virtual void sceneSubscribed(sceneId_t sceneId, ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ERendererEventResult_FAIL)
            m_lastState = ramses_display_manager::SceneState::Available;
    }
    virtual void sceneMapped(sceneId_t sceneId, ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ERendererEventResult_FAIL)
            m_lastState = ramses_display_manager::SceneState::Ready;
    }
    virtual void sceneShown(sceneId_t sceneId, ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ERendererEventResult_FAIL)
            m_lastState = ramses_display_manager::SceneState::Rendered;
    }
    virtual void sceneUnpublished(sceneId_t sceneId) override
    {
        if (sceneId == m_sceneId)
            m_lastState = ramses_display_manager::SceneState::Unavailable;
    }
    virtual void sceneUnsubscribed(sceneId_t sceneId, ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ERendererEventResult_FAIL)
            m_lastState = ramses_display_manager::SceneState::Unavailable;
    }
    virtual void sceneUnmapped(sceneId_t sceneId, ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ERendererEventResult_FAIL)
            m_lastState = ramses_display_manager::SceneState::Available;
    }
    virtual void sceneHidden(sceneId_t sceneId, ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ERendererEventResult_FAIL)
            m_lastState = ramses_display_manager::SceneState::Ready;
    }

    const sceneId_t m_sceneId;
    ramses_display_manager::SceneState m_lastState = ramses_display_manager::SceneState::Unavailable;
    std::unordered_set<displayId_t> m_displaysCreated;
};

