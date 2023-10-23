//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneGraphIteratorImpl.h"
#include "ramses/client/Node.h"
#include "impl/NodeImpl.h"
#include "impl/RamsesObjectTypeUtils.h"

#include <cstdint>

namespace ramses::internal
{
    SceneGraphIteratorImpl::SceneGraphIteratorImpl(Node& startNode, ETreeTraversalStyle traversalStyle, ERamsesObjectType objectFilterType)
        : m_traversalStyle(traversalStyle)
        , m_objectFilterType(objectFilterType)
    {
        m_nodesToExpand.push_back(&startNode.impl());
    }

    Node* SceneGraphIteratorImpl::getNext()
    {
        NodeImpl* next = nullptr;

        do
        {
            next = getNextInternal();
        } while (next != nullptr && !next->isOfType(m_objectFilterType));
        if (nullptr == next)
        {
            return nullptr;
        }
        return &RamsesObjectTypeUtils::ConvertTo<Node>(next->getRamsesObject());
    }

    void SceneGraphIteratorImpl::expandBreadthFirst(NodeImpl& node)
    {
        const size_t childCount = node.getChildCount();
        for (size_t i = 0; i < childCount; ++i)
        {
            m_nodesToExpand.push_back(&node.getChildImpl(i));
        }
    }

    void SceneGraphIteratorImpl::expandDepthFirst(NodeImpl& node)
    {
        const auto childCount = static_cast<int32_t>(node.getChildCount());
        for (int32_t i = childCount - 1; i >= 0; --i)
        {
            m_nodesToExpand.push_front(&node.getChildImpl(i));
        }
    }

    NodeImpl* SceneGraphIteratorImpl::getNextInternal()
    {
        if (m_nodesToExpand.empty())
        {
            return nullptr;
        }

        NodeImpl* next = m_nodesToExpand.front();
        m_nodesToExpand.pop_front();

        switch (m_traversalStyle)
        {
        case ETreeTraversalStyle::BreadthFirst:
            expandBreadthFirst(*next);
            break;
        case ETreeTraversalStyle::DepthFirst:
            expandDepthFirst(*next);
            break;
        }

        return next;
    }
}
