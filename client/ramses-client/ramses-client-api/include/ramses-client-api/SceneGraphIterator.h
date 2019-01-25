//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEGRAPHITERATOR_H
#define RAMSES_SCENEGRAPHITERATOR_H

#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /// Tree traversal style
    enum ETreeTraversalStyle
    {
        ETreeTraversalStyle_DepthFirst = 0,    //(with pre-order)
        ETreeTraversalStyle_BreadthFirst
    };

    class Node;

    /**
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
        SceneGraphIterator(Node& startNode, ETreeTraversalStyle traversalStyle, ERamsesObjectType objectType = ERamsesObjectType_Node);

        /**
        * @brief Destructor
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
        * @brief Copy constructor of SceneGraphIterator
        *
        * @param[in] other Other instance of SceneGraphIterator class
        */
        SceneGraphIterator(const SceneGraphIterator& other);

        /**
        * @brief Assignment operator of SceneGraphIterator.
        *
        * @param[in] other Other instance of SceneGraphIterator class
        * @return This instance after assignment
        */
        SceneGraphIterator& operator=(const SceneGraphIterator& other);

        /**
        * @brief Stores internal data for implementation specifics of SceneGraphIterator.
        */
        class SceneGraphIteratorImpl* impl;
    };
}

#endif

