//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/SceneUtils/ResourceUtils.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/SceneGraph/SceneAPI/Renderable.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "internal/SceneGraph/SceneAPI/GeometryDataBuffer.h"
#include "internal/SceneGraph/SceneAPI/TextureBuffer.h"
#include "internal/SceneGraph/Scene/DataLayout.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    namespace ResourceUtils
    {
        void DiffResources(ResourceContentHashVector const& old, ResourceContentHashVector const& curr, ResourceChanges& changes)
        {
            assert(std::is_sorted(old.cbegin(), old.cend()));
            assert(std::is_sorted(curr.cbegin(), curr.cend()));
            std::set_difference(curr.begin(), curr.end(), old.begin(), old.end(), std::back_inserter(changes.m_resourcesAdded));
            std::set_difference(old.begin(), old.end(), curr.begin(), curr.end(), std::back_inserter(changes.m_resourcesRemoved));
        }
    }
}
