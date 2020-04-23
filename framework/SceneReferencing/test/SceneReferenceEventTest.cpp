//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneReferencing/SceneReferenceEvent.h"

#include <gtest/gtest.h>

namespace ramses_internal
{
    bool checkEqual(SceneReferenceEvent const& a, SceneReferenceEvent const& b)
    {
        return
            a.type == b.type &&
            a.referencedScene == b.referencedScene &&
            a.consumerScene == b.consumerScene &&
            a.providerScene == b.providerScene &&
            a.dataConsumer == b.dataConsumer &&
            a.dataProvider == b.dataProvider &&
            a.sceneState == b.sceneState &&
            a.tag == b.tag &&
            a.status == b.status;
    }

    TEST(ASceneReferenceEvent, hasSameValuesAfterWritingAndReading)
    {
        SceneReferenceEvent a(SceneId{ 123 });
        SceneReferenceEvent b(SceneId{ 123 });
        std::vector<Byte> testVec;

        a.writeToBlob(testVec);
        b.readFromBlob(testVec);
        EXPECT_TRUE(checkEqual(a, b));

        a.type = SceneReferenceEventType::SceneFlushed;
        a.referencedScene = SceneId{ 1234 };
        a.consumerScene = SceneId{ 2345 };
        a.providerScene = SceneId{ 3456 };
        a.dataConsumer = DataSlotId{ 9876 };
        a.dataProvider = DataSlotId{ 8765 };
        a.sceneState = RendererSceneState::Ready;
        a.tag = SceneVersionTag{ 10000 };
        a.status = false;

        EXPECT_FALSE(checkEqual(a, b));

        testVec.clear();
        a.writeToBlob(testVec);
        b.readFromBlob(testVec);
        EXPECT_TRUE(checkEqual(a, b));
    }
}
