//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VISIBILITYNODE_H
#define RAMSES_VISIBILITYNODE_H

#include "ramses-client-api/Node.h"

namespace ramses
{
    class NodeImpl;

    /**
    * @brief The VisibilityNode is a node used to trigger
    * visibility of all MeshNodes in the subtree of the VisibilityNode.
    * Changes are propagated (=sent to renderer) on flush() to save performance
    *
    * This class is OBSOLETE and will be removed in next breaking RAMSES version.
    * Use GroupNode/MeshNode instead, they also have a visibility property.
    *
    */
    class RAMSES_API VisibilityNode : public Node
    {
    protected:
        /**
        * @brief Scene is the factory for creating VisibilityNode instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for VisibilityNode.
        *
        * @param[in] pimpl Internal data for implementation specifics of VisibilityNode (sink - instance becomes owner)
        */
        explicit VisibilityNode(NodeImpl& pimpl);

        /**
        * @brief Assignment operator of VisibilityNode.
        *
        * @param[in] other Other instance of VisibilityNode class
        * @return This instance after assignment
        */
        VisibilityNode& operator=(const VisibilityNode& other);
    };
}

#endif
