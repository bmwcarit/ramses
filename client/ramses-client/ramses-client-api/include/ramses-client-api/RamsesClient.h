//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESCLIENT_H
#define RAMSES_RAMSESCLIENT_H

#include "ramses-client-api/RamsesObject.h"
#include "ramses-client-api/SceneConfig.h"
#include "ramses-framework-api/RamsesFramework.h"

/**
* ramses namespace
*
* @brief The RAMSES namespace contains all client side objects and
* functions used to implement RAMSES applications. RAMSES refers to these
* applications as clients.
*/
namespace ramses
{
    class Scene;
    class IClientEventHandler;

    /**
    * @brief Entry point of RAMSES client API.
    *
    * The RAMSES client class handles application state and and is a factory for scenes and client resources.
    */
    class RAMSES_API RamsesClient : public RamsesObject
    {
    public:
        /**
        * @brief Create a new empty Scene.
        *
        * @param[in] sceneId The scene id for global identification of the Scene (is used for scene mapping on renderer side).
        * @param[in] sceneConfig The optional scene configuration that can be used to change parameters of scene to be created.
        * @param[in] name The optional name of the created Scene.
        * @return A pointer to the created Scene, null on failure
        */
        Scene* createScene(sceneId_t sceneId, const SceneConfig& sceneConfig = SceneConfig(), const char* name = nullptr);

        /**
        * @brief Loads scene contents and resources from a file.
        *
        * The file format has to match current Ramses SDK version in major and minor version number.
        * This method is not back compatible and will fail
        * if trying to load scene files saved using older Ramses SDK version.
        *
        * @param[in] fileName File name to load the scene from.
        * @param[in] localOnly Marks the scene to be loaded as valid for local only
        *                      optimization. This has the same effect as calling
        *                      SceneConfig::setPublicationMode(EScenePublicationMode_LocalOnly) before saving.
        * @return New instance of scene with contents loaded from a file.
        */
        Scene* loadSceneFromFile(const char* fileName, bool localOnly = false);

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
        * @param[in] localOnly Marks the scene to be loaded as valid for local only
        *                      optimization. This has the same effect as calling
        *                      SceneConfig::setPublicationMode(EScenePublicationMode_LocalOnly) before saving.
        * @return New instance of scene with contents loaded from a file.
        */
        Scene* loadSceneFromMemory(std::unique_ptr<unsigned char[], void(*)(const unsigned char*)> data, size_t size, bool localOnly = false);

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
        * @param[in] offset Absolute starting position of ramses scenen within fd.
        * @param[in] length Size of the scene data within fd.
        * @param[in] localOnly Marks the scene to be loaded as valid for local only
        *                      optimization. This has the same effect as calling
        *                      SceneConfig::setPublicationMode(EScenePublicationMode_LocalOnly) before saving.
        * @return New instance of scene with contents loaded from a file.
        */
        Scene* loadSceneFromFileDescriptor(int fd, size_t offset, size_t length, bool localOnly = false);

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
        * @param[in] localOnly Marks the scene to be loaded as valid for local only
        *                      optimization. This has the same effect as calling
        *                      SceneConfig::setPublicationMode(EScenePublicationMode_LocalOnly) before saving.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t loadSceneFromFileAsync(const char* fileName, bool localOnly = false);

        /**
        * @brief Destroys the given Scene. The reference of Scene is invalid after this call
        *
        * @param[in] scene The Scene to destroy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroy(Scene& scene);

        /**
        * @brief Find a scene from the client by name.
        *
        * @param[in] name The name of the scene to find.
        * @return Pointer to the scene if found, nullptr otherwise.
        */
        const Scene* findSceneByName(const char* name) const;

        /**
        * @copydoc findSceneByName(const char*) const
        **/
        Scene* findSceneByName(const char* name);

        /**
        * @brief Get a scene from the client by scene id.
        *
        * @param[in] sceneId The id of the scene to get.
        * @return Pointer to the scene if found, nullptr otherwise.
        */
        const Scene* getScene(sceneId_t sceneId) const;

        /**
        * @copydoc getScene(sceneId_t) const
        **/
        Scene* getScene(sceneId_t sceneId);

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
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t dispatchEvents(IClientEventHandler& clientEventHandler);

        /**
        * Stores internal data for implementation specifics of the API.
        */
        class RamsesClientImpl& impl;

        /**
         * @brief Constructor of RamsesClient
         */
        explicit RamsesClient(RamsesClientImpl&);

        /**
         * @brief Deleted default constructor
         */
        RamsesClient() = delete;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        RamsesClient(const RamsesClient& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RamsesClient& operator=(const RamsesClient& other) = delete;

    private:
        /**
        * @brief ClientFactory is the factory for RamsesClient
        */
        friend class ClientFactory;

        /**
        * @brief Destructor of RamsesClient
        */
        virtual ~RamsesClient();
    };
}

#endif
