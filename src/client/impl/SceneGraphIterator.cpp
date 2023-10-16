//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "ramses/client/SceneGraphIterator.h"

// internal
#include "SceneGraphIteratorImpl.h"

namespace ramses
{
    SceneGraphIterator::SceneGraphIterator(Node& startNode, ETreeTraversalStyle traversalStyle, ERamsesObjectType objectType)
        : m_impl{ std::make_unique<internal::SceneGraphIteratorImpl>(startNode, traversalStyle, objectType) }
    {
    }

    SceneGraphIterator::~SceneGraphIterator() = default;

    Node* SceneGraphIterator::getNext()
    {
        return m_impl->getNext();
    }
}

