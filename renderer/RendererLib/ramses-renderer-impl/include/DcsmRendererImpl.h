//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMRENDERERIMPL_H
#define RAMSES_DCSMRENDERERIMPL_H

#include "StatusObjectImpl.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-renderer-api/DcsmRenderer.h"
#include "ramses-renderer-api/DcsmRendererConfig.h"
#include "DisplayManager/IDisplayManager.h"
#include "ContentStates.h"
#include "SharedSceneState.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace ramses
{
    class RamsesRenderer;
    class RamsesFramework;
    class IDcsmConsumerImpl;
    class IRendererEventHandler;
    class IDcsmRendererEventHandler;

    class DcsmRendererImpl : public StatusObjectImpl, public IDcsmConsumerEventHandler, public ramses_display_manager::IEventHandler
    {
    public:
        DcsmRendererImpl(const DcsmRendererConfig& config, IDcsmConsumerImpl& dcsmConsumer, std::unique_ptr<ramses_display_manager::IDisplayManager> displayManager);

        status_t requestContentReady(ContentID contentID, uint64_t timeOut);
        status_t requestContentReadyAndLinkedViaOffscreenBuffer(ContentID contentID, uint32_t width, uint32_t height, ContentID consumerContentID, dataConsumerId_t consumerDataId, uint64_t timeOut);
        status_t showContent(ContentID contentID, AnimationInformation timingInfo);
        status_t hideContent(ContentID contentID, AnimationInformation timingInfo);
        status_t releaseContent(ContentID contentID, AnimationInformation timingInfo);
        status_t setCategorySize(Category categoryId, SizeInfo size, AnimationInformation timingInfo);
        status_t acceptStopOffer(ContentID contentID, AnimationInformation timingInfo);
        status_t update(uint64_t timeStampNow, IDcsmRendererEventHandler& eventHandler, IRendererEventHandler* customRendererEventHandler);

    private:
        // IDcsmConsumerEventHandler
        virtual void contentOffered(ramses::ContentID contentID, ramses::Category category) override;
        virtual void contentReady(ramses::ContentID contentID, ramses::ETechnicalContentType contentType, ramses::TechnicalContentDescriptor contentDescriptor) override;
        virtual void contentFocusRequest(ramses::ContentID contentID) override;
        virtual void contentStopOfferRequest(ramses::ContentID contentID) override;
        virtual void forceContentOfferStopped(ramses::ContentID contentID) override;

        // ramses_display_manager::IEventHandler
        virtual void sceneStateChanged(ramses::sceneId_t sceneId, ramses_display_manager::SceneState state, ramses::displayId_t displaySceneIsMappedTo) override;

        void executePendingCommands(uint64_t timeStampNow);
        void dispatchPendingEvents(IDcsmRendererEventHandler& eventHandler);

        void requestSceneState(ContentID contentID, ramses_display_manager::SceneState state);
        void handleContentStateChange(ContentID contentID, ContentState lastState);
        void removeContent(ContentID contentID);
        void scheduleSceneStateChange(ContentID contentThatTrigerredChange, ramses_display_manager::SceneState sceneState, uint64_t ts);
        ContentState getCurrentState(ContentID contentID) const;

        std::unique_ptr<ramses_display_manager::IDisplayManager> m_displayManager;
        IDcsmConsumerImpl& m_dcsmConsumer;

        struct CategoryInfo
        {
            SizeInfo size;
            displayId_t display = InvalidDisplayId;
            std::unordered_set<ContentID> assignedContentIds;
        };
        std::unordered_map<Category, CategoryInfo> m_categories;

        struct ContentInfo
        {
            Category category;

            bool readyRequested = false;
            bool dcsmReady = false;

            uint32_t obWidth = 0;
            uint32_t obHeight = 0;
            sceneId_t consumerSceneID{ 0 };
            dataConsumerId_t consumerDataID{ 0 };
        };
        std::unordered_map<ContentID, ContentInfo> m_contents;

        struct SceneInfo
        {
            SharedSceneState sharedState;
            std::unordered_set<ContentID> associatedContents;
        };
        std::unordered_map<sceneId_t, SceneInfo> m_scenes;

        enum class CommandType
        {
            SceneStateChange,
            RemoveContent
        };

        struct Command
        {
            CommandType type;
            uint64_t timePoint;

            ContentID contentId{ 0 };
            ramses_display_manager::SceneState sceneState = ramses_display_manager::SceneState::Unavailable;
        };
        std::vector<Command> m_pendingCommands;

        enum class EventType
        {
            ContentStateChanged,
            ContentFocusRequested,
            ContentStopOfferRequested,
            ContentNotAvailable
        };

        struct Event
        {
            EventType type;
            ContentID contentID;
            Category category;
            ContentState contentState;
            ContentState previousState;
        };
        std::vector<Event> m_pendingEvents;
        std::vector<Event> m_pendingEventsTmp; // to avoid allocs
    };
}

#endif
