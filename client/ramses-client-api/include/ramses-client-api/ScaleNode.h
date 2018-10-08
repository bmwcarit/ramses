//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCALENODE_H
#define RAMSES_SCALENODE_H

#include "ramses-client-api/Node.h"

namespace ramses
{
    class NodeImpl;

    /**
    * @brief The ScaleNode is an invisible node used for scaling
    * a subtree in the scene graph. The scaling will affect all child
    * nodes of the ScaleNode.
    *
    * This class is OBSOLETE and will be removed in next breaking RAMSES version.
    * Use GroupNode/MeshNode instead, they all can be fully transformed.
    *
    */
    class RAMSES_API ScaleNode : public Node
    {
    protected:
        /**
        * @brief Scene is the factory for creating ScaleNode instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for ScaleNode.
        *
        * @param[in] pimpl Internal data for implementation specifics of ScaleNode (sink - instance becomes owner)
        */
        explicit ScaleNode(NodeImpl& pimpl);

        /**
        * @brief Assignment operator of ScaleNode.
        *
        * @param[in] other Other instance of ScaleNode class
        * @return This instance after assignment
        */
        ScaleNode& operator=(const ScaleNode& other);
    };
}

#endif
