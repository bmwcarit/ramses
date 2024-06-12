//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneActionCollection.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/SceneReferencing/SceneReferenceAction.h"
#include "ramses/framework/EFeatureLevel.h"
#include <memory>

namespace ramses::internal
{
    class IScene;
    struct ResourceChanges;
    struct SceneSizeInformation;
    class IResource;
    struct FlushTimeInformation;

    class SceneActionApplier
    {
    public:
        using ResourceVector = std::vector<std::unique_ptr<IResource>>;

        static void ApplyActionsOnScene(IScene& scene, const SceneActionCollection& actions, EFeatureLevel featureLevel);

    private:
        static void GetSceneSizeInformation(SceneActionCollection::SceneActionReader& action, SceneSizeInformation& sizeInfo, EFeatureLevel featureLevel);
        static void ApplySingleActionOnScene(IScene& scene, SceneActionCollection::SceneActionReader& action, EFeatureLevel featureLevel);
    };
}
