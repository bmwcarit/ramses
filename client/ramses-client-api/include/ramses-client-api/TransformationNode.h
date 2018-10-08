//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSFORMATIONNODE_H
#define RAMSES_TRANSFORMATIONNODE_H

#include "ramses-client-api/Node.h"

namespace ramses
{
    class NodeImpl;

    /**
    * @brief The TransformationNode is a node that can be used for transformations (rotating, scaling,
    * positioning) a subtree in the scene graph. The transformation will affect all child
    * nodes of the TransformationNode.
    *
    * This class is OBSOLETE and will be removed in next breaking RAMSES version.
    * Use GroupNode/MeshNode instead, they all can be fully transformed.
    *
    */
    class RAMSES_API TransformationNode : public Node
    {
    protected:
        /**
        * @brief Scene is the factory for creating TransformationNode instances.
        */
        friend class SceneImpl;

        /**
        * @brief Custom pimpl constructor
        *
        * @param[in] pimpl Implementation specifics of the node (sink - instance becomes owner)
        */
        explicit TransformationNode(NodeImpl& pimpl);

        /**
        * @brief Assignment operator of TransformationNode.
        *
        * @param[in] other Other instance of TransformationNode class
        * @return This instance after assignment
        */
        TransformationNode& operator=(const TransformationNode& other);
    };
}

#endif
