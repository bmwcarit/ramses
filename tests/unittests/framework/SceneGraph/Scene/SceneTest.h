//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#pragma once

#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "gtest/gtest.h"
#include "ActionTestScene.h"
#include "internal/SceneGraph/Scene/ResourceChangeCollectingScene.h"
#include "internal/SceneGraph/Scene/DataLayoutCachedScene.h"

namespace ramses::internal
{
    using SceneTypes = ::testing::Types<
        Scene,
        TransformationCachedScene,
        ActionCollectingScene,
        ResourceChangeCollectingScene,
        DataLayoutCachedScene,
        ActionTestScene
    >;

    template <typename SCENE>
    class AScene : public testing::Test
    {
    protected:
        SCENE m_scene;
    };

}
