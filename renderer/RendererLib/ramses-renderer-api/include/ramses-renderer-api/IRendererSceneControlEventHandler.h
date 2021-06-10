//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDERERSCENECONTROLEVENTHANDLER_H
#define RAMSES_IRENDERERSCENECONTROLEVENTHANDLER_H

#include "ramses-renderer-api/Types.h"
#include "ramses-framework-api/RendererSceneState.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
    * @brief Provides an interface for handling the result of renderer scene control events.
    *        Implementation of this interface must be passed to #ramses::RendererSceneControl::dispatchEvents
    *        which will in return invoke methods of the interface according to events that occurred since last dispatching.
    */
    class RAMSES_API IRendererSceneControlEventHandler
    {
    public:
        /**
        * @brief This method will be called when state of a scene changes.
        * @details Typically this is a result of #ramses::RendererSceneControl::setSceneState call
        *          but can be also triggered externally (e.g. scene was unpublished by client).
        *          Note that there can be multiple state change callbacks in a row depending on
        *          number of state transitions needed between the previous state and target state
        *          when calling #ramses::RendererSceneControl::setSceneState.
        *
        * Limitation: This callback will not be executed if there is no display created and running.
        *             All published scenes (Available state) will be announced only after a display
        *             is created and running.
        *             Also all published scenes will be announced again if display is destroyed
        *             and created again and it is the only display on renderer.
        *
        * @param sceneId The ID of scene with changed state
        * @param state New state of the scene
        */
        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) = 0;

        /**
        * @brief This method will be called when the data link between offscreen buffer and scene's data slot is established.
        * @details This is a result of #ramses::RendererSceneControl::linkOffscreenBuffer call.
        *
        * @param offscreenBufferId The ID of offscreen buffer which is linked as data provider
        * @param consumerScene The ID of scene where the data consumer slot is
        * @param consumerId The ID of data consumer where the offscreen buffer is linked to
        * @param success True if succeeded, false otherwise - check renderer logs for concrete error message.
        */
        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) = 0;

        /**
        * @brief This method will be called when the data link between a data provider and data consumer is established.
        * @details This is a result of #ramses::RendererSceneControl::linkData call.
        *
        * @param providerScene The ID of scene where the data provider slot is
        * @param providerId The ID of data provider which was linked
        * @param consumerScene The ID of scene where the data consumer slot is
        * @param consumerId The ID of data consumer which was linked
        * @param success True if succeeded, false otherwise - check renderer logs for concrete error message.
        */
        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) = 0;

        /**
        * @brief This method will be called when the data link between a data provider and data consumer is destroyed.
        * @details This is a result of #ramses::RendererSceneControl::unlinkData call.
        *
        * @param consumerScene The ID of scene where the data consumer slot is
        * @param consumerId The ID of data consumer which was unlinked
        * @param success True if succeeded, false otherwise - check renderer logs for concrete error message.
        */
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) = 0;

        /**
        * @brief   This method will be called whenever a data provider is created.
        * @details The event is emitted also for every data provider in a newly available scene.
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataProviderId The created data provider id
        */
        virtual void dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId) = 0;

        /**
        * @brief   This method will be called when a data provider is destroyed.
        * @details The event is emitted only when data provider destroyed,
        *          not if scene becomes unavailable as a whole.
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataProviderId The destroyed data provider id
        */
        virtual void dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId) = 0;

        /**
        * @brief   This method will be called whenever a data consumer is created.
        * @details The event is emitted also for every data consumer in a newly available scene.
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataConsumerId The created data consumer id
        */
        virtual void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) = 0;

        /**
        * @brief   This method will be called when a data consumer is destroyed.
        * @details The event is emitted only when data consumer destroyed,
        *          not if scene becomes unavailable as a whole.
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataConsumerId The destroyed data consumer id
        */
        virtual void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) = 0;

        /**
        * @brief This method will be called after a flush with version tag (#ramses::Scene::flush) has been applied
        * @param sceneId The scene id of the scene which the versioned flush belongs to
        * @param sceneVersionTag The version tag of the scene flush
        */
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag) = 0;

        /**
        * @brief This method will be called whenever a scene which was not previously monitored for expiration has requested expiration
        *        monitoring by sending a scene flush with valid expiration timestamp (#ramses::Scene::setExpirationTimestamp)
        *        and that flush was applied on renderer side.
        *        From this point on, the scene will be monitored, can expire and recover (#sceneExpired, #sceneRecoveredFromExpiration)
        *        until monitoring disabled again (#sceneExpirationMonitoringDisabled).
        * @param sceneId The scene id of the scene that will be monitored for expiration
        */
        virtual void sceneExpirationMonitoringEnabled(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called whenever a scene which was previously monitored for expiration has requested
        *        to stop being monitored by sending a scene flush with invalid expiration timestamp (#ramses::Scene::setExpirationTimestamp)
        *        and that flush was applied on renderer side.
        *        This method will also be called if a previously monitored scene is unsubscribed from the renderer,
        *        i.e. drops state to #ramses::RendererSceneState::Available from previous #ramses::RendererSceneState::Ready
        *        (or potentially after canceled transition to #ramses::RendererSceneState::Ready).
        *        From this point on, the scene will not be monitored anymore, regardless if it previously expired or not,
        *        i.e. there will be no expiration events (#sceneExpired, #sceneRecoveredFromExpiration) until monitoring
        *        enabled again (#sceneExpirationMonitoringEnabled).
        * @param sceneId The scene id of the scene that will not be monitored for expiration anymore
        */
        virtual void sceneExpirationMonitoringDisabled(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called if a scene which is enabled for expiration monitoring (#sceneExpirationMonitoringEnabled)
        *        is on renderer (not necessarily rendered) at a state that expired, i.e. current time is after the expiration timestamp
        *        set via #ramses::Scene::setExpirationTimestamp.
        *        This callback is called only once when the scene expires even if scene stays expired in subsequent frames.
        *        When the scene is updated again with a new not anymore expired timestamp, #sceneRecoveredFromExpiration is called.
        * @param sceneId The scene id of the scene on which the event occurred
        */
        virtual void sceneExpired(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called if a scene which previously expired (#ramses::Scene::setExpirationTimestamp and #sceneExpired)
        *        was updated with a new expiration timestamp that is not expired anymore.
        *        This callback is called only once when the scene switches state from expired to not expired.
        *        This callback is not called when monitoring becomes disabled (#sceneExpirationMonitoringDisabled) while scene
        *        is expired (#sceneExpired).
        * @param sceneId The scene id of the scene on which the event occurred
        */
        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called when a new IVI video stream becomes available, or when an existing stream disappears
        * In terms of Wayland protocol, a stream is available if an "ivi_application" exists which has created a wayland surface
        * (wl_surface) with ivi_id=streamId, and the surface has at least one attached non-nullptr buffer (i.e. renderable content).
        *
        * It is possible that the ivi_application does not update its surface (by providing new buffers/frames), but RAMSES has
        * no way of knowing that, hence a stream is not reported unavailable in that case.
        *
        * A surface becomes unavailable whenever either the ivi_application is closed or when it attached
        * a nullptr buffer to the stream surface with id=streamId (i.e. actively told wayland that it should not render contents to
        * this ivi surface).
        *
        * @param streamId The IVI stream id
        * @param available True if the stream became available, and false if it disappeared
        */
        virtual void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) = 0;

        /**
        * @brief This method will be called when there were scene objects picked.
        *        A ramses::PickableObject can be 'picked' via a pick input event
        *        which is passed to ramses::RendererSceneControl when the scene is rendered (see ramses::RendererSceneControl::handlePickEvent).
        *
        * @param[in] sceneId ID of scene to which the picked objects belong.
        * @param[in] pickedObjects Pointer to first ID of the picked objects array.
        *        This array is valid only for the time of calling this method.
        * @param[in] pickedObjectsCount Number of picked object IDs in the \c pickedObjects array.
        */
        virtual void objectsPicked(sceneId_t sceneId, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) = 0;

        /// Empty destructor
        virtual ~IRendererSceneControlEventHandler() = default;
    };

    /**
    * @brief Convenience empty implementation of IRendererSceneControlEventHandler that can be used to derive from
    *        when only subset of event handling methods need to be implemented.
    */
    class RAMSES_API RendererSceneControlEventHandlerEmpty : public IRendererSceneControlEventHandler
    {
    public:
        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::sceneStateChanged
        */
        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override
        {
            (void)sceneId;
            (void)state;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::offscreenBufferLinked
        */
        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override
        {
            (void)offscreenBufferId;
            (void)consumerScene;
            (void)consumerId;
            (void)success;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::dataLinked
        */
        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override
        {
            (void)providerScene;
            (void)providerId;
            (void)consumerScene;
            (void)consumerId;
            (void)success;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::dataUnlinked
        */
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) override
        {
            (void)consumerScene;
            (void)consumerId;
            (void)success;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::dataProviderCreated
        */
        virtual void dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            (void)sceneId;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::dataProviderDestroyed
        */
        virtual void dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            (void)sceneId;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::dataConsumerCreated
        */
        virtual void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            (void)sceneId;
            (void)dataConsumerId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::dataConsumerDestroyed
        */
        virtual void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            (void)sceneId;
            (void)dataConsumerId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::sceneFlushed
        */
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag) override
        {
            (void)sceneId;
            (void)sceneVersionTag;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::sceneExpirationMonitoringEnabled
        */
        virtual void sceneExpirationMonitoringEnabled(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::sceneExpirationMonitoringDisabled
        */
        virtual void sceneExpirationMonitoringDisabled(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::sceneExpired
        */
        virtual void sceneExpired(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::sceneRecoveredFromExpiration
        */
        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::streamAvailabilityChanged
        */
        virtual void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) override
        {
            (void)streamId;
            (void)available;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler::objectsPicked
        */
        virtual void objectsPicked(sceneId_t sceneId, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override
        {
            (void)sceneId;
            (void)pickedObjects;
            (void)pickedObjectsCount;
        }
    };
}

#endif
