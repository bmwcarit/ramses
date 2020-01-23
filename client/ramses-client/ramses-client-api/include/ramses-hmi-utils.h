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
        static void DumpUnrequiredSceneObjectsToFile(const Scene& scene, std::ofstream& out);
    };
}

#endif
