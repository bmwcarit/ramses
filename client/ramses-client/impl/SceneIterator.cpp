//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/SceneIterator.h"
#include "SceneIteratorImpl.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RamsesClient.h"
#include "RamsesClientImpl.h"

namespace ramses
{
    SceneIterator::SceneIterator(const RamsesClient& client)
        : m_impl{ std::make_unique<SceneIteratorImpl>(client.m_impl.getListOfScenes()) }
    {
    }

    SceneIterator::~SceneIterator() = default;

    Scene* SceneIterator::getNext()
    {
        return m_impl->getNext();
    }
}
