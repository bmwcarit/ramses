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
#include "ramses-client-api/EVisibilityMode.h"

namespace ramses
{
    class SceneConfigImpl;

    /**
    * @ingroup CoreAPI
    * @brief The SceneConfig holds a set of parameters to be used when creating a scene.
    */
    class SceneConfig : public StatusObject
    {
    public:
        /**
        * @brief Empty constructor of SceneConfig - has default values
        */
        RAMSES_API SceneConfig();

        /**
        * @brief Destructor of SceneConfig
        */
        RAMSES_API ~SceneConfig() override;

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
        RAMSES_API status_t setPublicationMode(EScenePublicationMode publicationMode);

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RAMSES_API SceneConfig(const SceneConfig& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RAMSES_API SceneConfig(SceneConfig&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RAMSES_API SceneConfig& operator=(const SceneConfig& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RAMSES_API SceneConfig& operator=(SceneConfig&& other) noexcept;

        /**
        * Stores internal data for implementation specifics of SceneConfig.
        */
        std::reference_wrapper<SceneConfigImpl> m_impl;
    };
}

#endif
