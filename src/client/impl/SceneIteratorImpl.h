//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/IteratorImpl.h"
#include "impl/RamsesClientImpl.h"
#include "ramses/framework/RamsesObjectTypes.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses
{
    class Scene;
}

namespace ramses::internal
{
    class SceneIteratorImpl : public IteratorImpl<Scene*>
    {
    public:
        explicit SceneIteratorImpl(const SceneVector& scenes)
            : IteratorImpl{ TransformToScenePtrs(scenes) }
        {
        }

    private:
        static std::vector<Scene*> TransformToScenePtrs(const SceneVector& scenes)
        {
            std::vector<Scene*> scenePtrs;
            scenePtrs.reserve(scenes.size());
            for (auto& s : scenes)
                scenePtrs.push_back(s.get());

            return scenePtrs;
        }
    };
}
