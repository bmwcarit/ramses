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
        * Stores internal data for implementation specifics of SceneConfig.
        */
        class SceneConfigImpl& impl;
    };
}

#endif
