//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesObjectTypes.h"
#include "ramses/framework/APIExport.h"
#include <memory>

namespace ramses
{
    namespace internal
    {
        class SceneGraphIteratorImpl;
    }

    /// Tree traversal style
    enum class ETreeTraversalStyle
    {
        DepthFirst,    //(with pre-order)
        BreadthFirst
    };

    class Node;

    /**
    * @ingroup CoreAPI
    * @brief A SceneObjectIterator can iterate through the nodes in the scene graph with the order specified as traversal style
    */
    class RAMSES_API SceneGraphIterator
    {
    public:
        /**
        * @brief Constructor for SceneGraphIterator.
        * A SceneObjectIterator can iterate through the nodes in the scene graph with the order specified as traversal style
        *
        * @param[in] startNode root node of the (sub)tree that should be traversed
        * @param[in] traversalStyle traversal style that should be used
        * @param[in] objectType Type of objects to iterate through
        */
        SceneGraphIterator(Node& startNode, ETreeTraversalStyle traversalStyle, ERamsesObjectType objectType = ERamsesObjectType::Node);

        /**
        * @brief Destructor.
        */
        ~SceneGraphIterator();

        /**
        * @brief Returns the next node while iterating.
        *
        * @return next node, null if no more objects available
        *
        * Iterator is invalid and may no longer be used if any nodes are added or removed.
        */
        Node* getNext();

    protected:
        /**
        * @brief Stores internal data for implementation specifics of SceneGraphIterator.
        */
        std::unique_ptr<internal::SceneGraphIteratorImpl> m_impl;
    };
}

