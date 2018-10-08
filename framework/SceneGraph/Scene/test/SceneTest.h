//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_SCENETEST_H
#define RAMSES_SCENETEST_H

#include "framework_common_gmock_header.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/TextureSampler.h"
#include "gtest/gtest.h"
#include "ActionTestScene.h"
#include "Scene/ResourceChangeCollectingScene.h"
#include "Scene/DataLayoutCachedScene.h"

namespace ramses_internal
{
    typedef ::testing::Types<
        Scene,
        TransformationCachedScene,
        ActionCollectingScene,
        ResourceChangeCollectingScene,
        DataLayoutCachedScene,
        ActionTestScene
    > SceneTypes;

    template <typename SCENE>
    class AScene : public testing::Test
    {
    protected:
        SCENE m_scene;
    };

}

#endif
