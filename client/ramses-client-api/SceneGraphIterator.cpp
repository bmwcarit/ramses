//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "ramses-client-api/SceneGraphIterator.h"

// internal
#include "SceneGraphIteratorImpl.h"

namespace ramses
{
    SceneGraphIterator::SceneGraphIterator(Node& startNode, ETreeTraversalStyle traversalStyle, ERamsesObjectType objectType)
        : impl(new SceneGraphIteratorImpl(startNode, traversalStyle, objectType))
    {
    }

    Node* SceneGraphIterator::getNext()
    {
        return impl->getNext();
    }

    SceneGraphIterator::~SceneGraphIterator()
    {
        delete impl;
    }

}

