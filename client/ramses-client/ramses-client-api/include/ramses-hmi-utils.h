//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_HMI_UTILS_H
#define RAMSES_RAMSES_HMI_UTILS_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/APIExport.h"
#include "ramses-client-api/Scene.h"
#include <string>

namespace ramses
{
    class Node;
    class Scene;
    class Vector3fArray;

    /**
    * @brief A struct for representing a bounding sphere - see the BoundingSphereCollection class for usage.
    **/
    struct BoundingSphere
    {
        /**
        * @brief The x coordinate of the center.
        */
        float xPos;

        /**
        * @brief The y coordinate of the center.
        */
        float yPos;

        /**
        * @brief The z coordinate of the center.
        */
        float zPos;

        /**
        * @brief The radius of the sphere.
        */
        float radius;
    };

    /**
    * @brief A class for representing a collection of bounding spheres for the nodes in a scene.
    **/
    class RAMSES_API BoundingSphereCollection
    {
    public:

        /**
        * @brief Construct a new BoundingSphereCollection to be used for the given Scene.
        *
        * @param[in] scene The target scene.
        */
        BoundingSphereCollection(const Scene& scene);

        /**
        * @brief Set (or override) a bounding sphere for a given node.
        *
        * @param[in] node target node for the bounding sphere. The node must belong to the scene passed in the constructor.
        * @param[in] sphere the bounding sphere, represented in object space of the node.
        */
        void setBoundingSphere(const Node& node, const BoundingSphere sphere);

        /**
        * @brief Remove a bounding sphere for a given node.
        *
        * @param[in] node target node for the bounding sphere. The method will do nothing in case a bounding sphere did not exist.
        */
        void removeBoundingSphere(const Node& node);

        /**
        * @brief Computes a bounding sphere for a given node.
        *
        * This method is used to compute a bounding sphere of a subtree based on the following criteria:
        *
        * 1) The given node has a bounding sphere set => The sphere is returned.
        * 2) The given node has no bounding sphere set, but one of more child-nodes has bounding spheres => The returned sphere
        *    will be a computed bounding sphere enclosing all bounding spheres found on child-nodes.
        * 3) In case of none of the above, a bounding sphere with all members set to 0.f will be returned.
        *
        * REMARK: The computed bounding sphere is transformed into world space.
        *
        * @param[in] startNode initial node for the computation.
        * @result BoundingSphere the resulting bounding sphere.
        */
        BoundingSphere computeBoundingSphereInWorldSpace(const Node& startNode) const;

        /**
        * @brief Computes a bounding sphere for an array of vertices. The input array is assumed to be X,Y,Z positions.
        *
        * @param[in] vertices the input vertices.
        * @result BoundingSphere the resulting bounding sphere.
        */
        static BoundingSphere ComputeBoundingSphereForVertices(const Vector3fArray& vertices);

        /**
        * @brief Destructor for BoundingSphereCollection
        */
        ~BoundingSphereCollection();

        /**
        * @brief Internal data
        */
        class BoundingSphereCollectionImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        BoundingSphereCollection(const BoundingSphereCollection& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        BoundingSphereCollection& operator=(const BoundingSphereCollection& other) = delete;
    };

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
