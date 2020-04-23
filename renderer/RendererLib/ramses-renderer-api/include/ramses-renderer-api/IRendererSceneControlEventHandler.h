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
        * @brief This method will be called if a previously unknown or unpublished scene becomes published.
        * @details This is just informative callback, a scene can be set to a certain RendererSceneState
        *          even if it is not published yet/anymore, see #ramses::RendererSceneControl::setSceneState.
        *          Even though scene's RendererSceneState depends on whether scene is published or not,
        *          there is no 1:1 representation of published/unpublished in RendererSceneState enumeration,
        *          this comes partially from the fact that publish/unpublish is controlled from RamsesClient API,
        *          not the renderer's scene control API.
        *          There is no explicit callback when scene is unpublished, its state
        *          would be reported as #RendererSceneState::Unavailable via #sceneStateChanged as side effect,
        *          however a scene being in state #RendererSceneState::Unavailable does not
        *          necessarily mean it is unpublished.
        *
        * @param sceneId The ID of scene that was published
        */
        virtual void scenePublished(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called when state of a scene changes.
        * @details Typically this is a result of #ramses::RendererSceneControl::setSceneState call
        *          but can be also triggered externally (e.g. scene was unpublished by client).
        *          Note that there can be multiple state change callbacks in a row depending on
        *          number of state transitions needed between the previous state and target state
        *          when calling #ramses::RendererSceneControl::setSceneState.
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
        * @brief This method will be called after a flush with version tag (#ramses::Scene::flush) has been applied
        * @param sceneId The scene id of the scene which the versioned flush belongs to
        * @param sceneVersionTag The version tag of the scene flush
        */
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag) = 0;

        /**
        * @brief This method will be called if a scene which has an expiration timestamp set (#ramses::Scene::setExpirationTimestamp)
        *        is on renderer (not necessarily rendered) at a state that expired, i.e. current time is after the expiration timestamp.
        *        This callback is called only once when the scene expires even if scene stays expired in subsequent frames.
        *        When the scene is updated again with a new not anymore expired timestamp, #sceneRecoveredFromExpiration is called.
        * @param sceneId The scene id of the scene on which the event occurred
        */
        virtual void sceneExpired(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called if a scene which previously expired (#ramses::Scene::setExpirationTimestamp and #sceneExpired)
        *        was updated with a new expiration timestamp that is not expired anymore.
        *        This callback is called only once when the scene switches state from expired to not expired.
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
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) = 0;

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
        * @copydoc ramses::IRendererSceneControlEventHandler::scenePublished
        */
        virtual void scenePublished(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

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
        * @copydoc ramses::IRendererSceneControlEventHandler::sceneFlushed
        */
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag) override
        {
            (void)sceneId;
            (void)sceneVersionTag;
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
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) override
        {
            (void)streamId;
            (void)available;
        }
    };
}

#endif
