//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEGRAPHITERATORIMPL_H
#define RAMSES_SCENEGRAPHITERATORIMPL_H

#include "ramses-client-api/SceneGraphIterator.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include <deque>
#include "NodeImpl.h"

namespace ramses
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

#endif //RAMSES_SCENEGRAPHITERATORIMPL_H
