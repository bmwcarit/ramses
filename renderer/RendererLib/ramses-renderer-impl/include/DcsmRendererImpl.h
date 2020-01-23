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
#include "ramses-renderer-api/IDcsmRendererEventHandler.h"
#include "DisplayManager/IDisplayManager.h"
#include "Components/DcsmMetadata.h"
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

    class DcsmRendererImpl final : public StatusObjectImpl, public IDcsmConsumerEventHandler, public ramses_internal::IEventHandler
    {
    public:
        DcsmRendererImpl(const DcsmRendererConfig& config, IDcsmConsumerImpl& dcsmConsumer, std::unique_ptr<ramses_internal::IDisplayManager> displayManager);

        status_t requestContentReady(ContentID contentID, uint64_t timeOut);
        status_t showContent(ContentID contentID, AnimationInformation timingInfo);
        status_t hideContent(ContentID contentID, AnimationInformation timingInfo);
        status_t releaseContent(ContentID contentID, AnimationInformation timingInfo);
        status_t setCategorySize(Category categoryId, SizeInfo size, AnimationInformation timingInfo);
        status_t acceptStopOffer(ContentID contentID, AnimationInformation timingInfo);
        status_t assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder);
        status_t setDisplayBufferClearColor(displayBufferId_t displayBuffer, float r, float g, float b, float a);
        status_t linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t update(uint64_t timeStampNow, IDcsmRendererEventHandler& eventHandler, IRendererEventHandler* customRendererEventHandler);

    private:
        // IDcsmConsumerEventHandler
        virtual void contentOffered(ramses::ContentID contentID, ramses::Category category) override;
        virtual void contentReady(ramses::ContentID contentID, ramses::ETechnicalContentType contentType, ramses::TechnicalContentDescriptor contentDescriptor) override;
        virtual void contentFocusRequest(ramses::ContentID contentID) override;
        virtual void contentStopOfferRequest(ramses::ContentID contentID) override;
        virtual void forceContentOfferStopped(ramses::ContentID contentID) override;
        virtual void contentMetadataUpdated(ramses::ContentID contentID, const DcsmMetadataUpdate& metadataUpdate) override;

        // ramses_internal::IEventHandler
        virtual void scenePublished(ramses::sceneId_t sceneId) override;
        virtual void sceneStateChanged(ramses::sceneId_t sceneId, ramses_internal::SceneState state, ramses::displayId_t displaySceneIsMappedTo) override;
        virtual void offscreenBufferLinked(ramses::displayBufferId_t offscreenBufferId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, bool success) override;
        virtual void dataLinked(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, bool success) override;

        void executePendingCommands();
        void dispatchPendingEvents(IDcsmRendererEventHandler& eventHandler);

        void requestSceneState(ContentID contentID, ramses_internal::SceneState state);
        void handleContentStateChange(ContentID contentID, ContentState lastState);
        void removeContent(ContentID contentID);
        void scheduleSceneStateChange(ContentID contentThatTrigerredChange, ramses_internal::SceneState sceneState, uint64_t ts);
        ContentState getCurrentState(ContentID contentID) const;
        void processTimedOutRequests();

        const sceneId_t* findContentScene(ContentID contentID) const;

        std::unique_ptr<ramses_internal::IDisplayManager> m_displayManager;
        IDcsmConsumerImpl& m_dcsmConsumer;

        uint64_t m_timeStampNow = 0;

        struct CategoryInfo
        {
            SizeInfo size;
            displayId_t display;
            std::unordered_set<ContentID> assignedContentIds;
        };
        std::unordered_map<Category, CategoryInfo> m_categories;

        struct ContentInfo
        {
            Category category;

            bool readyRequested = false;
            bool dcsmReady = false;
            uint64_t readyRequestTimeOut = std::numeric_limits<uint64_t>::max();
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
            ramses_internal::SceneState sceneState = ramses_internal::SceneState::Unavailable;
        };
        std::vector<Command> m_pendingCommands;

        enum class EventType
        {
            ContentStateChanged,
            ContentFocusRequested,
            ContentStopOfferRequested,
            ContentNotAvailable,
            ContentMetadataUpdate,
            OffscreenBufferLinked,
            DataLinked
        };

        struct Event
        {
            EventType type;
            ContentID contentID{};
            Category category{};
            ContentState contentState = ContentState::Invalid;
            ContentState previousState = ContentState::Invalid;
            DcsmRendererEventResult result = DcsmRendererEventResult::TimedOut;
            ramses_internal::DcsmMetadata metadata{};

            ContentID providerContentID{};
            ContentID consumerContentID{};
            dataProviderId_t providerID{ 0 };
            dataConsumerId_t consumerID{ 0 };
            displayBufferId_t displayBuffer{};
        };
        std::vector<Event> m_pendingEvents;
        std::vector<Event> m_pendingEventsTmp; // to avoid allocs
    };
}

#endif
