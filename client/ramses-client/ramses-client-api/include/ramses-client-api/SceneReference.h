//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEREFERENCE_H
#define RAMSES_SCENEREFERENCE_H

#include "ramses-client-api/SceneObject.h"
#include "ramses-framework-api/RendererSceneState.h"

namespace ramses
{
    /**
    * @brief The SceneReference object refers to another ramses scene using its sceneId.
    * @details The SceneReference object references a scene, which might be otherwise unknown
    *          to this RamsesClient, but is or expected to be known to a RamsesRenderer subscribed to its master scene
    *          (scene in which the reference was created using #ramses::Scene::createSceneReference).
    *          The SceneReference allows to remotely change limited set of states of the referenced scene on renderer side,
    *          results of those requests are reported asynchronously in form of event callbacks, see #ramses::IClientEventHandler.
    *
    *          There cannot be multiple instances of SceneReference referring to the same sceneId, however SceneReference
    *          can be destroyed using #ramses::Scene::destroy and re-created with same sceneId in another scene,
    *          i.e. change its 'master' scene.
    *          It is recommended to get SceneReference to #ramses::RendererSceneState::Unavailable state and wait for confirmation
    *          (#ramses::IClientEventHandler::sceneReferenceStateChanged) before destroying it, otherwise the scene remains on renderer
    *          side with no owner, potentially consuming resources and causing undesired results if shown, also any pending requests, events
    *          or actions will either fail or get lost (no master scene to report to).
    *          Even though recommended, setting referenced scene to unavailable before destroying is not strictly required,
    *          so it is possible to change its master scene regardless of its actual state on renderer side (even if actively rendered)
    *          but this should be done only with extra caution and understanding of the consequences mentioned above.
    */
    class RAMSES_API SceneReference : public SceneObject
    {
    public:
        /**
        * @brief   Set a requested state for this scene reference.
        * @details The #ramses::RamsesRenderer will
        *          bring the referenced scene to this state, if preconditions are met.
        *          When requesting #ramses::RendererSceneState::Ready or higher the referenced scene
        *          will inherit display mapping from the master scene (#ramses::Scene where this reference was created),
        *          that implies that the mapping of master scene has to be set on renderer side first
        *          (#ramses::RendererSceneControl::setSceneMapping).
        *
        *          Whenever a state of the referenced scene changes on renderer side,
        *          there will be a callback #ramses::IClientEventHandler::sceneReferenceStateChanged.
        *          Note that scene state on renderer side can also change due to reasons not controlled from
        *          client side (e.g. referenced scene is unpublished).
        *
        *          This is just a request to the renderer to change the scene state,
        *          there are requirements that have to be met for every scene state change.
        *          The renderer waits with the scene state change till all the conditions are met,
        *          therefore the request cannot fail but it might never happen.
        *          It is application's responsibility to implement a timeout logic if needed.
        *
        *          It is not allowed to request state Unavailable, as this state can only be the result
        *          of external conditions.
        *
        * @param[in] requestedState
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t requestState(RendererSceneState requestedState);

        /**
        * @brief Get the currently requested state for this scene reference.
        * @return The state of the reference.
        */
        RendererSceneState getRequestedState() const;

        /**
        * @brief Get the sceneId of the referenced scene.
        * @return The scene id of the referenced scene
        */
        sceneId_t getReferencedSceneId() const;

        /**
        * @brief Request callbacks (#ramses::IClientEventHandler::sceneReferenceFlushed) to be triggered whenever a flush
        *        with valid version tag (#ramses::Scene::flush) has been applied to the referenced scene on the renderer.
        *        Enabling notifications after they were previously disabled will also trigger said event once
        *        with the last applied valid version tag (i.e. version tag last applied before notifications were enabled).
        *        Note that in case there is flush with version applied and notifications enabled within the same renderer
        *        update loop the #ramses::IClientEventHandler::sceneReferenceFlushed callback might be triggered twice
        *        reporting the same version, however the order of reporting is always strictly kept and matching order
        *        of flushes applied.
        *
        *        Scene reference has to be in Ready state to be able to receive scene version tag notifications.
        *
        * @param[in] flag enable/disable notifications for this scene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t requestNotificationsForSceneVersionTags(bool flag);

        /**
        * @brief   Set scene render order
        * @details Scenes are rendered one after each other on renderer side,
        *          lower number means scene will be rendered before all scenes with higher number.
        *          The render order specified here is relative to its master scene render order,
        *          master scene render order is set on renderer side (#ramses::RendererSceneControl::setSceneDisplayBufferAssignment).
        *
        * @param[in] renderOrder Lower value means that a scene is rendered before a scene with higher value. Default is 0.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRenderOrder(int32_t renderOrder);

        /**
        * Stores internal data for implementation specifics of SceneReference.
        */
        class SceneReferenceImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating SceneReference instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for SceneReference.
        *
        * @param[in] pimpl Internal data for implementation specifics of SceneReference (sink - instance becomes owner)
        */
        explicit SceneReference(SceneReferenceImpl& pimpl);

        /**
        * @brief Destructor of the SceneReference
        */
        virtual ~SceneReference() override;
    };
}

#endif
