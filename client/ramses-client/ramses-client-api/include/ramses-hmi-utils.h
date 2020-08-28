//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_HMI_UTILS_H
#define RAMSES_RAMSES_HMI_UTILS_H

#include "ramses-framework-api/APIExport.h"
#include "ramses-client-api/Scene.h"
#include <string>

namespace ramses
{
    class ResourceDataPool;

    /**
    * @brief Utility functions especially created for functions not yet designated for direct API integration.
    **/
    class RAMSES_API RamsesHMIUtils
    {
    public:
        /**
         * @brief Checks if all resources for scene are locally available
         *
         * This method can be called to verify that at the time of calling all resources that are used inside scene are locally
         * available. That means this scene could be mapped on a renderer and the renderer would be able to retrieve all needed
         * resources.
         * It does not check for resources in remote providers or resource caches.
         *
         * @param[in] scene The scene to verify resources for
         * @result True when all resources are available, false if not
         */
        static bool AllResourcesForSceneKnown(const Scene& scene);

        /**
         * @brief Dumps all objects of a scene which do not contribute to the visual appearance of the scene on screen.
         * This includes disabled RenderPass-es, invisible MeshNode-s, client resources which are not used by the scene, and so on.
         * The output is in text form, starts with a list of all unrequired objects and their names and concludes with a
         * statistic (number of unrequired objects out of all objects of that type)
         *
         * @param[in] scene the source scene
         */
        static void DumpUnrequiredSceneObjects(const Scene& scene);

        /**
         * @brief As RamsesHMIUtils::DumpUnrequiredSceneObjects but write to given stream.
         *
         * @param[in] scene the source scene
         * @param[out] out stream to write to
         */
        static void DumpUnrequiredSceneObjectsToFile(const Scene& scene, std::ostream& out);

        /**
         * @brief Returns the ResourceDataPool for this client.
         * @deprecated This function and the returned class are deprecated. See #ramses::ResourceDataPool for details.
         *
         * @param[in] client The client to get the ResourceDataPool from.
         * return The reference to the ResourceDataPool.
         */
        static ResourceDataPool& GetResourceDataPoolForClient(RamsesClient& client);

        /**
         * @brief Saves all resources of a scene to a resource file, which can be opened in a ResourceDataPool.
         * @deprecated This functionality is deprecated. See #ramses::ResourceDataPool for details.
         *
         * @param[in] scene The scene to save resource of.
         * @param[in] filename The file path to write the resource file to.
         * @param[in] compress if set to true, resources might be compressed before saving
         *                     otherwise, uncompressed data will be saved
         * return True if operation succeeded.
         */
        static bool SaveResourcesOfSceneToResourceFile(Scene const& scene, std::string const& filename, bool compress);
    };
}

#endif
