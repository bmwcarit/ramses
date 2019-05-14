//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEITERATORIMPL_H
#define RAMSES_SCENEITERATORIMPL_H

#include "IteratorImpl.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "Collections/Vector.h"

namespace ramses
{
    class Scene;

    class SceneIteratorImpl : public IteratorImpl<Scene*>
    {
    public:
        explicit SceneIteratorImpl(const std::vector<Scene*>& scenes)
            : IteratorImpl(scenes)
        {
        }
    };
}

#endif
