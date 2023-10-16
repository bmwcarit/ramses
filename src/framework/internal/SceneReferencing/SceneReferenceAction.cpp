//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneReferencing/SceneReferenceAction.h"

namespace ramses::internal
{
    void SceneReferenceActionUtils::WriteToCollection(SceneReferenceActionVector const& actions, SceneActionCollection& collection)
    {
        collection.write(uint32_t(actions.size()));
        for (const auto& a : actions)
        {
            collection.write(a.type);
            collection.write(a.consumerScene);
            collection.write(a.consumerId);
            collection.write(a.providerScene);
            collection.write(a.providerId);
        }
    }

    void SceneReferenceActionUtils::ReadFromCollection(SceneReferenceActionVector& actions, SceneActionCollection::SceneActionReader& reader)
    {
        uint32_t count = 0;
        reader.read(count);
        actions.reserve(actions.size() + count);
        for (uint32_t i = 0u; i < count; ++i)
        {
            actions.push_back({});
            reader.read(actions.back().type);
            reader.read(actions.back().consumerScene);
            reader.read(actions.back().consumerId);
            reader.read(actions.back().providerScene);
            reader.read(actions.back().providerId);
        }
    }

    size_t SceneReferenceActionUtils::GetSizeEstimate(SceneReferenceActionVector const& actions)
    {
        return sizeof(uint32_t) + sizeof(SceneReferenceAction) * actions.size();
    }
}
