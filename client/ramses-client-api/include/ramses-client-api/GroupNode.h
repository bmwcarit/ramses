//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GROUPNODE_H
#define RAMSES_GROUPNODE_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/Node.h"

namespace ramses
{
    /**
     * @brief The GroupNode is an invisible node used for grouping
     * nodes within the scene graph.
    */
    class RAMSES_API GroupNode : public Node
    {
    protected:
        /**
        * @brief Scene is the factory for creating GroupNode instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for GroupNode.
        *
        * @param[in] pimpl Internal data for implementation specifics of GroupNode (sink - instance becomes owner)
        */
        explicit GroupNode(NodeImpl& pimpl);

        /**
        * @brief Copy constructor of Scene
        *
        * @param[in] other Other instance of GroupNode class
        */
        GroupNode(const GroupNode& other);

        /**
        * @brief Assignment operator of Scene.
        *
        * @param[in] other Other instance of GroupNode class
        * @return This instance after assignment
        */
        GroupNode& operator=(const GroupNode& other);

        /**
        * @brief Destructor of the Scene
        */
        virtual ~GroupNode();
    };
}

#endif
