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
#include "RendererSceneControlImpl.h"
#include <map>
#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    class IDcsmConsumerImpl;
    class IRendererSceneControl;
    class CategoryInfoUpdate;

    class DcsmContentControlImpl final : public StatusObjectImpl, public IDcsmConsumerEventHandler, public IRendererSceneControlEventHandler, public IRendererSceneControlEventHandler_SpecialForWayland
    {
    public:
        DcsmContentControlImpl(IDcsmConsumerImpl& dcsmConsumer, IRendererSceneControl& sceneControl);

        status_t addContentCategory(Category category, displayId_t display, const CategoryInfoUpdate& categoryInformation);
        status_t removeContentCategory(Category category);
        status_t requestContentReady(ContentID contentID, uint64_t timeOut);
        status_t showContent(ContentID contentID, AnimationInformation timingInfo);
        status_t hideContent(ContentID contentID, AnimationInformation timingInfo);
        status_t releaseContent(ContentID contentID, AnimationInformation timingInfo);
        status_t setCategoryInfo(Category categoryId, const CategoryInfoUpdate& categoryInfo, AnimationInformation timingInfo);
        status_t acceptStopOffer(ContentID contentID, AnimationInformation timingInfo);
        status_t assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder);
        status_t linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t linkContentToTextureConsumer(ContentID contentID, ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t unlinkData(ContentID consumerContentID, dataConsumerId_t consumerId);
        status_t handlePickEvent(ContentID contentID, float bufferNormalizedCoordX, float bufferNormalizedCoordY);
        status_t update(uint64_t timeStampNow, IDcsmContentControlEventHandler& eventHandler);
        status_t sendContentStatus(ContentID contentID, DcsmStatusMessage const& message);

    private:
        struct ContentLink
        {
            ContentID provider;
            ContentID consumer;
            dataConsumerId_t dataConsumerId;
            bool eventSent;
        };

        // IDcsmConsumerEventHandler
        virtual void contentOffered(ContentID contentID, Category category, ETechnicalContentType contentType) override;
        virtual void contentDescription(ContentID contentID, TechnicalContentDescriptor contentDescriptor) override;
        virtual void contentReady(ContentID contentID) override;
        virtual void contentEnableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) override;
        virtual void contentDisableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) override;
        virtual void contentStopOfferRequest(ContentID contentID) override;
        virtual void forceContentOfferStopped(ContentID contentID) override;
        virtual void contentMetadataUpdated(ContentID contentID, const DcsmMetadataUpdate& metadataUpdate) override;

        // IRendererSceneControlEventHandler
        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override;
        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override;

#ifdef RAMSES_ENABLE_EXTERNAL_BUFFER_EVENTS
        virtual void externalBufferLinked(externalBufferId_t externalBufferId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override;
#endif

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
        // todo (jonathan) cleanup with next major version
        // IRendererSceneControlEventHandler_SpecialForWayland
        virtual void streamBufferLinked(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId, bool success) override;
        virtual void streamBufferEnabled(streamBufferId_t streamBufferId, bool status) override;

        void executePendingCommands();
        void dispatchPendingEvents(IDcsmContentControlEventHandler& eventHandler);

        void requestSceneState(ContentID contentID, RendererSceneState state);
        void handleContentStateChange(ContentID contentID, ContentState lastState);
        void removeContent(ContentID contentID);
        void scheduleHideAnimation(ContentID contentID, AnimationInformation animTime, RendererSceneState targetState);
        ContentState determineCurrentContentState(ContentID contentID) const;
        void processTimedOutRequests();
        void resetContentStateAfterTechnicalContentReset(ContentID contentID);

        TechnicalContentDescriptor getTechnicalContentAssociatedWithContent(ContentID contentID) const;
        std::vector<ContentID> findContentsAssociatingSceneId(sceneId_t sceneId) const;
        std::vector<ContentID> findContentsAssociatingSurfaceId(waylandIviSurfaceId_t surfaceId) const;
        std::vector<ContentID> findContentsAssociatingTechnicalIdAndType(TechnicalContentDescriptor descriptor, ETechnicalContentType type) const;

        void goToConsolidatedDesiredState(TechnicalContentDescriptor techId);
        void goToConsolidatedDesiredSceneState(TechnicalContentDescriptor techId);
        void goToConsolidatedDesiredStreamState(TechnicalContentDescriptor techId);
        void applyTechnicalStateChange(TechnicalContentDescriptor techId, RendererSceneState state);

        void createStreamBuffer(displayId_t displayId, waylandIviSurfaceId_t surfaceId);
        void rememberLink(ContentLink link);
        void removePendingShow(ContentID contentID);

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

        enum class ContentDcsmState
        {
            Assigned,
            ReadyRequested,
            Ready,
            Shown
        };
        static_assert(ContentDcsmState::Assigned < ContentDcsmState::ReadyRequested &&
            ContentDcsmState::ReadyRequested < ContentDcsmState::Ready&&
            ContentDcsmState::Ready < ContentDcsmState::Shown,
            "Comparison operator in use, make sure to not break order");

        struct ContentInfo
        {
            Category category;
            ETechnicalContentType contentType;
            ContentDcsmState dcsmState = ContentDcsmState::Assigned;
            TechnicalContentDescriptor descriptor;
            displayBufferId_t displayBufferAssignment;
            uint64_t readyRequestTimeOut = std::numeric_limits<uint64_t>::max();
            bool stopOfferRequested = false;
        };
        std::unordered_map<ContentID, ContentInfo> m_contents;

        std::vector<ContentLink> m_contentTextureLinks;

        struct OfferedContents
        {
            Category category;
            ContentID contentID;
            ETechnicalContentType contentType;
        };
        std::vector<OfferedContents> m_offeredContentsForOtherCategories;

        struct TechnicalContentInfo
        {
            ETechnicalContentType techType;
            SharedSceneState sharedState;
            displayId_t display;
            std::unordered_set<ContentID> associatedContents;
        };

        std::unordered_map<TechnicalContentDescriptor, TechnicalContentInfo> m_techContents;
        std::unordered_map<waylandIviSurfaceId_t, streamBufferId_t> m_streamBuffers;
        std::unordered_set<waylandIviSurfaceId_t> m_availableStreams;

        struct SceneStateChangeCommand
        {
            uint64_t timePoint;
            ETechnicalContentType techType;
            TechnicalContentDescriptor technicalId;
            RendererSceneState sceneState = RendererSceneState::Unavailable;
        };
        std::map<ContentIdentifier, SceneStateChangeCommand> m_pendingSceneStateChangeCommands;

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
            StreamAvailable,
            ContentLinkedToTextureConsumer
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
