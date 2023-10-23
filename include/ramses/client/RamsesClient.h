//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/RamsesObject.h"
#include "ramses/client/SceneConfig.h"

#include <string_view>

/**
* ramses namespace
*
* @brief The RAMSES namespace contains all client side objects and
* functions used to implement RAMSES applications. RAMSES refers to these
* applications as clients.
*/
namespace ramses
{
    namespace internal
    {
        class ClientFactory;
        class RamsesClientImpl;
    }

    class Scene;
    class IClientEventHandler;

    /**
    * @brief Entry point of RAMSES client API.
    * @ingroup CoreAPI
    *
    * The RAMSES client class handles application state and and is a factory for scenes and client resources.
    */
    class RAMSES_API RamsesClient : public RamsesObject
    {
    public:
        /**
         * @brief Create a new empty Scene. The scene is published local-only (#ramses::EScenePublicationMode::LocalOnly)
         *
         * @param[in] sceneId The scene id for global identification of the Scene (is used for scene mapping on renderer side).
         * @param[in] name The optional name of the created Scene.
         * @return A pointer to the created Scene, null on failure
         */
        Scene* createScene(sceneId_t sceneId, std::string_view name = {});

        /**
        * @brief Create a new empty Scene.
        *
        * @param[in] sceneConfig The scene configuration, see #ramses::SceneConfig for details. A unique sceneId must be provided.
        * @param[in] name The optional name of the created Scene.
        * @return A pointer to the created Scene, null on failure
        */
        Scene* createScene(const SceneConfig& sceneConfig, std::string_view name = {});

        /**
        * @brief Loads scene contents and resources from a file.
        *
        * The file format has to match current Ramses SDK version in major and minor version number.
        * This method is not back compatible and will fail
        * if trying to load scene files saved using older Ramses SDK version.
        *
        * @param[in] fileName File name to load the scene from.
        * @param[in] config optional configuration object to override default behavior, see #ramses::SceneConfig for details
        * @return New instance of scene with contents loaded from a file.
        */
        Scene* loadSceneFromFile(std::string_view fileName, const SceneConfig& config = {});

        /**
        * @brief Loads scene contents and resources from a memory buffer.
        *
        * The file format has to match current Ramses SDK version in major and minor version number. This method is not
        * back compatible and will fail if trying to load scene files saved using older Ramses SDK version.
        *
        * Ramses takes ownership of the memory buffer passed in via data and will delete it via the provided deleter from
        * unique_ptr when not used anymore. The caller may not modify the referenced memory anymore after this call.
        * The behavior is undefined if data does not contain a complete serialized ramses scene or if size does not
        * match the size of the scene data in bytes.
        *
        * The deleter on data allows safe memory ownership passing on windows when ramses is used as dll. For more
        * details and a convenience wrapper see #ramses::RamsesUtils::LoadSceneFromMemory.
        *
        * @param[in] data Memory buffer to load the scene from.
        * @param[in] size The size in bytes of the data memory.
        * @param[in] config optional configuration object to override default behavior, see #ramses::SceneConfig for details
        * @return New instance of scene with contents loaded from a file.
        */
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        Scene* loadSceneFromMemory(std::unique_ptr<std::byte[], void (*)(const std::byte*)> data, size_t size, const SceneConfig& config = {});

        /**
        * @brief Loads scene contents and resources from an open file descriptor.
        *
        * The file format has to match current Ramses SDK version in major and minor version number.
        * This method is not back compatible and will fail
        * if trying to load scene files saved using older Ramses SDK version.
        *
        * The ramses scene must be in the already opened filedescriptor at absolute position offset within
        * the file. The filedescriptor must be opened for read access and may not be modified anymore after
        * this call. The filedescriptor must support seeking.
        * Ramses takes ownership of the filedescriptor and will close it when not needed anymore.
        *
        * The behavior is undefined if the filedescriptor does not contain a complete serialized ramses scene
        * at offset.
        *
        * @param[in] fd Open and readable filedescriptor.
        * @param[in] offset Absolute starting position of ramses scene within fd.
        * @param[in] length Size of the scene data within fd.
        * @param[in] config optional configuration object to override default behavior, see #ramses::SceneConfig for details
        * @return New instance of scene with contents loaded from a file.
        */
        Scene* loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, const SceneConfig& config = {});

        /**
        * @brief Loads scene contents and resources asynchronously from a file.
        *        The file format has to match current Ramses SDK version in major and minor version number.
        *        This method is not backwards compatible and will fail
        *        if trying to load scene files saved using older Ramses SDK version.
        *
        *        This method does not directly return a scene but provides its results
        *        after loading is finished when calling dispatchEvents().
        *        There will be one event for each loaded resource file followed by an
        *        event for the scene file.
        *
        * @param[in] fileName File name to load the scene from.
        * @param[in] config optional configuration object to override default behavior, see #ramses::SceneConfig for details
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool loadSceneFromFileAsync(std::string_view fileName, const SceneConfig& config = {});

        /**
        * Attempts to parse feature level from a Ramses scene file.
        *
        * @param[in] fileName file path to Ramses scene file
        * @param[out] detectedFeatureLevel feature level detected in given file (valid only if parsing successful!)
        * @return true if parsing was successful, false otherwise.
        */
        static bool GetFeatureLevelFromFile(std::string_view fileName, EFeatureLevel& detectedFeatureLevel);

        /**
        * Attempts to parse feature level from a Ramses scene file.
        *
        * @param[in] fd open and readable file descriptor
        * @param[in] offset absolute starting position of ramses scene within \c fd
        * @param[in] length size of the scene data within \c fd
        * @param[out] detectedFeatureLevel feature level detected in given file (valid only if parsing successful!)
        * @return true if parsing was successful, false otherwise.
        */
        static bool GetFeatureLevelFromFile(int fd, size_t offset, size_t length, EFeatureLevel& detectedFeatureLevel);

        /**
        * @brief Destroys the given Scene. The reference of Scene is invalid after this call
        *
        * @param[in] scene The Scene to destroy
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool destroy(Scene& scene);

        /**
        * @brief Find a scene from the client by name.
        *
        * @param[in] name The name of the scene to find.
        * @return Pointer to the scene if found, nullptr otherwise.
        */
        [[nodiscard]] const Scene* findSceneByName(std::string_view name) const;

        /**
        * @copydoc findSceneByName(std::string_view) const
        **/
        [[nodiscard]] Scene* findSceneByName(std::string_view name);

        /**
        * @brief Get a scene from the client by scene id.
        *
        * @param[in] sceneId The id of the scene to get.
        * @return Pointer to the scene if found, nullptr otherwise.
        */
        [[nodiscard]] const Scene* getScene(sceneId_t sceneId) const;

        /**
        * @copydoc getScene(sceneId_t) const
        **/
        [[nodiscard]] Scene* getScene(sceneId_t sceneId);

        /**
        * @brief Some methods on the client provide asynchronous results. These can be synchronously
        *        received by calling this functions at regular intervals.
        *
        *        The point in time and the ordering of asynchronous events from different calls is
        *        unspecified. The ordering of events from one asynchronous load call is as specified
        *        in the documentation for that call.
        *
        * @param[in] clientEventHandler User class that implements the callbacks that can be triggered if a corresponding event happened.
        *                               Check IClientEventHandler documentation for more details.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool dispatchEvents(IClientEventHandler& clientEventHandler);

        /**
        * @brief Get #ramses::RamsesFramework which was used to create this #ramses::RamsesClient instance.
        * @return Reference to #ramses::RamsesFramework which was used to create this #ramses::RamsesClient instance.
        */
        [[nodiscard]] const RamsesFramework& getRamsesFramework() const;
        /** @copydoc getRamsesFramework() const */
        [[nodiscard]] RamsesFramework& getRamsesFramework();

        /**
         * Get the internal data for implementation specifics of RamsesClient.
         */
        [[nodiscard]] internal::RamsesClientImpl& impl();

        /**
         * Get the internal data for implementation specifics of RamsesClient.
         */
        [[nodiscard]] const internal::RamsesClientImpl& impl() const;

    protected:
        /**
        * Stores internal data for implementation specifics of the API.
        */
        internal::RamsesClientImpl& m_impl;

    private:
        /**
         * @brief Constructor of RamsesClient
         */
        explicit RamsesClient(std::unique_ptr<internal::RamsesClientImpl> impl);

        /**
        * @brief ClientFactory is the factory for RamsesClient
        */
        friend class internal::ClientFactory;
    };
}
