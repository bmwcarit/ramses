//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ICLIENTEVENTHANDLER_H
#define RAMSES_ICLIENTEVENTHANDLER_H

#include "ramses-framework-api/APIExport.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/RendererSceneState.h"

namespace ramses
{
    class Scene;
    class SceneReference;

    /**
    * @brief Provides an interface for handling the result of client events.
    *        Implementation of this interface must be passed to RamsesClient::dispatchEvents
    *        which will in return invoke methods of the interface according to events that occurred since last dispatching.
    */
    class RAMSES_API IClientEventHandler
    {
    public:
        /**
        * @brief This method will be called when asynchronous loading of a resource file failed.
        * @param filename The filename of the resource file that failed to load
        */
        virtual void resourceFileLoadFailed(const char* filename) = 0;

        /**
        * @brief This method will be called when asynchronous loading of a resource file
        *        successfully finished.
        * @param filename The filename of the resource file that finished loading.
        */
        virtual void resourceFileLoadSucceeded(const char* filename) = 0;

        /**
        * @brief This method will be called when asynchronous loading of a scene or one
        *        of its associated resource files failed.
        *        There will also be events generated for the resource files provided to
        *        loadSceneFromFileAsync().
        *
        * @param filename The filename of the scene file that failed to load.
        */
        virtual void sceneFileLoadFailed(const char* filename) = 0;

        /**
        * @brief This method will be called when asynchronous loading of a scene file
        *        and its associated resource files successfully finished.
        *        There will also be events generated for the resource files provided to
        *        loadSceneFromFileAsync().
        *
        * @param filename The filename of the scene file that finished loading.
        * @param loadedScene Pointer to the newly loaded scene.
        */
        virtual void sceneFileLoadSucceeded(const char* filename, Scene* loadedScene) = 0;

        /**
        * @brief This method will be called when state on renderer side of a scene referenced
        *        using #ramses::SceneReference changed.
        * @details Typically this is a result of #ramses::SceneReference::requestState call
        *          but can be also triggered externally (e.g. scene was unpublished by client).
        *          Note that there can be multiple state change callbacks in a row depending on
        *          number of state transitions needed between the previous state and target state
        *          when calling #ramses::SceneReference::requestState.
        *
        * @param sceneRef Instance of #ramses::SceneReference that references the scene with changed state
        * @param state New renderer state of the referenced scene
        */
        virtual void sceneReferenceStateChanged(SceneReference& sceneRef, RendererSceneState state) = 0;

        /**
        * @brief This method will be called after a flush with version tag (#ramses::Scene::flush) has been applied
        *        and there is a #ramses::SceneReference with enabled version tag notifications
        *        (#ramses::SceneReference::requestNotificationsForSceneVersionTags).
        * @param sceneRef Instance of #ramses::SceneReference that references the scene which the versioned flush belongs to
        * @param versionTag Version tag of the scene flush
        */
        virtual void sceneReferenceFlushed(SceneReference& sceneRef, sceneVersionTag_t versionTag) = 0;

        /**
        * @brief This method will be called when the data link between a data provider and data consumer is established
        *        (or failed to be established) on renderer side.
        * @details This callback is response to #ramses::Scene::linkData call on a scene that belongs
        *          to this #ramses::RamsesClient, not by data link initiated from other clients' scenes or renderer.
        *
        * @param providerScene The ID of scene where the data provider slot is,
        *                      can be either a #ramses::Scene from this #ramses::RamsesClient or another scene referenced via #ramses::SceneReference
        * @param providerId The ID of data provider which was linked
        * @param consumerScene The ID of scene where the data consumer slot is,
        *                      can be either a #ramses::Scene from this #ramses::RamsesClient or another scene referenced via #ramses::SceneReference
        * @param consumerId The ID of data consumer which was linked
        * @param success True if succeeded, false otherwise - check renderer logs for concrete error message.
        */
        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) = 0;

        /**
        * @brief This method will be called when the data link between a data provider and data consumer is destroyed
        *        (or failed to be destroyed) on renderer side.
        * @details This callback is response to #ramses::Scene::unlinkData call on a scene that belongs
        *          to this #ramses::RamsesClient, not by data unlink initiated from other clients' scenes or renderer.
        *
        * @param consumerScene The ID of scene where the data consumer slot is,
        *                      can be either a #ramses::Scene from this #ramses::RamsesClient or another scene referenced via #ramses::SceneReference
        * @param consumerId The ID of data consumer which was unlinked
        * @param success True if succeeded, false otherwise - check renderer logs for concrete error message.
        */
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, bool success) = 0;

        /**
        * @brief Empty destructor
        */
        virtual ~IClientEventHandler() = default;
    };
}

#endif
