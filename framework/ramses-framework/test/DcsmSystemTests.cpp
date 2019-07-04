//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/DcsmConsumer.h"
#include "ramses-framework-api/DcsmProvider.h"

#include "DcsmEventHandlerMocks.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace ramses
{
    using namespace testing;

    class ADcsmSystem : public Test
    {
    protected:
        ADcsmSystem()
            : framework(RamsesFrameworkConfig())
            , consumer(*framework.createDcsmConsumer())
            , provider(*framework.createDcsmProvider())
        {
        }

        void TearDown()
        {
            framework.destroyDcsmConsumer(consumer);
            framework.destroyDcsmProvider(provider);
        }

        void dispatch()
        {
            // do this excessively to unveil potential wrong calls of eventHandlers
            EXPECT_EQ(consumer.dispatchEvents(consHandler), StatusOK);
            EXPECT_EQ(provider.dispatchEvents(provHandler), StatusOK);
            EXPECT_EQ(consumer.dispatchEvents(consHandler), StatusOK);
            EXPECT_EQ(provider.dispatchEvents(provHandler), StatusOK);
        }

        void offerContent(ContentID id_, Category cat_, sceneId_t scene_)
        {
            EXPECT_CALL(consHandler, contentOffered(id_, cat_));
            EXPECT_EQ(provider.offerContent(id_, cat_, scene_), StatusOK);
            dispatch();
        }

        void assignContentToConsumer(ContentID id_, SizeInfo size_, AnimationInformation anim_)
        {
            EXPECT_CALL(provHandler, contentSizeChange(id_, size_, anim_));
            EXPECT_EQ(consumer.assignContentToConsumer(id_, size_), StatusOK);
            dispatch();
        }

        void showContent(ContentID id_, AnimationInformation anim_)
        {
            EXPECT_CALL(provHandler, contentShow(id_, anim_));
            EXPECT_EQ(consumer.contentStateChange(id_, EDcsmState::Shown, anim_), StatusOK);
            dispatch();
        }

        void hideContent(ContentID id_, AnimationInformation anim_)
        {
            EXPECT_CALL(provHandler, contentHide(id_, anim_));
            EXPECT_EQ(consumer.contentStateChange(id_, EDcsmState::Ready, anim_), StatusOK);
            dispatch();
        }

        void stopOfferByProvider(ContentID id_, AnimationInformation anim_)
        {
            EXPECT_CALL(consHandler, contentStopOfferRequest(id_));
            EXPECT_EQ(provider.requestStopOfferContent(id_), StatusOK);
            dispatch();

            EXPECT_CALL(provHandler, stopOfferAccepted(id_, anim_));
            EXPECT_EQ(consumer.acceptStopOffer(id, anim_), StatusOK);
            dispatch();
        }

        void releaseContent(ContentID id_, AnimationInformation anim_)
        {
            EXPECT_CALL(provHandler, contentRelease(id_, anim_));
            EXPECT_EQ(consumer.contentStateChange(id_, EDcsmState::Assigned, anim_), StatusOK);
            dispatch();
        }

        void unassignConsumer(ContentID id_, AnimationInformation anim_)
        {
            EXPECT_CALL(provHandler, contentRelease(id_, anim_));
            EXPECT_EQ(consumer.contentStateChange(id_, EDcsmState::Offered, anim_), StatusOK);
            dispatch();
        }

        void requestAndMarkReady(ContentID id_, sceneId_t sceneId_)
        {
            EXPECT_CALL(provHandler, contentReadyRequested(id_));
            EXPECT_EQ(consumer.contentStateChange(id_, EDcsmState::Ready, AnimationInformation()), StatusOK);
            dispatch();
            EXPECT_CALL(consHandler, contentReady(id_, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(sceneId_)));
            EXPECT_EQ(provider.markContentReady(id_), StatusOK);
            dispatch();
        }

        RamsesFramework framework;
        DcsmConsumer& consumer;
        DcsmProvider& provider;

        StrictMock<ramses_internal::DcsmConsumerEventHandlerMock> consHandler;
        StrictMock<DcsmProviderEventHandlerMock> provHandler;

        ContentID id = ContentID(123);
        SizeInfo size{ 800, 600 };
    };

    TEST_F(ADcsmSystem, canDoAFullContentLifecycle)
    {
        offerContent(id, Category(111), sceneId_t(18));
        EXPECT_EQ(provider.markContentReady(id), StatusOK);

        assignContentToConsumer(id, size, AnimationInformation());

        EXPECT_CALL(consHandler, contentReady(id, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor(18)));
        EXPECT_EQ(consumer.contentStateChange(id, EDcsmState::Ready, AnimationInformation()), StatusOK);
        dispatch();

        showContent(id, AnimationInformation{ 200, 300 });
        hideContent(id, AnimationInformation{ 200, 300 });
        showContent(id, AnimationInformation{ 200, 300 });

        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }

    TEST_F(ADcsmSystem, canDoAFullContentLifecycleLateMarkReady)
    {
        offerContent(id, Category(111), sceneId_t(18));
        assignContentToConsumer(id, size, AnimationInformation());

        requestAndMarkReady(id, sceneId_t(18));

        showContent(id, AnimationInformation{ 200, 300 });
        hideContent(id, AnimationInformation{ 200, 300 });
        showContent(id, AnimationInformation{ 200, 300 });

        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }

    TEST_F(ADcsmSystem, allowsToAssignAndUnassignToSameContentRepeatedly)
    {
        offerContent(id, Category(111), sceneId_t(18));
        assignContentToConsumer(id, size, AnimationInformation());
        requestAndMarkReady(id, sceneId_t(18));
        showContent(id, AnimationInformation{ 200, 300 });

        for (int i = 0; i < 3; ++i)
        {
            unassignConsumer(id, AnimationInformation{ 200, 300 });
            assignContentToConsumer(id, size, AnimationInformation());
            requestAndMarkReady(id, sceneId_t(18));
            showContent(id, AnimationInformation{ 200, 300 });
        }
        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }

    TEST_F(ADcsmSystem, allowsToRereadyTheSameContentRepeatedly)
    {
        offerContent(id, Category(111), sceneId_t(18));
        assignContentToConsumer(id, size, AnimationInformation());
        requestAndMarkReady(id, sceneId_t(18));
        showContent(id, AnimationInformation{ 200, 300 });

        for (int i = 0; i < 3; ++i)
        {
            releaseContent(id, AnimationInformation{ 200, 300 });
            requestAndMarkReady(id, sceneId_t(18));
            showContent(id, AnimationInformation{ 200, 300 });
        }
        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }
}
