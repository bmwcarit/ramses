//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONTENTCONTROLIMPL_H
#define RAMSES_DCSMCONTENTCONTROLIMPL_H

#include "StatusObjectImpl.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-renderer-api/DcsmContentControl.h"
#include "ramses-renderer-api/DcsmContentControlConfig.h"
#include "ramses-renderer-api/IDcsmContentControlEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "Components/DcsmMetadata.h"
#include "ContentStates.h"
#include "SharedSceneState.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "Components/CategoryInfo.h"
#include "Components/DcsmTypes.h"

namespace ramses
{
    class IDcsmConsumerImpl;
    class IRendererSceneControl;
    class CategoryInfoUpdate;

    class DcsmContentControlImpl final : public StatusObjectImpl, public IDcsmConsumerEventHandler, public IRendererSceneControlEventHandler
    {
    public:
        DcsmContentControlImpl(const DcsmContentControlConfig& config, IDcsmConsumerImpl& dcsmConsumer, IRendererSceneControl& sceneControl);

        status_t addContentCategory(Category category, DcsmContentControlConfig::CategoryInfo categoryInformation);
        status_t removeContentCategory(Category category);
        status_t requestContentReady(ContentID contentID, uint64_t timeOut);
        status_t showContent(ContentID contentID, AnimationInformation timingInfo);
        status_t hideContent(ContentID contentID, AnimationInformation timingInfo);
        status_t releaseContent(ContentID contentID, AnimationInformation timingInfo);
        status_t setCategorySize(Category categoryId, const CategoryInfoUpdate& size, AnimationInformation timingInfo);
        status_t acceptStopOffer(ContentID contentID, AnimationInformation timingInfo);
        status_t assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder);
        status_t linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t unlinkData(ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t handlePickEvent(ContentID contentID, float bufferNormalizedCoordX, float bufferNormalizedCoordY);
        status_t update(uint64_t timeStampNow, IDcsmContentControlEventHandler& eventHandler);

    private:
        // IDcsmConsumerEventHandler
        virtual void contentOffered(ContentID contentID, Category category) override;
        virtual void contentDescription(ContentID contentID, ETechnicalContentType contentType, TechnicalContentDescriptor contentDescriptor) override;
        virtual void contentReady(ContentID contentID) override;
        virtual void contentEnableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) override;
        virtual void contentDisableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) override;
        virtual void contentStopOfferRequest(ContentID contentID) override;
        virtual void forceContentOfferStopped(ContentID contentID) override;
        virtual void contentMetadataUpdated(ContentID contentID, const DcsmMetadataUpdate& metadataUpdate) override;

        // IRendererSceneControlEventHandler
        virtual void scenePublished(sceneId_t sceneId) override;
        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override;
        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override;
        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override;
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override;
        virtual void objectsPicked(sceneId_t scene, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override;
        virtual void dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId) override;
        virtual void dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId) override;
        virtual void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override;
        virtual void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override;
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag) override;
        virtual void sceneExpirationMonitoringEnabled(sceneId_t sceneId) override;
        virtual void sceneExpirationMonitoringDisabled(sceneId_t sceneId) override;
        virtual void sceneExpired(sceneId_t sceneId) override;
        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) override;
        virtual void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) override;

        void executePendingCommands();
        void dispatchPendingEvents(IDcsmContentControlEventHandler& eventHandler);

        void requestSceneState(ContentID contentID, RendererSceneState state);
        void handleContentStateChange(ContentID contentID, ContentState lastState);
        void removeContent(ContentID contentID);
        void scheduleSceneStateChange(ContentID contentThatTrigerredChange, RendererSceneState sceneState, uint64_t ts);
        ContentState getCurrentState(ContentID contentID) const;
        void processTimedOutRequests();

        sceneId_t findSceneAssociatedWithContent(ContentID contentID) const;
        std::vector<ContentID> findContentsAssociatingScene(sceneId_t sceneId) const;


        IRendererSceneControl& m_sceneControl;
        IDcsmConsumerImpl& m_dcsmConsumer;

        uint64_t m_timeStampNow = 0;

        struct CategoryData
        {
            ramses_internal::CategoryInfo categoryInfo;
            displayId_t display;
            std::unordered_set<ContentID> assignedContentIds;
        };
        std::unordered_map<Category, CategoryData> m_categories;

        struct ContentInfo
        {
            Category category;
            ramses_internal::ETechnicalContentType contentType;

            bool readyRequested = false;
            bool dcsmReady = false;
            uint64_t readyRequestTimeOut = std::numeric_limits<uint64_t>::max();
        };
        std::unordered_map<ContentID, ContentInfo> m_contents;

        struct OfferedContents
        {
            Category category;
            ContentID contentID;
        };
        std::vector<OfferedContents> m_offeredContentsForOtherCategories;

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
            RendererSceneState sceneState = RendererSceneState::Unavailable;
        };
        std::vector<Command> m_pendingCommands;

        enum class EventType
        {
            ContentStateChanged,
            ContentEnableFocusRequest,
            ContentDisableFocusRequest,
            ContentStopOfferRequested,
            ContentNotAvailable,
            ContentMetadataUpdate,
            OffscreenBufferLinked,
            DataLinked,
            DataUnlinked,
            ObjectsPicked,
            DataProviderCreated,
            DataProviderDestroyed,
            DataConsumerCreated,
            DataConsumerDestroyed,
            ContentFlushed,
            ContentExpirationMonitoringEnabled,
            ContentExpirationMonitoringDisabled,
            ContentExpired,
            ContentRecoveredFromExpiration,
            StreamAvailable
        };

        struct Event
        {
            EventType type;
            ContentID contentID{};
            Category category{};
            ContentState contentState = ContentState::Invalid;
            ContentState previousState = ContentState::Invalid;
            DcsmContentControlEventResult result = DcsmContentControlEventResult::TimedOut;
            ramses_internal::DcsmMetadata metadata{};
            sceneVersionTag_t version{};

            ContentID providerContentID{};
            ContentID consumerContentID{};
            dataProviderId_t providerID{};
            dataConsumerId_t consumerID{};
            displayBufferId_t displayBuffer{};
            std::vector<pickableObjectId_t> pickedObjectIds{};
            waylandIviSurfaceId_t streamSource{};
            int32_t focusRequest = 0;
            bool streamAvailable{ false };
        };
        std::vector<Event> m_pendingEvents;
        std::vector<Event> m_pendingEventsTmp; // to avoid allocs
    };
}

#endif
