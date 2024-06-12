//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/SceneActionCollectionCreator.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "internal/Components/FlushTimeInformation.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace ramses::internal
{
    class ASceneActionCollectionCreatorAndApplier : public ::testing::Test
    {
    public:
        ASceneActionCollectionCreatorAndApplier()
            : creator(collection, EFeatureLevel_Latest)
        {
        }

        SceneActionCollection collection;
        SceneActionCollectionCreator creator;
    };

    TEST_F(ASceneActionCollectionCreatorAndApplier, createsExpectedNumberAndTypeOfActions)
    {
        creator.allocateNode(0, NodeHandle(1u));
        creator.allocateRenderState(RenderStateHandle(2u));

        ASSERT_EQ(2u, collection.numberOfActions());
        EXPECT_EQ(ESceneActionId::AllocateNode, collection[0].type());
        EXPECT_EQ(ESceneActionId::AllocateRenderState, collection[1].type());
    }

}
