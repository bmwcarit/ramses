//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "Scene/SceneActionUtils.h"
#include <algorithm>

namespace ramses_internal
{
    UInt32 SceneActionCollectionUtils::CountNumberOfActionsOfType(const SceneActionCollection& actions, ESceneActionId type)
    {
        auto p = [&type](const SceneActionCollection::SceneActionReader& reader)-> bool
            {
                return reader.type() == type;
            };

        return static_cast<UInt32>(std::count_if(actions.begin(),actions.end(), p));
    }

    UInt32 SceneActionCollectionUtils::CountNumberOfActionsOfType(const SceneActionCollection& actions, const SceneActionIdVector& types)
    {
        auto p = [&types](const SceneActionCollection::SceneActionReader& action)-> Bool
            {
                return contains_c(types, action.type());
            };

        return static_cast<UInt32>(std::count_if(actions.begin(),actions.end(), p));
    }
}
