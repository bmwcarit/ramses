//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECONFIG_H
#define RAMSES_SCENECONFIG_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-client-api/EScenePublicationMode.h"

namespace ramses
{
    /**
    * @brief The SceneConfig holds a set of parameters to be used when creating a scene.
    */
    class RAMSES_API SceneConfig : public StatusObject
    {
    public:
        /**
        * @brief Empty constructor of SceneConfig - has default values
        */
        SceneConfig();

        /**
        * @brief Copy constructor of SceneConfig
        * @param[in] other Other instance of SceneConfig
        */
        SceneConfig(const SceneConfig& other);

        /**
        * @brief Destructor of SceneConfig
        */
        virtual ~SceneConfig();

        /**
        * @brief Set the publication mode that will be used for this scene.
        *
        * Later calls to publish must use the same value as given here.
        * Setting this to EScenePublicationMode_LocalOnly for scenes that will never be
        * published remotely enables optimization possibilities.
        *
        * @param[in] publicationMode Publication mode to use with scene.
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setPublicationMode(EScenePublicationMode publicationMode);

        /**
         * @brief Enable latency monitoring and set the maximum allowed latency.
         *
         * When enabled by setting maxLatencyInMilliseconds > 0 only the ramses::Scene::flushWithTimestamp
         * can be used to flush the scene with non-zero timestamp in synchronized PTP timebase.
         *
         * When the scene with the flush is rendered the renderer will check that
         * "currentPtpTime <= (flushTimestamp + limitInMilliseconds)".
         * If this condition is not met the renderer reports that via renderer event that
         * the maximum latency was exceeded, meaning that the state of the scene rendered is too old.
         *
         * Renderer callbacks reporting latency changes (refer to them for more details):
         *   IRendererEventHandler::sceneUpdateLatencyExceeded
         *   IRendererEventHandler::sceneUpdateLatencyBackBelowLimit
         *
         * @param[in] maxLatencyInMilliseconds Maximum latency in milliseconds, monitoring is enabled only if > 0
         * @return StatusOK on success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setMaximumLatency(uint32_t maxLatencyInMilliseconds);

        /**
        * Stores internal data for implementation specifics of SceneConfig.
        */
        class SceneConfigImpl& impl;
    };
}

#endif
