//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "ComponentMocks.h"
#include "Components/ClientSceneLogicShadowCopy.h"
#include "Components/ClientSceneLogicDirect.h"
#include "Components/ISceneGraphSender.h"
#include "Scene/ClientScene.h"
#include "Scene/EScenePublicationMode.h"
#include "Scene/SceneActionApplier.h"
#include "Scene/SceneActionUtils.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

using namespace ramses_internal;

namespace
{
    // Need real matcher class to handle copying of ctor argument by calling explicit copy
    class IsSceneActionCollectionMatcher : public MatcherInterface<const SceneActionCollection&>
    {
    public:
        explicit IsSceneActionCollectionMatcher(const SceneActionCollection& actions)
            : m_actions(actions.copy())
        {
        }

        virtual bool MatchAndExplain(const SceneActionCollection& arg, MatchResultListener* /*listener*/) const override
        {
            if (arg.numberOfActions() > 0 &&
                arg.numberOfActions() == m_actions.numberOfActions() &&
                arg.back().type() == ESceneActionId_Flush &&
                m_actions.back().type() == ESceneActionId_Flush)
            {
                for (UInt32 i = 0; i < arg.numberOfActions() - 1; ++i)
                {
                    if (arg[i].size() != m_actions[i].size() ||
                        arg[i].type() != m_actions[i].type() ||
                        PlatformMemory::Compare(arg[i].data(), m_actions[i].data(), arg[i].size()) != 0)
                    {
                        return false;
                    }
                }
                bool argHasSizeInfo = false;
                bool actionsHasSizeInfo = false;
                UInt64 idxArg = 0u;
                UInt64 idxActions = 0u;
                SceneSizeInformation sizeArg;
                SceneSizeInformation sizeActions;
                SceneResourceChanges resArg;
                SceneResourceChanges resActions;
                SceneReferenceActionVector refsArg;
                SceneReferenceActionVector refsActions;
                SceneVersionTag versionTagArg;
                SceneVersionTag versionTagActions;
                FlushTimeInformation timeInfoArg;
                FlushTimeInformation timeInfoActions;
                SceneActionApplier::ReadParameterForFlushAction(arg.back(), idxArg, argHasSizeInfo, sizeArg, resArg, refsArg, timeInfoArg, versionTagArg);
                SceneActionApplier::ReadParameterForFlushAction(m_actions.back(), idxActions, actionsHasSizeInfo, sizeActions, resActions, refsActions, timeInfoActions, versionTagActions);
                return argHasSizeInfo == actionsHasSizeInfo
                    && timeInfoArg == timeInfoActions
                    && versionTagArg == versionTagActions
                    && sizeArg == sizeActions;
            }
            return arg == m_actions;
        }

        virtual void DescribeTo(::std::ostream* os) const override
        {
            *os << "is expected SceneActionCollection";
        }

        virtual void DescribeNegationTo(::std::ostream* os) const override
        {
            *os << "is not expected SceneActionCollection";
        }

    private:
        SceneActionCollection m_actions;
    };

    inline Matcher<const SceneActionCollection&> IsSceneActionCollection(const SceneActionCollection& actions) {
        return MakeMatcher(new IsSceneActionCollectionMatcher(actions));
    }
}

class SceneGraphSenderMock : public ISceneGraphSender
{
public:
    SceneGraphSenderMock() {}
    virtual ~SceneGraphSenderMock() {}
    MOCK_METHOD(void, sendPublishScene, (SceneId sceneId, EScenePublicationMode publicationMode, const String& name), (override));
    MOCK_METHOD(void, sendUnpublishScene, (SceneId sceneId, EScenePublicationMode publicationMode), (override));
    MOCK_METHOD(void, sendCreateScene, (const Guid& to, const SceneInfo& sceneInfo, EScenePublicationMode publicationMode), (override));
    MOCK_METHOD(void, sendSceneActionList_rvr, (const std::vector<Guid>& to, const SceneActionCollection & sceneAction, SceneId sceneId, EScenePublicationMode publicationMode));

    void sendSceneActionList(const std::vector<Guid>& to, SceneActionCollection&& sceneAction, SceneId sceneId, EScenePublicationMode publicationMode) override
    {
        sendSceneActionList_rvr(to, sceneAction, sceneId, publicationMode);
        sceneAction.clear(); // simulate moved away
    }
};

template <typename T>
class AClientSceneLogic_All : public ::testing::Test
{
public:
    AClientSceneLogic_All()
        : m_myID(765)
        , m_sceneId(33u)
        , m_scene(SceneInfo(m_sceneId))
        , m_sceneLogic(m_sceneGraphProviderComponent, m_scene, m_myID)
        , m_rendererID(1337)
    {
    }

protected:
    void publish()
    {
        EXPECT_CALL(m_sceneGraphProviderComponent, sendPublishScene(m_sceneId, ramses_internal::EScenePublicationMode_LocalAndRemote, m_scene.getName()));
        m_sceneLogic.publish(ramses_internal::EScenePublicationMode_LocalAndRemote);
    }

    void unpublish()
    {
        expectSceneUnpublish();
        m_sceneLogic.unpublish();
    }

    void addSubscriber()
    {
        m_sceneLogic.addSubscriber(m_rendererID);
    }

    void flush(FlushTimeInformation fti = {}, SceneVersionTag versionTag = SceneVersionTag::Invalid())
    {
        m_sceneLogic.flushSceneActions(fti, versionTag);
    }

    void expectSendOnActionList(const SceneActionCollection& actions)
    {
        EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneActionList_rvr(_, IsSceneActionCollection(actions), _, _));
    }

    SceneActionCollection createFlushSceneActionList(bool expectSendSize = true, UInt64 flushIdx = 1)
    {
        SceneActionCollection collection;
        SceneActionCollectionCreator creator(collection);
        creator.flush(flushIdx, expectSendSize);
        return collection;
    }

    void expectFlushSceneActionList(bool expectSendSize = true, UInt64 flushIdx = 1)
    {
        expectSendOnActionList(createFlushSceneActionList(expectSendSize, flushIdx));
    }

    void publishAndAddSubscriberWithoutPendingActions()
    {
        publish();
        expectSceneSend();
        expectFlushSceneActionList();
        addSubscriber();
        flush();
        Mock::VerifyAndClearExpectations(&m_sceneGraphProviderComponent);
    }

    void expectSceneSend()
    {
        const SceneInfo sceneInfo(m_sceneId, m_scene.getName());
        EXPECT_CALL(m_sceneGraphProviderComponent, sendCreateScene(m_rendererID, sceneInfo, _));
    }

    void expectSceneUnpublish()
    {
        EXPECT_CALL(m_sceneGraphProviderComponent, sendUnpublishScene(m_sceneId, _));
    }

    ramses_internal::Guid m_myID;
    ramses_internal::SceneId m_sceneId;
    ramses_internal::ClientScene m_scene;
    StrictMock<SceneGraphSenderMock> m_sceneGraphProviderComponent;
    T m_sceneLogic;
    ramses_internal::Guid m_rendererID;
};

class AClientSceneLogic_ShadowCopy : public AClientSceneLogic_All<ClientSceneLogicShadowCopy>
{
public:
    AClientSceneLogic_ShadowCopy() {}
};

class AClientSceneLogic_Direct : public AClientSceneLogic_All<ClientSceneLogicDirect>
{
public:
    AClientSceneLogic_Direct() {}
};

typedef ::testing::Types<ClientSceneLogicShadowCopy, ClientSceneLogicDirect> ClientSceneLogicTypes;
TYPED_TEST_SUITE(AClientSceneLogic_All, ClientSceneLogicTypes);


TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneRegistrationWhenCreatingScene)
{
    // strict mock ensures that creating causes no calls to SceneGraphProviderComponent
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneRegistrationWhenCreatingSceneRegisteringAppAndRemovingScene)
{
    // strict mock ensures no further calls to SceneGraphProviderComponent
    this->m_sceneLogic.unpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneRegistrationWhenCreatingSceneRegisteringAppAndDisablingSceneDistribution)
{
    // strict mock ensures that creating causes no further calls to SceneGraphProviderComponent
    this->m_scene.allocateTransform(this->m_scene.allocateNode());
    EXPECT_EQ(2u, this->m_scene.getSceneActionCollection().numberOfActions());
    this->m_sceneLogic.unpublish();
}

TYPED_TEST(AClientSceneLogic_All, isNotPublishedInitially)
{
    EXPECT_FALSE(this->m_sceneLogic.isPublished());
}

TYPED_TEST(AClientSceneLogic_All, clearsCollectedActionsOnFlush)
{
    this->m_scene.allocateTransform(this->m_scene.allocateNode());
    EXPECT_EQ(2u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_TRUE(this->m_scene.getSceneActionCollection().empty());
}

TYPED_TEST(AClientSceneLogic_All, keepsPendingActionIfNotFlushed)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->m_scene.allocateNode();
    const SceneActionCollection& actions = this->m_scene.getSceneActionCollection();
    EXPECT_EQ(1u, actions.numberOfActions());
    EXPECT_EQ(ESceneActionId_AllocateNode, actions[0].type());
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, keepsPendingActionVectorIfNotFlushed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateTransform(this->m_scene.allocateNode());

    const SceneActionCollection& actions = this->m_scene.getSceneActionCollection();
    EXPECT_EQ(2u, actions.numberOfActions());
    EXPECT_EQ(ESceneActionId_AllocateNode, actions[0].type());
    EXPECT_EQ(ESceneActionId_AllocateTransform, actions[1].type());

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNothingIfFlushingNonDistributedScene)
{
    this->m_sceneLogic.flushSceneActions({}, {});
    // No call expectation
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneOutIfSceneDistributedWithNoSubscribers)
{
    this->publish();
    // No call expectation
    EXPECT_TRUE(this->m_sceneLogic.isPublished());

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendsPublishOnlyOnce)
{
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendPublishScene(this->m_sceneId, ramses_internal::EScenePublicationMode_LocalAndRemote, this->m_scene.getName()));
    this->m_sceneLogic.publish(ramses_internal::EScenePublicationMode_LocalAndRemote);
    this->m_sceneLogic.publish(ramses_internal::EScenePublicationMode_LocalAndRemote);

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotFlushAnythingForDistributedSceneWithNoSubscribers)
{
    this->publish();
    this->m_sceneLogic.flushSceneActions({}, {});
    // No call expectation

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendsOutSceneToSubscribedRendererWhenDistributionEnabled)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    EXPECT_TRUE(this->m_sceneLogic.isPublished());
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, flushesPendingActionsIfSceneDistributedAndHasPendingActions)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    EXPECT_TRUE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, everyFlushGeneratesSceneActionSentToSubscriber)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->expectFlushSceneActionList(false);
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectFlushSceneActionList(false);
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, unskippableEmptyFlushesGeneratesSceneActionSentToSubscriber)
{
    // has and checks first flush
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    // flush with change
    this->m_sceneLogic.flushSceneActions({}, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

     // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});

    // has expiration
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(_, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({FlushTime::Clock::time_point{std::chrono::milliseconds{1}}, FlushTime::Clock::time_point{}}, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, skippedFlushesAreCounted)
{
    this->publish();
    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->addSubscriber();

    UInt32 initialValue = this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue();
    //first flush
    this->flush();
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);
    EXPECT_EQ(initialValue, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

     // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(initialValue+1, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

    // non empty
    this->m_scene.allocateNode();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(_, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(initialValue+1, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

     // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(initialValue+2, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

    this->expectSceneUnpublish();
}


TEST_F(AClientSceneLogic_ShadowCopy, doesNotClearPendingActionsIfWaitingSubscriberRemoved)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));

    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_sceneLogic.removeSubscriber(newRendererID);
    EXPECT_FALSE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, doesNotClearPendingActionsIfWaitingSubscriberRemoved)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_sceneLogic.removeSubscriber(newRendererID);
    EXPECT_FALSE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, doesNotClearPendingActionsIfSubscriberRemovedIsNotLast)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);

    this->m_scene.allocateNode();

    this->m_sceneLogic.removeSubscriber(newRendererID);

    EXPECT_FALSE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, doesNotClearPendingActionsIfSubscriberRemovedIsNotLast)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_scene.allocateNode();
    this->m_sceneLogic.removeSubscriber(newRendererID);
    EXPECT_FALSE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, willSendSceneAgainToSubscriberThatUnsubscribedAndSubscribedAgain)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_sceneLogic.removeSubscriber(this->m_rendererID);

    expectSceneSend();
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, willSendSceneAgainToSubscriberThatUnsubscribedAndSubscribedAgain)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_sceneLogic.removeSubscriber(this->m_rendererID);

    expectSceneSend();
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    flush();

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNothingIfRemovingSubscriberTwice)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_sceneLogic.removeSubscriber(this->m_rendererID);
    this->m_sceneLogic.removeSubscriber(this->m_rendererID);

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, canDisableDistribution)
{
    this->publish();

    this->expectSceneUnpublish();
    this->m_sceneLogic.unpublish();
    EXPECT_FALSE(this->m_sceneLogic.isPublished());
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneAgainIfEnablingDistributionSecondTime)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->m_sceneLogic.publish(ramses_internal::EScenePublicationMode_LocalAndRemote);
    // No call expectation
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneAgainIfReenablingDistribution)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->unpublish();
    this->publish();
    // No call expectation

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotFlushPendingActionsIfSceneStoppedBeingDistributed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();
    this->unpublish();

    this->m_sceneLogic.flushSceneActions({}, {});
    // No call expectation
}

TYPED_TEST(AClientSceneLogic_All, disablesDistributionTwiceAndDoesNotSendActionsOnSecondDisable)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendUnpublishScene(this->m_sceneId, _));
    this->m_sceneLogic.unpublish();

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);
    this->m_scene.allocateNode();
    this->m_sceneLogic.unpublish();
}

TYPED_TEST(AClientSceneLogic_All, keepsCollectingPendingActionsIfSceneStoppedBeingDistributed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    this->unpublish();

    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());
}

TYPED_TEST(AClientSceneLogic_All, publishesSceneAgainAfterRepublish)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->unpublish();
    this->publish();
    // no other call expected, subscribers were removed with unpublish

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotKeepPendingChangesAfterRepublish)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->unpublish();
    this->publish();

    this->m_scene.allocateNode();

    this->m_sceneLogic.flushSceneActions({}, {});
    // no call expected, subscribers were removed with unpublish

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, resendsSceneAfterRepublishToReaddedSubscriber)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->unpublish();
    this->publish();

    this->expectSceneSend();
    this->expectFlushSceneActionList();
    addSubscriber();

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, resendsSceneAfterRepublishToReaddedSubscriber)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->unpublish();
    this->publish();

    this->expectSceneSend();
    this->expectFlushSceneActionList();
    addSubscriber();
    this->flush();

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, continuesCollectingActionsAfterRepublish)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->unpublish();
    this->publish();
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsOutDistributedSceneToNewlySubscribedRenderer)
{
    this->publish();

    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->flush();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, sendsOutDistributedSceneToNewlySubscribedRenderer)
{
    this->publish();

    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->flush();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, doesNothingIfExistingSubscriberAddedAgain)
{
    this->publish();

    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->flush();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);

    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    // No call expectation

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, doesNotAddAgainIfExistingSubscriberAddedAgain)
{
    this->publish();

    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();
    Mock::VerifyAndClearExpectations(&m_sceneGraphProviderComponent);

    this->expectFlushSceneActionList(false);
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    this->expectSceneUnpublish();
}


TEST_F(AClientSceneLogic_ShadowCopy, canSubscribeToSceneEvenWithPendingActions)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, canSubscribeToSceneEvenWithPendingActions)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle handle = this->m_scene.allocateNode();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());

    SceneActionCollection collection;
    SceneActionCollectionCreator creator(collection);
    creator.allocateNode(0, handle);
    creator.flush(2u, true, this->m_scene.getSceneSizeInformation());

    EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, IsSceneActionCollection(collection), _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));
    EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ newRendererID }, IsSceneActionCollection(collection), _, _));

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->flush();

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, SendCleanSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");

    // expect direct scene send to new renderer
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));
    this->expectFlushSceneActionList();

    this->m_sceneLogic.addSubscriber(newRendererID);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, SendCleanSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");

    // expect direct scene send to new renderer
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));

    this->m_sceneLogic.addSubscriber(newRendererID);
    EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{this->m_rendererID}, IsSceneActionCollection(createFlushSceneActionList(false, 2)), _, _));
    EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{newRendererID}, IsSceneActionCollection(createFlushSceneActionList(true, 2)), _, _));
    this->flush();

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesAppendFlushActionIfOtherSceneActionsAreFlushed)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0u, NodeHandle(1));

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(2u, actionsFromSendScene.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateNode, actionsFromSendScene[0].type());
        EXPECT_EQ(ESceneActionId_Flush, actionsFromSendScene[1].type());
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, appendsDefaultFlushInfoWhenSendingSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publish();

    this->m_scene.allocateNode(0u, NodeHandle(1));

    const FlushTimeInformation ftiIn{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)) };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiIn, versionTagIn);

    this->expectSceneSend();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(2u, actionsFromSendScene.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateNode, actionsFromSendScene[0].type());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene[1].type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferences;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene[1], flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferences, timeInfo, versionTag);
        EXPECT_TRUE(hasSizeInfo);
        EXPECT_EQ(ftiIn, timeInfo);
        EXPECT_EQ(versionTagIn, versionTag);
    });
    this->addSubscriber();
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, appendsDefaultFlushInfoWhenSendingSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publish();

    this->m_scene.allocateNode(0u, NodeHandle(1));

    const FlushTimeInformation ftiInIgnored{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)) };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiInIgnored, versionTagIn);

    this->expectSceneSend();

    const FlushTimeInformation ftiInUsed{ FlushTime::Clock::time_point(std::chrono::milliseconds(5)), FlushTime::Clock::time_point(std::chrono::milliseconds(6)) };
    const SceneVersionTag versionTagUsed{ 666 };
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(2u, actionsFromSendScene.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateNode, actionsFromSendScene[0].type());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene[1].type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene[1], flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
        EXPECT_TRUE(hasSizeInfo);
        EXPECT_EQ(ftiInUsed, timeInfo);
        EXPECT_EQ(versionTagUsed, versionTag);
    });
    this->addSubscriber();
    this->flush(ftiInUsed, versionTagUsed);
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendSceneSizesTogetherWithFlushIfSceneSizeIncreased)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_GE(actionsFromSendScene.numberOfActions(), 1u);
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());
        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
        EXPECT_TRUE(hasSizeInfo);
        EXPECT_EQ(this->m_scene.getSceneSizeInformation(), sizeInfo);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, canUseNullptrWhenReadingFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_GE(actionsFromSendScene.numberOfActions(), 1u);
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());
        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneSizesTogetherWithFlushIfSceneSizeUnchanged)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    // create 2 nodes
    const NodeHandle node1 = this->m_scene.allocateNode();
    const NodeHandle node2 = this->m_scene.allocateNode();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    // set child/parent relation - no size change
    this->m_scene.addChildToNode(node1, node2);
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_GE(actionsFromSendScene.numberOfActions(), 1u);
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());
        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
        EXPECT_FALSE(hasSizeInfo);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendListOfNewResourcesTogetherWithFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateRenderable(this->m_scene.allocateNode());

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    const ResourceContentHash dummyRes(666u, 0);

    const RenderTargetHandle renderTarget = this->m_scene.allocateRenderTarget();
    const RenderBufferHandle renderBuffer = this->m_scene.allocateRenderBuffer({ 1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_R16, ERenderBufferAccessMode_ReadWrite, 0u });
    const StreamTextureHandle streamTex = this->m_scene.allocateStreamTexture(0u, dummyRes);
    const BlitPassHandle blitpassHandle = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(5u, actionsFromSendScene.numberOfActions());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene[4].type());
        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene[4], flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
        EXPECT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(dummyRes, resourceChanges.m_addedClientResourceRefs[0]);
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());

        EXPECT_EQ(4u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(renderTarget, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderTarget, resourceChanges.m_sceneResourceActions[0].action);
        EXPECT_EQ(renderBuffer, resourceChanges.m_sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderBuffer, resourceChanges.m_sceneResourceActions[1].action);
        EXPECT_EQ(streamTex, resourceChanges.m_sceneResourceActions[2].handle);
        EXPECT_EQ(ESceneResourceAction_CreateStreamTexture, resourceChanges.m_sceneResourceActions[2].action);
        EXPECT_EQ(blitpassHandle, resourceChanges.m_sceneResourceActions[3].handle);
        EXPECT_EQ(ESceneResourceAction_CreateBlitPass, resourceChanges.m_sceneResourceActions[3].action);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, flushClearsListsOfChangedResources)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateRenderable(this->m_scene.allocateNode());
    const ResourceContentHash dummyRes(666u, 0);
    this->m_scene.allocateRenderTarget();
    this->m_scene.allocateRenderBuffer({ 1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_R16, ERenderBufferAccessMode_ReadWrite, 0u });
    this->m_scene.allocateStreamTexture(0u, dummyRes);
    this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        bool isDummy = false;
        SceneSizeInformation dummySizes;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, isDummy, dummySizes, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
        EXPECT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(4u, resourceChanges.m_sceneResourceActions.size());
    });
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.allocateNode();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        bool isDummy = false;
        SceneSizeInformation dummySizes;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, isDummy, dummySizes, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    });
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendListOfObsoleteResourcesTogetherWithFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateRenderable(this->m_scene.allocateNode());
    const ResourceContentHash dummyRes(666u, 0);
    this->m_scene.allocateDataLayout({}, ResourceContentHash(45u, 0));
    const RenderTargetHandle renderTarget = this->m_scene.allocateRenderTarget();
    const RenderBufferHandle renderBuffer = this->m_scene.allocateRenderBuffer({ 1u, 1u, ERenderBufferType_ColorBuffer, ETextureFormat_R16, ERenderBufferAccessMode_ReadWrite, 0u });
    const StreamTextureHandle streamTex = this->m_scene.allocateStreamTexture(0u, dummyRes);
    const BlitPassHandle blitpassHandle = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.releaseRenderTarget(renderTarget);
    this->m_scene.releaseRenderBuffer(renderBuffer);
    this->m_scene.releaseStreamTexture(streamTex);
    this->m_scene.releaseBlitPass(blitpassHandle);

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(5u, actionsFromSendScene.numberOfActions());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene[4].type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferencingInfo;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene[4], flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferencingInfo, timeInfo, versionTag);
        EXPECT_EQ(0u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(1u, resourceChanges.m_removedClientResourceRefs.size());
        EXPECT_EQ(dummyRes,  resourceChanges.m_removedClientResourceRefs[0]);

        EXPECT_EQ(4u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(renderTarget, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderTarget, resourceChanges.m_sceneResourceActions[0].action);
        EXPECT_EQ(renderBuffer, resourceChanges.m_sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderBuffer, resourceChanges.m_sceneResourceActions[1].action);
        EXPECT_EQ(streamTex, resourceChanges.m_sceneResourceActions[2].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyStreamTexture, resourceChanges.m_sceneResourceActions[2].action);
        EXPECT_EQ(blitpassHandle, resourceChanges.m_sceneResourceActions[3].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyBlitPass, resourceChanges.m_sceneResourceActions[3].action);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererAfterFlushIfThereArePendingActions)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle node = this->m_scene.allocateNode(0u, NodeHandle(1u));

    // expect flush to original renderer first
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->m_scene.allocateRenderable(node, RenderableHandle(2));

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));
    // expect newly flushed actions to new renderer
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(2u, actionsFromSendScene.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateNode, actionsFromSendScene[0].type());
        EXPECT_EQ(ESceneActionId_Flush, actionsFromSendScene[1].type());
    });
    this->m_sceneLogic.addSubscriber(newRendererID);

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    // expect newly flushed actions to original and new renderer
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(UnorderedElementsAre(this->m_rendererID, newRendererID), _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(2u, actionsFromSendScene.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateRenderable, actionsFromSendScene[0].type());
        EXPECT_EQ(ESceneActionId_Flush, actionsFromSendScene[1].type());
    });
    this->m_sceneLogic.flushSceneActions({}, {});

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererWithNewResources)
{
    // add some active subscriber so actions are queued
    this->publish();
    auto rendHandle = this->m_scene.allocateRenderable(this->m_scene.allocateNode());
    const ResourceContentHash hash(0xff00ff00, 0);
    auto layoutHandle = this->m_scene.allocateDataLayout({}, hash);
    auto instanceHandle = this->m_scene.allocateDataInstance(layoutHandle);
    this->m_scene.setRenderableDataInstance(rendHandle, ERenderableDataSlotType_Geometry, instanceHandle);
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.allocateNode(); // action not flushed

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(5u, actionsFromSendScene.numberOfActions());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferenceActions;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);
        ASSERT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_addedClientResourceRefs[0]);
        EXPECT_TRUE(resourceChanges.m_removedClientResourceRefs.empty());
        EXPECT_TRUE(resourceChanges.m_sceneResourceActions.empty());
    });
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererWithValidTimeInfoAndVersion)
{
    // add some active subscriber so actions are queued
    this->publish();

    const FlushTimeInformation ftiIn{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)) };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiIn, versionTagIn);

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(1u, actionsFromSendScene.numberOfActions());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferenceActions;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);
        EXPECT_EQ(ftiIn, timeInfo);
        EXPECT_EQ(versionTagIn, versionTag);
    });
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererWithLastAppliedTimeInfoAndVersion)
{
    // add some active subscriber so actions are queued
    this->publish();

    const FlushTimeInformation ftiIn{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)) };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiIn, versionTagIn);

    const FlushTimeInformation ftiIn2{ FlushTime::Clock::time_point(std::chrono::milliseconds(22)), FlushTime::Clock::time_point(std::chrono::milliseconds(33)) };
    const SceneVersionTag versionTagIn2{ 123 };
    this->m_sceneLogic.flushSceneActions(ftiIn2, versionTagIn2);

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(1u, actionsFromSendScene.numberOfActions());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferenceActions;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);
        EXPECT_EQ(ftiIn2, timeInfo);
        EXPECT_EQ(versionTagIn2, versionTag);
    });
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, sendsSceneToNewlySubscribedRendererWithNewResources)
{
    // add some active subscriber so actions are queued
    this->publish();
    auto rendHandle = this->m_scene.allocateRenderable(this->m_scene.allocateNode());
    const ResourceContentHash hash(0xff00ff00, 0);
    auto layoutHandle = this->m_scene.allocateDataLayout({}, hash);
    auto instanceHandle = this->m_scene.allocateDataInstance(layoutHandle);
    this->m_scene.setRenderableDataInstance(rendHandle, ERenderableDataSlotType_Geometry, instanceHandle);
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.allocateNode(); // action not flushed

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    this->m_sceneLogic.addSubscriber(newRendererID);

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(6u, actionsFromSendScene.numberOfActions());
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferenceActions;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);
        ASSERT_EQ(1u, resourceChanges.m_addedClientResourceRefs.size());
        EXPECT_EQ(hash, resourceChanges.m_addedClientResourceRefs[0]);
        EXPECT_TRUE(resourceChanges.m_removedClientResourceRefs.empty());
        EXPECT_TRUE(resourceChanges.m_sceneResourceActions.empty());
    });
    this->flush();
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, doesNotSendSceneUpdatesToNewSubscriberThatUnsubscribedBeforeFlush)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, sceneInfo, _));
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);

    this->m_scene.allocateNode();

    this->m_sceneLogic.removeSubscriber(newRendererID);

    // expect flush to original renderer first
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, doesNotSendAnythingToNewSubscriberThatUnsubscribedBeforeFlush)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_scene.allocateNode();
    this->m_sceneLogic.removeSubscriber(newRendererID);

    // expect flush to original renderer first
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sceneReferenceActionsAreNotSentToNewlySubscribedRenderer)
{
    this->m_scene.linkData(SceneReferenceHandle{ 1 }, DataSlotId{ 2 }, SceneReferenceHandle{ 3 }, DataSlotId{ 4 });

    this->publish();
    const ramses_internal::Guid newRendererID("12345678-1234-5678-0000-123456789012");
    this->m_sceneLogic.addSubscriber(newRendererID);

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
        {
            ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

            bool hasSizeInfo;
            SceneResourceChanges resourceChanges;
            SceneReferenceActionVector sceneReferenceActions;
            SceneSizeInformation sizeInfo;
            FlushTimeInformation timeInfo;
            SceneVersionTag versionTag;
            UInt64 flushIndex = 0u;
            SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);

            EXPECT_TRUE(sceneReferenceActions.empty());
    });
    this->flush();
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, flushSendsOnlySceneReferenceActionsSinceLastFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.linkData(SceneReferenceHandle{ 1 }, DataSlotId{ 2 }, SceneReferenceHandle{ 3 }, DataSlotId{ 4 });
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(_, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
        {
            ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

            bool hasSizeInfo;
            SceneResourceChanges resourceChanges;
            SceneReferenceActionVector sceneReferenceActions;
            SceneSizeInformation sizeInfo;
            FlushTimeInformation timeInfo;
            SceneVersionTag versionTag;
            UInt64 flushIndex = 0u;
            SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);

            ASSERT_EQ(1u, sceneReferenceActions.size());
            EXPECT_EQ(SceneReferenceActionType::LinkData, sceneReferenceActions[0].type);
            EXPECT_EQ(SceneReferenceHandle{ 1 }, sceneReferenceActions[0].providerScene);
            EXPECT_EQ(DataSlotId{ 2 }, sceneReferenceActions[0].providerId);
            EXPECT_EQ(SceneReferenceHandle{ 3 }, sceneReferenceActions[0].consumerScene);
            EXPECT_EQ(DataSlotId{ 4 }, sceneReferenceActions[0].consumerId);
    });
    this->flush();

    this->m_scene.unlinkData(SceneReferenceHandle{ 1 }, DataSlotId{ 2 });
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(_, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
        {
            ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

            bool hasSizeInfo;
            SceneResourceChanges resourceChanges;
            SceneReferenceActionVector sceneReferenceActions;
            SceneSizeInformation sizeInfo;
            FlushTimeInformation timeInfo;
            SceneVersionTag versionTag;
            UInt64 flushIndex = 0u;
            SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);

            ASSERT_EQ(1u, sceneReferenceActions.size());
            EXPECT_EQ(SceneReferenceActionType::UnlinkData, sceneReferenceActions[0].type);
            EXPECT_EQ(SceneReferenceHandle{ 1 }, sceneReferenceActions[0].consumerScene);
            EXPECT_EQ(DataSlotId{ 2 }, sceneReferenceActions[0].consumerId);
    });
    this->flush();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(_, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        ASSERT_EQ(ESceneActionId_Flush, actionsFromSendScene.back().type());

        bool hasSizeInfo;
        SceneResourceChanges resourceChanges;
        SceneReferenceActionVector sceneReferenceActions;
        SceneSizeInformation sizeInfo;
        FlushTimeInformation timeInfo;
        SceneVersionTag versionTag;
        UInt64 flushIndex = 0u;
        SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, sceneReferenceActions, timeInfo, versionTag);

        EXPECT_TRUE(sceneReferenceActions.empty());
    });
    this->flush({}, SceneVersionTag{1});  // force flush by version
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, unpublishesSceneAndDoesNotFlushBeforeRemovingScene)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    this->expectSceneUnpublish();
    this->m_sceneLogic.unpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendAnythingToSubscribersWithoutFlush)
{
    this->publish();
    this->addSubscriber();
    this->unpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesSendSceneCreationToSubscriberAfterFlush)
{
    this->publish();
    this->addSubscriber();
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->flush();
    this->unpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesSendSceneToSubscriberAfterFlush)
{
    this->publish();
    this->addSubscriber();

    this->m_scene.allocateNode(0,NodeHandle(0u));
    this->m_scene.allocateTransform(NodeHandle(0u), TransformHandle(0u));

    // copy the current action collection to use it later
    // for checking
    SceneActionCollection expectedActions(this->m_scene.getSceneActionCollection().copy());
    SceneActionCollectionCreator creator(expectedActions);
    creator.flush(1u, true, this->m_scene.getSceneSizeInformation());

    this->expectSceneSend();
    this->expectSendOnActionList(expectedActions);
    this->flush();

    this->unpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, doesSendSceneAfterFlushToLateSubscribers)
{
    this->publish();

    this->m_scene.allocateNode(0, NodeHandle(0u));
    this->m_scene.allocateTransform(NodeHandle(0u), TransformHandle(0u));

    // copy the current action collection to use it later
    // for checking
    SceneActionCollection expectedActions(this->m_scene.getSceneActionCollection().copy());
    SceneActionCollectionCreator creator(expectedActions);
    creator.flush(1u, true, this->m_scene.getSceneSizeInformation());

    this->flush();

    // so far no expectations except scene publishing
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->expectSceneSend();
    this->expectSendOnActionList(expectedActions);

    this->addSubscriber();
    this->unpublish();
}

TEST_F(AClientSceneLogic_Direct, doesSendSceneAfterFlushToLateSubscribers)
{
    this->publish();

    this->m_scene.allocateNode(0, NodeHandle(0u));
    this->m_scene.allocateTransform(NodeHandle(0u), TransformHandle(0u));

    // copy the current action collection to use it later
    // for checking
    SceneActionCollection expectedActions(this->m_scene.getSceneActionCollection().copy());
    SceneActionCollectionCreator creator(expectedActions);
    creator.flush(1u, true, this->m_scene.getSceneSizeInformation());

    this->flush();

    // so far no expectations except scene publishing
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->addSubscriber();
    this->expectSceneSend();
    this->expectSendOnActionList(expectedActions);

    this->flush();

    this->unpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sceneActionsAreNotModifiedWhenLastRendererUnsubscribed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle node = this->m_scene.allocateNode();
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.removeSubscriber(this->m_rendererID);
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(0u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_scene.releaseNode(node);
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(0u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->expectSceneSend();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(_, _, _, _));
    this->addSubscriber();
    EXPECT_EQ(0u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, sceneActionsAreNotModifiedWhenLastRendererUnsubscribed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle node = this->m_scene.allocateNode();
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.removeSubscriber(this->m_rendererID);
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(0u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_scene.releaseNode(node);
    EXPECT_EQ(1u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(0u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->addSubscriber();
    EXPECT_EQ(0u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, increasesStatisticCounterForSceneActionsSentWithFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode();

    UInt32 initialValue = this->m_scene.getStatisticCollection().statSceneActionsSent.getCounterValue();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        EXPECT_EQ(initialValue + actionsFromSendScene.numberOfActions(), this->m_scene.getStatisticCollection().statSceneActionsSent.getCounterValue());
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, increasesStatisticCounterForSceneActionsSentWithSendShadowCopySceneToWaitingSubscribers)
{
    this->publish();
    this->expectSceneSend();

    EXPECT_EQ(0u, this->m_scene.getStatisticCollection().statSceneActionsSent.getCounterValue());

    this->m_sceneLogic.flushSceneActions({}, {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
    {
        EXPECT_EQ(actionsFromSendScene.numberOfActions(), this->m_scene.getStatisticCollection().statSceneActionsSent.getCounterValue());
    });
    this->addSubscriber();
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, getWaitingAndActiveSubscribersDoesWhatItSays)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    EXPECT_THAT(this->m_sceneLogic.getWaitingAndActiveSubscribers(), UnorderedElementsAre(this->m_rendererID));

    const Guid newRendererID(9000);
    this->m_sceneLogic.addSubscriber(newRendererID);
    EXPECT_THAT(this->m_sceneLogic.getWaitingAndActiveSubscribers(), UnorderedElementsAre(this->m_rendererID, newRendererID));

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, removeSubscriberRemoveOnlyGivenSubscriber)
{
    const Guid newRendererID(9000);
    this->publishAndAddSubscriberWithoutPendingActions();
    this->m_sceneLogic.addSubscriber(newRendererID);
    EXPECT_THAT(this->m_sceneLogic.getWaitingAndActiveSubscribers(), UnorderedElementsAre(this->m_rendererID, newRendererID));

    this->m_sceneLogic.removeSubscriber(this->m_rendererID);
    EXPECT_THAT(this->m_sceneLogic.getWaitingAndActiveSubscribers(), UnorderedElementsAre(newRendererID));

    this->m_sceneLogic.removeSubscriber(newRendererID);
    EXPECT_THAT(this->m_sceneLogic.getWaitingAndActiveSubscribers(), UnorderedElementsAre());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, scansForClientResourcesWhenFlushingUnpublishedScene)
{
    this->m_scene.allocateNode();
    this->m_scene.allocateStreamTexture(0, ResourceContentHash{ 0, 1 });
    this->flush();
    this->m_scene.allocateNode();
    this->m_scene.allocateStreamTexture(0, ResourceContentHash{ 0, 2 });
    this->flush();
    this->publish();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneActionList_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _)).WillOnce([&](const auto&, const auto& actionsFromSendScene, auto, auto)
        {
            bool hasSizeInfo;
            SceneResourceChanges resourceChanges;
            SceneReferenceActionVector referenceInfo;
            SceneSizeInformation sizeInfo;
            FlushTimeInformation timeInfo;
            SceneVersionTag versionTag;
            UInt64 flushIndex = 0u;
            SceneActionApplier::ReadParameterForFlushAction(actionsFromSendScene.back(), flushIndex, hasSizeInfo, sizeInfo, resourceChanges, referenceInfo, timeInfo, versionTag);
            EXPECT_EQ(2u, resourceChanges.m_addedClientResourceRefs.size());
        });

    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(m_rendererID);
    this->expectSceneUnpublish();
}

