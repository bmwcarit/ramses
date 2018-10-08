//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ROTATENODE_H
#define RAMSES_ROTATENODE_H

#include "ramses-client-api/Node.h"

namespace ramses
{
    class NodeImpl;

    /**
    * @brief The RotateNode is a node used for rotating
    * a subtree in the scene graph. The rotation will affect all child
    * nodes of the RotateNode.
    *
    * This class is OBSOLETE and will be removed in next breaking RAMSES version.
    * Use GroupNode/MeshNode instead, they all can be fully transformed.
    *
    */
    class RAMSES_API RotateNode : public Node
    {
    protected:
        /**
        * @brief Scene is the factory for creating RotateNode instances.
        */
        friend class SceneImpl;

        /**
        * @brief Copy constructor of RotateNode
        *
        * @param[in] pimpl Internal data for implementation specifics of RotateNode (sink - instance becomes owner)
        */
        explicit RotateNode(NodeImpl& pimpl);

        /**
        * @brief Assignment operator of RotateNode.
        *
        * @param[in] other Other instance of RotateNode class
        * @return This instance after assignment
        */
        RotateNode& operator=(const RotateNode& other);
    };
}

#endif
