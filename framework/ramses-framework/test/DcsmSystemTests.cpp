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

        void TearDown() override
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

        void offerContent(ContentID id_, Category cat_, sceneId_t scene_, EDcsmOfferingMode mode_)
        {
            EXPECT_CALL(consHandler, contentOffered(id_, cat_, ETechnicalContentType::RamsesSceneID));
            EXPECT_EQ(provider.offerContent(id_, cat_, scene_, mode_), StatusOK);
            dispatch();
        }

        void assignContentToConsumer(ContentID id_, const CategoryInfoUpdate& categoryInfo_, AnimationInformation anim_, sceneId_t sceneId_)
        {
            EXPECT_CALL(provHandler, contentSizeChange(id_, _, anim_)).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
                ramses::CategoryInfoUpdate update{categoryInfo_.getRenderSize(), categoryInfo_.getCategoryRect(), categoryInfo_.getSafeRect()};
                EXPECT_EQ(update, infoupdate);
                });
            EXPECT_CALL(consHandler, contentDescription(id_, TechnicalContentDescriptor{ sceneId_.getValue() }));
            EXPECT_EQ(consumer.assignContentToConsumer(id_, categoryInfo_), StatusOK);
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

        void requestAndMarkReady(ContentID id_)
        {
            EXPECT_CALL(provHandler, contentReadyRequested(id_));
            EXPECT_EQ(consumer.contentStateChange(id_, EDcsmState::Ready, AnimationInformation()), StatusOK);
            dispatch();
            EXPECT_CALL(consHandler, contentReady(id_));
            EXPECT_EQ(provider.markContentReady(id_), StatusOK);
            dispatch();
        }

        RamsesFramework framework;
        DcsmConsumer& consumer;
        DcsmProvider& provider;

        StrictMock<ramses_internal::DcsmConsumerEventHandlerMock> consHandler;
        StrictMock<DcsmProviderEventHandlerMock> provHandler;

        ContentID id = ContentID(123);
        CategoryInfoUpdate categoryInfo{ {0u, 0u}, { 0u, 0u, 800u, 600u } };
    };

    TEST_F(ADcsmSystem, localConsumerReceivesLocalAndLocalAndRemoteOffers)
    {
        offerContent(id, Category(111), sceneId_t(18), EDcsmOfferingMode::LocalAndRemote);
        EXPECT_EQ(provider.markContentReady(id), StatusOK);

        ContentID id2(124);
        offerContent(id2, Category(112), sceneId_t(19), EDcsmOfferingMode::LocalOnly);
        EXPECT_EQ(provider.markContentReady(id2), StatusOK);
    }

    TEST_F(ADcsmSystem, canDoAFullContentLifecycle)
    {
        offerContent(id, Category(111), sceneId_t(18), EDcsmOfferingMode::LocalAndRemote);
        EXPECT_EQ(provider.markContentReady(id), StatusOK);

        assignContentToConsumer(id, categoryInfo, AnimationInformation(), sceneId_t(18));

        EXPECT_CALL(consHandler, contentReady(id));
        EXPECT_EQ(consumer.contentStateChange(id, EDcsmState::Ready, AnimationInformation()), StatusOK);
        dispatch();

        showContent(id, AnimationInformation{ 200, 300 });
        hideContent(id, AnimationInformation{ 200, 300 });
        showContent(id, AnimationInformation{ 200, 300 });

        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }

    TEST_F(ADcsmSystem, canDoAFullContentLifecycleLateMarkReady)
    {
        offerContent(id, Category(111), sceneId_t(18), EDcsmOfferingMode::LocalAndRemote);
        assignContentToConsumer(id, categoryInfo, AnimationInformation(), sceneId_t(18));

        requestAndMarkReady(id);

        showContent(id, AnimationInformation{ 200, 300 });
        hideContent(id, AnimationInformation{ 200, 300 });
        showContent(id, AnimationInformation{ 200, 300 });

        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }

    TEST_F(ADcsmSystem, allowsToAssignAndUnassignToSameContentRepeatedly)
    {
        offerContent(id, Category(111), sceneId_t(18), EDcsmOfferingMode::LocalAndRemote);
        assignContentToConsumer(id, categoryInfo, AnimationInformation(), sceneId_t(18));
        requestAndMarkReady(id);
        showContent(id, AnimationInformation{ 200, 300 });

        for (int i = 0; i < 3; ++i)
        {
            unassignConsumer(id, AnimationInformation{ 200, 300 });
            assignContentToConsumer(id, categoryInfo, AnimationInformation(), sceneId_t(18));
            requestAndMarkReady(id);
            showContent(id, AnimationInformation{ 200, 300 });
        }
        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }

    TEST_F(ADcsmSystem, allowsToRereadyTheSameContentRepeatedly)
    {
        offerContent(id, Category(111), sceneId_t(18), EDcsmOfferingMode::LocalAndRemote);
        assignContentToConsumer(id, categoryInfo, AnimationInformation(), sceneId_t(18));
        requestAndMarkReady(id);
        showContent(id, AnimationInformation{ 200, 300 });

        for (int i = 0; i < 3; ++i)
        {
            releaseContent(id, AnimationInformation{ 200, 300 });
            requestAndMarkReady(id);
            showContent(id, AnimationInformation{ 200, 300 });
        }
        stopOfferByProvider(id, AnimationInformation{ 200, 300 });
    }

    TEST_F(ADcsmSystem, canDoOfferWithMetadata)
    {
        DcsmMetadataCreator mdf;
        mdf.setPreviewDescription(U"asdf");
        EXPECT_CALL(consHandler, contentOffered(id, Category(123), ETechnicalContentType::RamsesSceneID));
        EXPECT_EQ(provider.offerContentWithMetadata(id, Category(123), sceneId_t(18), EDcsmOfferingMode::LocalAndRemote, mdf), StatusOK);

        EXPECT_CALL(consHandler, contentMetadataUpdated(id, _)).
            WillOnce(Invoke([](auto, auto& prov) { EXPECT_EQ(U"asdf", prov.getPreviewDescription()); }));
        assignContentToConsumer(id, categoryInfo, AnimationInformation(), sceneId_t(18));

        dispatch();
    }

    TEST_F(ADcsmSystem, canUpdateMetadataAfterOffer)
    {
        offerContent(id, Category(111), sceneId_t(18), EDcsmOfferingMode::LocalAndRemote);
        DcsmMetadataCreator mdf;
        mdf.setPreviewDescription(U"00asdf");
        EXPECT_EQ(provider.updateContentMetadata(id, mdf), StatusOK);

        EXPECT_CALL(consHandler, contentMetadataUpdated(id, _)).
            WillOnce(Invoke([](auto, auto& prov) { EXPECT_EQ(U"00asdf", prov.getPreviewDescription()); }));
        assignContentToConsumer(id, categoryInfo, AnimationInformation(), sceneId_t(18));

        dispatch();
    }

    TEST_F(ADcsmSystem, canDoAFullContentLifecycleWithWaylandIviSurfaceIdContent)
    {
        EXPECT_CALL(consHandler, contentOffered(id, Category(123), ETechnicalContentType::WaylandIviSurfaceID));
        EXPECT_EQ(provider.offerContent(id, Category(123), waylandIviSurfaceId_t(5432), EDcsmOfferingMode::LocalAndRemote), StatusOK);
        dispatch();

        EXPECT_CALL(provHandler, contentSizeChange(id, _, _)).WillOnce([&](const auto&, const auto& infoupdate, const auto&) {
            ramses::CategoryInfoUpdate update{categoryInfo.getRenderSize(), categoryInfo.getCategoryRect(), categoryInfo.getSafeRect()};
            EXPECT_EQ(update, infoupdate);
        });
        EXPECT_CALL(consHandler, contentDescription(id, TechnicalContentDescriptor{5432}));
        EXPECT_EQ(consumer.assignContentToConsumer(id, categoryInfo), StatusOK);
        dispatch();

        requestAndMarkReady(id);

        showContent(id, AnimationInformation{200, 300});
        hideContent(id, AnimationInformation{200, 300});
        showContent(id, AnimationInformation{200, 300});

        stopOfferByProvider(id, AnimationInformation{200, 300});
    }
}
