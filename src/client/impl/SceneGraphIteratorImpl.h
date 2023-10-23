//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/SceneGraphIterator.h"
#include "ramses/framework/RamsesObjectTypes.h"
#include <deque>
#include "impl/NodeImpl.h"

namespace ramses::internal
{
    class SceneGraphIteratorImpl
    {
    public:
        SceneGraphIteratorImpl(Node& startNode, ETreeTraversalStyle traversalStyle, ERamsesObjectType objectFilterType);
        Node* getNext();

    private:
        void expandBreadthFirst(NodeImpl& node);
        void expandDepthFirst(NodeImpl& node);
        NodeImpl* getNextInternal();

        ETreeTraversalStyle m_traversalStyle;
        std::deque<NodeImpl*> m_nodesToExpand;
        const ERamsesObjectType m_objectFilterType;
    };

}
