//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/EScenePublicationMode.h"

#include <memory>

namespace ramses
{
    namespace internal
    {
        class SceneConfigImpl;
    }

    /**
    * @ingroup CoreAPI
    * @brief The SceneConfig holds a set of parameters to be used when creating a scene or loading the scene from file/memory.
    * Some parameters are only relevant for loading scene.
    */
    class RAMSES_API SceneConfig
    {
    public:
        /**
        * @brief default constructor
        */
        SceneConfig();

        /**
        * @brief constructor of SceneConfig
        *
        * @param sceneId Each scene requires a sceneId for global identification (sceneId is used for scene mapping on renderer side).
        * If no sceneId is provided Ramses will use the sceneId stored in the serialized file or raise an error if the scene is created at runtime.
        * @param publicationMode see #ramses::SceneConfig::setPublicationMode for details
        */
        explicit SceneConfig(sceneId_t sceneId, EScenePublicationMode publicationMode = EScenePublicationMode::LocalOnly);

        /**
        * @brief Destructor of SceneConfig
        */
        ~SceneConfig();

        /**
        * @brief Set the publication mode that will be used for this scene.
        *
        * Later calls to publish must use the same value as given here.
        * Setting this to #ramses::EScenePublicationMode::LocalOnly for scenes that will never be
        * published remotely enables optimization possibilities.
        *
        * @param publicationMode Publication mode to use with scene. #ramses::EScenePublicationMode::LocalOnly is default.
        */
        void setPublicationMode(EScenePublicationMode publicationMode);

        /**
         * Sets the sceneId for the loaded/created scene. By default the id stored in the binary will be used.
         *
         * @param sceneId The sceneId for the loaded scene
         */
        void setSceneId(sceneId_t sceneId);

        /**
         * By default, ramses performs sanity checks on the binary data when the scene is loaded from file.
         * This behavior can be disabled here (not recommended unless there are other measures to ensure valid binary data).
         *
         * @param enabled flag to disable/enable memory verification when loading the scene
         */
        void setMemoryVerificationEnabled(bool enabled);

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        SceneConfig(const SceneConfig& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        SceneConfig(SceneConfig&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        SceneConfig& operator=(const SceneConfig& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        SceneConfig& operator=(SceneConfig&& other) noexcept;

        /**
         * Get the internal data for implementation specifics of SceneConfig.
         */
        [[nodiscard]] internal::SceneConfigImpl& impl();

        /**
         * Get the internal data for implementation specifics of SceneConfig.
         */
        [[nodiscard]] const internal::SceneConfigImpl& impl() const;

    protected:
        /**
        * Stores internal data for implementation specifics of SceneConfig.
        */
        std::unique_ptr<internal::SceneConfigImpl> m_impl;
    };
}
