//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ICLIENTEVENTHANDLER_H
#define RAMSES_ICLIENTEVENTHANDLER_H

#include "APIExport.h"

namespace ramses
{
    class Scene;

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
        * @brief Empty destructor
        */
        virtual ~IClientEventHandler() {}
    };
}

#endif
