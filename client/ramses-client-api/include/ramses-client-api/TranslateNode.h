//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSLATENODE_H
#define RAMSES_TRANSLATENODE_H

#include "ramses-client-api/Node.h"

namespace ramses
{
    class NodeImpl;

    /**
    * @brief The TranslateNode is a node used for translating
    * a subtree in the scene graph. The translation will affect all child
    * nodes of the TranslateNode.
    *
    * This class is OBSOLETE and will be removed in next breaking RAMSES version.
    * Use GroupNode/MeshNode instead, they all can be fully transformed.
    *
    */
    class RAMSES_API TranslateNode : public Node
    {
    protected:
        /**
        * @brief Scene is the factory for creating TranslateNode instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for TranslateNode.
        *
        * @param[in] pimpl Internal data for implementation specifics of TranslateNode (sink - instance becomes owner)
        */
        explicit TranslateNode(NodeImpl& pimpl);

        /**
        * @brief Assignment operator of TranslateNode.
        *
        * @param[in] other Other instance of TranslateNode class
        * @return This instance after assignment
        */
        TranslateNode& operator=(const TranslateNode& other);
    };
}

#endif
