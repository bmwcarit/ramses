//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneGraphIteratorImpl.h"
#include "ramses-client-api/Node.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "NodeImpl.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses
{
    SceneGraphIteratorImpl::SceneGraphIteratorImpl(Node& startNode, ETreeTraversalStyle traversalStyle, ERamsesObjectType objectFilterType)
        : m_traversalStyle(traversalStyle)
        , m_objectFilterType(objectFilterType)
    {
        m_nodesToExpand.push_back(&startNode.m_impl);
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
        const uint32_t childCount = node.getChildCount();
        for (uint32_t i = 0; i < childCount; ++i)
        {
            m_nodesToExpand.push_back(&node.getChildImpl(i));
        }
    }

    void SceneGraphIteratorImpl::expandDepthFirst(NodeImpl& node)
    {
        const int32_t childCount = static_cast<int32_t>(node.getChildCount());
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
        case ETreeTraversalStyle_BreadthFirst:
            expandBreadthFirst(*next);
            break;
        case ETreeTraversalStyle_DepthFirst:
            expandDepthFirst(*next);
            break;
        default:
            assert(false);
        }

        return next;
    }
}
