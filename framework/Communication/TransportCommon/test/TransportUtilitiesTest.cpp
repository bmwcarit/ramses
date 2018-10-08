//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "framework_common_gmock_header.h"
#include "TransportCommon/TransportUtilities.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "gmock/gmock-generated-nice-strict.h"

namespace ramses_internal
{
    using namespace testing;

    struct SplitSceneActionsToChunksMock
    {
        virtual ~SplitSceneActionsToChunksMock() = default;
        MOCK_METHOD3(call, void(Pair<UInt32, UInt32>, Pair<const Byte*, const Byte*>, bool));
    };

    class TransportUtilitiesTest : public ::testing::Test
    {
    public:
        TransportUtilitiesTest()
            : sizeOfOneSceneAction(100u)
            , numberOfSceneActions(5u)
            , dataBase(nullptr)
        {
            chunkActionsCallback = [this](Pair<UInt32, UInt32> actionRange, Pair<const Byte*, const Byte*> dataRange, bool isIncomplete)
            {
                chunkActionsMock.call(actionRange, dataRange, isIncomplete);
            };

            for (UInt32 i = 0; i < numberOfSceneActions; ++i)
            {
                sceneActions.addRawSceneActionInformation(ESceneActionId_TestAction, i*sizeOfOneSceneAction);
            }
            Vector<Byte> data(numberOfSceneActions*sizeOfOneSceneAction);
            sceneActions.appendRawData(data.data(), data.size());
            dataBase = sceneActions.collectionData().data();

            singleSceneAction.addRawSceneActionInformation(ESceneActionId_TestAction, 0);
            data.resize(sizeOfOneSceneAction);
            singleSceneAction.appendRawData(data.data(), data.size());
            dataBaseSingle = singleSceneAction.collectionData().data();
        }

    protected:
        const UInt32 sizeOfOneSceneAction;
        const UInt32 numberOfSceneActions;
        SceneActionCollection sceneActions;
        const Byte* dataBase;
        SceneActionCollection singleSceneAction;
        const Byte* dataBaseSingle;
        StrictMock<SplitSceneActionsToChunksMock> chunkActionsMock;
        std::function<void(Pair<UInt32, UInt32>, Pair<const Byte*, const Byte*>, bool)> chunkActionsCallback;
    };

    TEST_F(TransportUtilitiesTest, getSingleCallbackForSingleAction)
    {
        SceneActionCollection actions;
        SceneActionCollectionCreator creator(actions);
        creator.allocateNode(0, NodeHandle(1u));
        const Byte* data = actions.collectionData().data();
        const UInt32 dataSize = static_cast<UInt32>(actions.collectionData().size());

        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 1u), MakePair(data, data + dataSize), false));
        TransportUtilities::SplitSceneActionsToChunks(actions, 10u, dataSize, chunkActionsCallback);
    }

    TEST_F(TransportUtilitiesTest, getSingleCallbackForAllActionsWhenSendSizesLargeEnough)
    {
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 5u), MakePair(dataBase, dataBase + 500u), false));
        TransportUtilities::SplitSceneActionsToChunks(sceneActions, sceneActions.numberOfActions(), static_cast<UInt32>(sceneActions.collectionData().size()), chunkActionsCallback);
    }

    TEST_F(TransportUtilitiesTest, getMultipleCallbacksWhenNumberOfActionsIsTooSmall)
    {
        InSequence seq;
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 3u), MakePair(dataBase, dataBase + 300u), false));
        EXPECT_CALL(chunkActionsMock, call(MakePair(3u, 5u), MakePair(dataBase + 300u, dataBase + 500u), false));
        TransportUtilities::SplitSceneActionsToChunks(sceneActions, 3, 600u, chunkActionsCallback);
    }

    TEST_F(TransportUtilitiesTest, getMultipleSingleActionsWhenNumberLimitSetToOne)
    {
        InSequence seq;
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 1u), MakePair(dataBase + 0,    dataBase + 100u), false));
        EXPECT_CALL(chunkActionsMock, call(MakePair(1u, 2u), MakePair(dataBase + 100u, dataBase + 200u), false));
        EXPECT_CALL(chunkActionsMock, call(MakePair(2u, 3u), MakePair(dataBase + 200u, dataBase + 300u), false));
        EXPECT_CALL(chunkActionsMock, call(MakePair(3u, 4u), MakePair(dataBase + 300u, dataBase + 400u), false));
        EXPECT_CALL(chunkActionsMock, call(MakePair(4u, 5u), MakePair(dataBase + 400u, dataBase + 500u), false));
        TransportUtilities::SplitSceneActionsToChunks(sceneActions, 1, 600u, chunkActionsCallback);
    }

    TEST_F(TransportUtilitiesTest, getMultipleCompleteActionsWhenDataSizeFitsExactly)
    {
        InSequence seq;
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 2u), MakePair(dataBase, dataBase + 200u), false));
        EXPECT_CALL(chunkActionsMock, call(MakePair(2u, 4u), MakePair(dataBase + 200u, dataBase + 400u), false));
        EXPECT_CALL(chunkActionsMock, call(MakePair(4u, 5u), MakePair(dataBase + 400u, dataBase + 500u), false));
        TransportUtilities::SplitSceneActionsToChunks(sceneActions, 10u, 200u, chunkActionsCallback);
    }

    TEST_F(TransportUtilitiesTest, getMultipleCallbacksWhenSizeOfActionArrayIsTooSmall)
    {
        InSequence seq;
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 4u), MakePair(dataBase, dataBase + 399u), true));
        EXPECT_CALL(chunkActionsMock, call(MakePair(3u, 5u), MakePair(dataBase + 399u, dataBase + 500u), false));
        TransportUtilities::SplitSceneActionsToChunks(sceneActions, 6, 399u, chunkActionsCallback);
    }

    TEST_F(TransportUtilitiesTest, getMultipleIncompleteCallbacksWhenSizeTooSmall)
    {
        InSequence seq;
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 1u), MakePair(dataBaseSingle, dataBaseSingle + 40u), true));
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 1u), MakePair(dataBaseSingle + 40u, dataBaseSingle + 80u), true));
        EXPECT_CALL(chunkActionsMock, call(MakePair(0u, 1u), MakePair(dataBaseSingle + 80u, dataBaseSingle + 100u), false));
        TransportUtilities::SplitSceneActionsToChunks(singleSceneAction, 3, 40u, chunkActionsCallback);
    }
}
