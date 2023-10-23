//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ComponentMocks.h"
#include "internal/Components/ClientSceneLogicShadowCopy.h"
#include "internal/Components/ClientSceneLogicDirect.h"
#include "internal/Components/ISceneGraphSender.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/Components/SceneUpdate.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"

namespace ramses::internal
{
    // Need real matcher class to handle copying of ctor argument by calling explicit copy
    class ContainsSceneUpdateMatcher : public MatcherInterface<const SceneUpdate&>
    {
    public:
        explicit ContainsSceneUpdateMatcher(const SceneUpdate& update)
        {
            m_sceneUpdate.actions = update.actions.copy();
            m_sceneUpdate.flushInfos = update.flushInfos.copy();
        }

        bool MatchAndExplain(const SceneUpdate& updatearg, MatchResultListener* /*listener*/) const override
        {
            const SceneActionCollection& arg = updatearg.actions;
            if (arg.numberOfActions() > 0 &&
                arg.numberOfActions() == m_sceneUpdate.actions.numberOfActions() )
            {
                for (uint32_t i = 0; i < arg.numberOfActions() - 1; ++i)
                {
                    if (arg[i].size() != m_sceneUpdate.actions[i].size() ||
                        arg[i].type() != m_sceneUpdate.actions[i].type() ||
                        PlatformMemory::Compare(arg[i].data(), m_sceneUpdate.actions[i].data(), arg[i].size()) != 0)
                    {
                        return false;
                    }
                }
                return m_sceneUpdate.flushInfos.hasSizeInfo == updatearg.flushInfos.hasSizeInfo
                    && m_sceneUpdate.flushInfos.flushTimeInfo == updatearg.flushInfos.flushTimeInfo
                    && m_sceneUpdate.flushInfos.versionTag == updatearg.flushInfos.versionTag
                    && m_sceneUpdate.flushInfos.sizeInfo == updatearg.flushInfos.sizeInfo;
            }
            return updatearg.actions == m_sceneUpdate.actions;
        }

        void DescribeTo(::std::ostream* os) const override
        {
            *os << "is expected SceneActionCollection";
        }

        void DescribeNegationTo(::std::ostream* os) const override
        {
            *os << "is not expected SceneActionCollection";
        }

    private:
        SceneUpdate m_sceneUpdate;
    };

    inline Matcher<const SceneUpdate&> ContainsSceneUpdate(const SceneUpdate& sceneupdate) {
        return MakeMatcher(new ContainsSceneUpdateMatcher(sceneupdate));
    }

class SceneGraphSenderMock : public ISceneGraphSender
{
public:
    SceneGraphSenderMock() = default;
    ~SceneGraphSenderMock() override = default;
    MOCK_METHOD(void, sendPublishScene, (SceneId sceneId, EScenePublicationMode publicationMode, std::string_view name), (override));
    MOCK_METHOD(void, sendUnpublishScene, (SceneId sceneId, EScenePublicationMode publicationMode), (override));
    MOCK_METHOD(void, sendCreateScene, (const Guid& to, const SceneId& sceneId, EScenePublicationMode publicationMode), (override));
    MOCK_METHOD(void, sendSceneUpdate_rvr, (const std::vector<Guid>& to, const SceneUpdate & sceneAction, SceneId sceneId, EScenePublicationMode publicationMode, StatisticCollectionScene& sceneStatistics));

    void sendSceneUpdate(const std::vector<Guid>& to, SceneUpdate&& update, SceneId sceneId, EScenePublicationMode publicationMode, StatisticCollectionScene& sceneStatistics) override
    {
        sendSceneUpdate_rvr(to, update, sceneId, publicationMode, sceneStatistics);
        update.actions.clear(); // simulate moved away
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
        , m_sceneLogic(m_sceneGraphProviderComponent, m_scene, m_resourceComponent, m_myID)
        , m_rendererID(1337)
        , m_arrayResourceRaw(new ArrayResource(EResourceType::IndexArray, 1, EDataType::UInt16, nullptr, {}))
        , m_effectResourceRaw(new EffectResource("foo", {}, {}, {}, {}, {}, {}))
        , m_textureResourceRaw(new TextureResource(EResourceType::Texture2D, TextureMetaInfo(1u, 1u, 1u, EPixelStorageFormat::R8, false, {}, { 1u }), {}))
        , m_arrayResource(m_arrayResourceRaw)
        , m_effectResource(m_effectResourceRaw)
        , m_textureResource(m_textureResourceRaw)
        , m_resInfo(1)
    {
        EXPECT_CALL(this->m_resourceComponent, resolveResources(_)).Times(AnyNumber()).WillRepeatedly(Return(ManagedResourceVector{}));
        EXPECT_CALL(this->m_resourceComponent, knowsResource(_)).Times(AnyNumber()).WillRepeatedly(Return(true));
        EXPECT_CALL(this->m_resourceComponent, getResourceInfo(_)).Times(AnyNumber()).WillRepeatedly(ReturnRef(this->m_resInfo[0]));
        this->m_arrayResourceRaw->setResourceData(ResourceBlob{ 1 }, { 1u, 1u });
        this->m_textureResourceRaw->setResourceData(ResourceBlob{ 1 }, { 2u, 2u });
    }

protected:
    void publish()
    {
        std::string_view name{m_scene.getName()};
        EXPECT_CALL(m_sceneGraphProviderComponent, sendPublishScene(m_sceneId, EScenePublicationMode::LocalAndRemote, name));
        m_sceneLogic.publish(EScenePublicationMode::LocalAndRemote);
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

    void expectSendOnActionList(const SceneUpdate& actions)
    {
        EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, ContainsSceneUpdate(actions), _, _, _));
    }

    SceneUpdate createFlushSceneActionList(bool expectSendSize = true, uint64_t flushIdx = 1)
    {
        SceneUpdate update;
        SceneActionCollectionCreator creator(update.actions);
        update.flushInfos.hasSizeInfo = expectSendSize;
        update.flushInfos.flushCounter = flushIdx;
        return update;
    }

    void expectFlushSceneActionList(bool expectSendSize = true, uint64_t flushIdx = 1)
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
        EXPECT_CALL(m_sceneGraphProviderComponent, sendCreateScene(m_rendererID, m_sceneId, _));
    }

    void expectSceneUnpublish()
    {
        EXPECT_CALL(m_sceneGraphProviderComponent, sendUnpublishScene(m_sceneId, _));
    }

    void expectResourceQueries(ManagedResourceVector const& newResources = {}, ManagedResourceVector allResources = {}, bool otherResChanges = false, bool expectResolveAllResourcesForDirect = false, bool expectResolveAllResourcesForShadowCopy = true)
    {
        Mock::VerifyAndClearExpectations(&this->m_resourceComponent);

        // allResources are newResources by default
        if (allResources.empty())
            allResources = newResources;

        ResourceContentHashVector allHashes;
        std::transform(allResources.begin(), allResources.end(), std::back_inserter(allHashes), [](auto const& res) { return res->getHash(); });
        std::sort(allHashes.begin(), allHashes.end());

        InSequence inseq;

        if (!newResources.empty() || otherResChanges)
        {
            ResourceContentHashVector newHashes;
            std::transform(newResources.begin(), newResources.end(), std::back_inserter(newHashes), [](auto const& res) { return res->getHash(); });
            std::sort(newHashes.begin(), newHashes.end());

            if (!newResources.empty())
                EXPECT_CALL(this->m_resourceComponent, resolveResources(newHashes)).WillOnce(Return(newResources));

            for (auto const& hash : allHashes)
            {
                EXPECT_CALL(this->m_resourceComponent, knowsResource(hash)).WillOnce(Return(true));
                EXPECT_CALL(this->m_resourceComponent, getResourceInfo(hash)).WillOnce([allResources, this](auto const& hash_) -> ResourceInfo const&
                    {
                        auto it = std::find_if(allResources.begin(), allResources.end(), [&hash_](auto const& res) { return res->getHash() == hash_; });
                        EXPECT_NE(it, allResources.end());
                        this->m_resInfo.push_back(ResourceInfo(it->get())); // keep resinfo alive to be able to return a ref to it
                        return this->m_resInfo[this->m_resInfo.size() - 1];
                    });
            }
        }

        // shadow scene logic will do an additional resolve for all resources when resources changed to keep them alive
        // direct scene logic will do an additional resolve if there was a subscriber since last flush
        if ((expectResolveAllResourcesForShadowCopy && std::is_same<T, ClientSceneLogicShadowCopy>()) || (expectResolveAllResourcesForDirect && std::is_same<T, ClientSceneLogicDirect>()))
            EXPECT_CALL(this->m_resourceComponent, resolveResources(allHashes)).WillOnce(Return(allResources));
    }

    void expectStatistics(EResourceStatisticIndex index, std::vector<uint64_t> cmp)
    {
        auto const& stats = this->m_scene.getStatisticCollection();
        EXPECT_EQ(stats.statResourceCount[index].getCounterValue(), cmp[0]);
        EXPECT_EQ(stats.statResourceAvgSize[index].getCounterValue(), cmp[1]);
        EXPECT_EQ(stats.statResourceMaxSize[index].getCounterValue(), cmp[2]);
    }

    Guid m_myID;
    SceneId m_sceneId;
    ClientScene m_scene;
    StrictMock<ResourceProviderComponentMock> m_resourceComponent;
    StrictMock<SceneGraphSenderMock> m_sceneGraphProviderComponent;
    T m_sceneLogic;
    Guid m_rendererID;

    ArrayResource* m_arrayResourceRaw;
    EffectResource* m_effectResourceRaw;
    TextureResource* m_textureResourceRaw;
    ManagedResource m_arrayResource;
    ManagedResource m_effectResource;
    ManagedResource m_textureResource;
    std::vector<ResourceInfo> m_resInfo;
};

class AClientSceneLogic_ShadowCopy : public AClientSceneLogic_All<ClientSceneLogicShadowCopy>
{
public:
    AClientSceneLogic_ShadowCopy() = default;
};

class AClientSceneLogic_Direct : public AClientSceneLogic_All<ClientSceneLogicDirect>
{
public:
    AClientSceneLogic_Direct() = default;
};

using ClientSceneLogicTypes = ::testing::Types<ClientSceneLogicShadowCopy, ClientSceneLogicDirect>;
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
    this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});
    EXPECT_EQ(2u, this->m_scene.getSceneActionCollection().numberOfActions());
    this->m_sceneLogic.unpublish();
}

TYPED_TEST(AClientSceneLogic_All, isNotPublishedInitially)
{
    EXPECT_FALSE(this->m_sceneLogic.isPublished());
}

TYPED_TEST(AClientSceneLogic_All, clearsCollectedActionsOnFlush)
{
    this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});
    EXPECT_EQ(2u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_TRUE(this->m_scene.getSceneActionCollection().empty());
}

TYPED_TEST(AClientSceneLogic_All, keepsPendingActionIfNotFlushed)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->m_scene.allocateNode(0, {});

    const SceneActionCollection& actions = this->m_scene.getSceneActionCollection();
    EXPECT_EQ(1u, actions.numberOfActions());
    EXPECT_EQ(ESceneActionId::AllocateNode, actions[0].type());
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, keepsPendingActionVectorIfNotFlushed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateTransform(this->m_scene.allocateNode(0, {}), {});

    const SceneActionCollection& actions = this->m_scene.getSceneActionCollection();
    EXPECT_EQ(2u, actions.numberOfActions());
    EXPECT_EQ(ESceneActionId::AllocateNode, actions[0].type());
    EXPECT_EQ(ESceneActionId::AllocateTransform, actions[1].type());

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNothingIfFlushingNonDistributedScene)
{
    this->m_sceneLogic.flushSceneActions({}, {});
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
    std::string_view name{this->m_scene.getName()};
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendPublishScene(this->m_sceneId, EScenePublicationMode::LocalAndRemote, name));
    this->m_sceneLogic.publish(EScenePublicationMode::LocalAndRemote);
    this->m_sceneLogic.publish(EScenePublicationMode::LocalAndRemote);

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotFlushAnythingForDistributedSceneWithNoSubscribers)
{
    this->publish();
    this->m_sceneLogic.flushSceneActions({}, {});

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

    this->m_scene.allocateNode(0, {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    EXPECT_TRUE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, unskippableEmptyFlushesGeneratesSceneActionSentToSubscriber)
{
    // has and checks first flush
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    // flush with change
    this->m_sceneLogic.flushSceneActions({}, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});

    // has expiration
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({ FlushTime::Clock::time_point{std::chrono::milliseconds{1}}, FlushTime::Clock::time_point{}, {}, false }, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, fluhsNotSkippedIfEmptyWithExpirationEnabledOrDisabled)
{
    // has and checks first flush
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    FlushTimeInformation flushTimeInfo = { FlushTime::InvalidTimestamp, {}, {} , false };

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    // flush with change
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    // enable expiration
    flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point{ std::chrono::milliseconds{1} };
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    // disable expiration
    flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point{ std::chrono::milliseconds{0} };
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, flushNotSkippedIfEmptyWithEffectTimeSyncEnabled)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    FlushTimeInformation flushTimeInfo;
    flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point{ std::chrono::milliseconds(1000) };

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    // send effect time sync
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    flushTimeInfo.isEffectTimeSync = true;
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, skippedFlushesAreCounted)
{
    this->publish();
    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->addSubscriber();

    uint32_t initialValue = this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue();
    //first flush
    this->flush();
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);
    EXPECT_EQ(initialValue, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(initialValue + 1, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

    // non empty
    this->m_scene.allocateNode(0, {});
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(initialValue + 1, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(initialValue + 2, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, unskippableEmptyFlushesGeneratesSceneActionSentToSubscriber)
{
    // has and checks first flush
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    // flush with change
    this->m_sceneLogic.flushSceneActions({}, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

     // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});

    // has expiration
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({FlushTime::Clock::time_point{std::chrono::milliseconds{1}}, FlushTime::Clock::time_point{}, {}, false}, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, fluhsNotSkippedIfEmptyWithExpirationEnabledOrDisabled)
{
    // has and checks first flush
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    FlushTimeInformation flushTimeInfo = { FlushTime::InvalidTimestamp, {}, {} , false};

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    // flush with change
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    // enable expiration
    flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point{ std::chrono::milliseconds{1} };
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    // disable expiration
    flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point{ std::chrono::milliseconds{0} };
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, flushNotSkippedIfEmptyWithEffectTimeSyncEnabled)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    FlushTimeInformation flushTimeInfo;
    flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point{std::chrono::milliseconds(1000)};

    // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    // send effect time sync
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
    flushTimeInfo.isEffectTimeSync = true;
    this->m_sceneLogic.flushSceneActions(flushTimeInfo, {});
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, skippedFlushesAreCounted)
{
    this->publish();
    this->expectSceneSend();
    this->expectFlushSceneActionList();
    this->addSubscriber();

    uint32_t initialValue = this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue();
    //first flush
    this->flush();
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);
    EXPECT_EQ(initialValue, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

     // empty -> nothing sent
    this->m_sceneLogic.flushSceneActions({}, {});
    EXPECT_EQ(initialValue+1, this->m_scene.getStatisticCollection().statSceneActionsSentSkipped.getCounterValue());

    // non empty
    this->m_scene.allocateNode(0, {});
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _));
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

    this->m_scene.allocateNode(0, {});

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, m_sceneId, _));

    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_sceneLogic.removeSubscriber(newRendererID);
    EXPECT_FALSE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, doesNotClearPendingActionsIfWaitingSubscriberRemoved)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_sceneLogic.removeSubscriber(newRendererID);
    EXPECT_FALSE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, doesNotClearPendingActionsIfSubscriberRemovedIsNotLast)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, m_sceneId, _));
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);

    this->m_scene.allocateNode(0, {});

    this->m_sceneLogic.removeSubscriber(newRendererID);

    EXPECT_FALSE(this->m_scene.getSceneActionCollection().empty());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, doesNotClearPendingActionsIfSubscriberRemovedIsNotLast)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_scene.allocateNode(0, {});
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
    this->m_sceneLogic.publish(EScenePublicationMode::LocalAndRemote);
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

    this->m_scene.allocateNode(0, {});
    this->unpublish();

    this->m_sceneLogic.flushSceneActions({}, {});
}

TYPED_TEST(AClientSceneLogic_All, disablesDistributionTwiceAndDoesNotSendActionsOnSecondDisable)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendUnpublishScene(this->m_sceneId, _));
    this->m_sceneLogic.unpublish();

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);
    this->m_scene.allocateNode(0, {});
    this->m_sceneLogic.unpublish();
}

TYPED_TEST(AClientSceneLogic_All, keepsCollectingPendingActionsIfSceneStoppedBeingDistributed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

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

    this->m_scene.allocateNode(0, {});

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

    this->m_scene.allocateNode(0, {});
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

    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    this->expectSceneUnpublish();
}


TEST_F(AClientSceneLogic_ShadowCopy, canSubscribeToSceneEvenWithPendingActions)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, this->m_sceneId, _));
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, canSubscribeToSceneEvenWithPendingActions)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle handle = this->m_scene.allocateNode(0, {});

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");

    SceneUpdate update;
    SceneActionCollectionCreator creator(update.actions);
    creator.allocateNode(0, handle);
    update.flushInfos.hasSizeInfo = true;
    update.flushInfos.flushCounter = 2;
    update.flushInfos.sizeInfo = this->m_scene.getSceneSizeInformation();


    EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, ContainsSceneUpdate(update), _, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, this->m_sceneId, _));
    EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ newRendererID }, ContainsSceneUpdate(update), _, _, _));

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->flush();

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, SendCleanSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");

    // expect direct scene send to new renderer
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, this->m_sceneId, _));
    this->expectFlushSceneActionList();

    this->m_sceneLogic.addSubscriber(newRendererID);

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, SendCleanSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");

    // expect direct scene send to new renderer
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, this->m_sceneId, _));

    this->m_sceneLogic.addSubscriber(newRendererID);
    EXPECT_CALL(m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{newRendererID}, ContainsSceneUpdate(createFlushSceneActionList(true, 2)), _, _, _));
    this->flush();

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesAppendFlushActionIfOtherSceneActionsAreFlushed)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0u, NodeHandle(1));

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(1u, update.actions.numberOfActions());
        EXPECT_EQ(ESceneActionId::AllocateNode, update.actions[0].type());
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, appendsDefaultFlushInfoWhenSendingSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publish();

    this->m_scene.allocateNode(0u, NodeHandle(1));

    const FlushTimeInformation ftiIn{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)), FlushTime::Clock::getClockType(), false };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiIn, versionTagIn);

    this->expectSceneSend();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& updateFromSendScene, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(1u, updateFromSendScene.actions.numberOfActions());
        EXPECT_EQ(ESceneActionId::AllocateNode, updateFromSendScene.actions[0].type());

        EXPECT_TRUE(updateFromSendScene.flushInfos.hasSizeInfo);
        EXPECT_EQ(ftiIn, updateFromSendScene.flushInfos.flushTimeInfo);
        EXPECT_EQ(versionTagIn, updateFromSendScene.flushInfos.versionTag);
    });
    this->addSubscriber();
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, appendsTimeSyncInfoWhenSendingSceneToNewSubscriber)
{
    this->publish();

    const auto syncTime = FlushTime::Clock::time_point(std::chrono::milliseconds(4));
    const auto flushTime = FlushTime::Clock::time_point(std::chrono::milliseconds(5));
    const auto flushTime2 = FlushTime::Clock::time_point(std::chrono::milliseconds(6));
    const auto expirationTime = FlushTime::Clock::time_point(std::chrono::milliseconds(11));
    const auto clockType      = FlushTime::Clock::getClockType();

    this->flush({FlushTime::InvalidTimestamp, syncTime, clockType, true}, {});
    this->m_scene.allocateNode(0u, NodeHandle(1));
    this->flush({expirationTime, flushTime, clockType, false}, {});

    this->expectSceneSend();
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& updateFromSendScene, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(1u, updateFromSendScene.actions.numberOfActions());
        EXPECT_EQ(ESceneActionId::AllocateNode, updateFromSendScene.actions[0].type());

        EXPECT_TRUE(updateFromSendScene.flushInfos.hasSizeInfo);
        const FlushTimeInformation& flushTimeInfo = updateFromSendScene.flushInfos.flushTimeInfo;
        // flushTime is not transmitted
        EXPECT_EQ(syncTime, flushTimeInfo.internalTimestamp);
        EXPECT_EQ(expirationTime, flushTimeInfo.expirationTimestamp);
        EXPECT_EQ(clockType, flushTimeInfo.clock_type);
        EXPECT_TRUE(flushTimeInfo.isEffectTimeSync);
    });
    this->addSubscriber();
    if (std::is_same<TypeParam, ClientSceneLogicDirect>::value)
    {
        // ClientSceneLogicShadowCopy sends scene when subscriber is added (no flush needed)
        this->flush({expirationTime, flushTime2, clockType, false});
    }
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, appendsDefaultFlushInfoWhenSendingSceneToNewSubscriber)
{
    // add some active subscriber so actions are queued
    this->publish();

    this->m_scene.allocateNode(0u, NodeHandle(1));

    const FlushTimeInformation ftiInIgnored{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)), FlushTime::Clock::getClockType(), false };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiInIgnored, versionTagIn);

    this->expectSceneSend();

    const FlushTimeInformation ftiInUsed{ FlushTime::Clock::time_point(std::chrono::milliseconds(5)), FlushTime::Clock::time_point(std::chrono::milliseconds(6)), FlushTime::Clock::getClockType(), false };
    const SceneVersionTag versionTagUsed{ 666 };
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& updateFromSendScene, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(1u, updateFromSendScene.actions.numberOfActions());
        EXPECT_EQ(ESceneActionId::AllocateNode, updateFromSendScene.actions[0].type());

        EXPECT_TRUE(updateFromSendScene.flushInfos.hasSizeInfo);
        EXPECT_EQ(ftiInUsed, updateFromSendScene.flushInfos.flushTimeInfo);
        EXPECT_EQ(versionTagUsed, updateFromSendScene.flushInfos.versionTag);
    });
    this->addSubscriber();
    this->flush(ftiInUsed, versionTagUsed);
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendSceneSizesTogetherWithFlushIfSceneSizeIncreased)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_GE(update.actions.numberOfActions(), 0u);

        EXPECT_TRUE(update.flushInfos.hasSizeInfo);
        EXPECT_EQ(this->m_scene.getSceneSizeInformation(), update.flushInfos.sizeInfo);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, canUseNullptrWhenReadingFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_GE(update.actions.numberOfActions(), 0u);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotSendSceneSizesTogetherWithFlushIfSceneSizeUnchanged)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    // create 2 nodes
    const NodeHandle node1 = this->m_scene.allocateNode(0, {});
    const NodeHandle node2 = this->m_scene.allocateNode(0, {});
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    // set child/parent relation - no size change
    this->m_scene.addChildToNode(node1, node2);
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        EXPECT_FALSE(update.flushInfos.hasSizeInfo);
    });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendListOfNewResourcesTogetherWithFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateRenderable(this->m_scene.allocateNode(0, {}), {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    const ResourceContentHash hash = this->m_arrayResource->getHash();

    const RenderTargetHandle renderTarget = this->m_scene.allocateRenderTarget({});
    const RenderBufferHandle renderBuffer = this->m_scene.allocateRenderBuffer({ 1u, 1u, EPixelStorageFormat::R16F, ERenderBufferAccessMode::ReadWrite, 0u }, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, hash, {} }, {});
    const BlitPassHandle blitpassHandle = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u), {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(4u, update.actions.numberOfActions());
        auto& resourceChanges = update.flushInfos.resourceChanges;
        EXPECT_EQ(1u, resourceChanges.m_resourcesAdded.size());
        EXPECT_EQ(hash, resourceChanges.m_resourcesAdded[0]);
        EXPECT_EQ(0u, resourceChanges.m_resourcesRemoved.size());

        EXPECT_EQ(3u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(renderTarget, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderTarget, resourceChanges.m_sceneResourceActions[0].action);
        EXPECT_EQ(renderBuffer, resourceChanges.m_sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_CreateRenderBuffer, resourceChanges.m_sceneResourceActions[1].action);
        EXPECT_EQ(blitpassHandle, resourceChanges.m_sceneResourceActions[2].handle);
        EXPECT_EQ(ESceneResourceAction_CreateBlitPass, resourceChanges.m_sceneResourceActions[2].action);
    });
    this->expectResourceQueries({ this->m_arrayResource });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendsResourcesTogetherWithFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateRenderable(this->m_scene.allocateNode(0, {}), {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    ResourceBlob blob(1024 * EnumToSize(EDataType::Float));
    std::generate(blob.data(), blob.data() + blob.size(), [](){ static uint8_t i{9}; return std::byte(++i); });
    const ResourceContentHash hash = this->m_arrayResource->getHash();

    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, hash, {} }, {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            ASSERT_EQ(1u, update.resources.size());
            EXPECT_EQ(hash, update.resources[0]->getHash());
        });

    this->expectResourceQueries({ this->m_arrayResource });
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, flushClearsListsOfChangedResources)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateRenderable(this->m_scene.allocateNode(0, {}), {});
    this->m_scene.allocateRenderTarget({});
    this->m_scene.allocateRenderBuffer({ 1u, 1u, EPixelStorageFormat::R16F, ERenderBufferAccessMode::ReadWrite, 0u }, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_arrayResource->getHash(), {} }, {});
    this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u), {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        auto& resourceChanges = update.flushInfos.resourceChanges;
        EXPECT_EQ(1u, resourceChanges.m_resourcesAdded.size());
        EXPECT_EQ(0u, resourceChanges.m_resourcesRemoved.size());
        EXPECT_EQ(3u, resourceChanges.m_sceneResourceActions.size());
    });
    this->expectResourceQueries({ this->m_arrayResource });
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.allocateNode(0, {});
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        auto& resourceChanges = update.flushInfos.resourceChanges;
        EXPECT_EQ(0u, resourceChanges.m_resourcesAdded.size());
        EXPECT_EQ(0u, resourceChanges.m_resourcesRemoved.size());
        EXPECT_EQ(0u, resourceChanges.m_sceneResourceActions.size());
    });
    this->expectResourceQueries({}, { this->m_arrayResource }, false, false, false);
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sendListOfObsoleteResourcesTogetherWithFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateRenderable(this->m_scene.allocateNode(0, {}), {});
    const ResourceContentHash hash = this->m_arrayResource->getHash();
    this->m_scene.allocateDataLayout({}, ResourceContentHash(45u, 0), {});
    const RenderTargetHandle renderTarget = this->m_scene.allocateRenderTarget({});
    const RenderBufferHandle renderBuffer = this->m_scene.allocateRenderBuffer({ 1u, 1u, EPixelStorageFormat::R16F, ERenderBufferAccessMode::ReadWrite, 0u }, {});
    const DataSlotHandle dataSlot = this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, hash, {} }, {});
    const BlitPassHandle blitpassHandle = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u), {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->expectResourceQueries({ this->m_arrayResource });
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.releaseRenderTarget(renderTarget);
    this->m_scene.releaseRenderBuffer(renderBuffer);
    this->m_scene.releaseDataSlot(dataSlot);
    this->m_scene.releaseBlitPass(blitpassHandle);

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(4u, update.actions.numberOfActions());

        auto& resourceChanges = update.flushInfos.resourceChanges;
        EXPECT_EQ(0u, resourceChanges.m_resourcesAdded.size());
        EXPECT_EQ(1u, resourceChanges.m_resourcesRemoved.size());
        EXPECT_EQ(hash,  resourceChanges.m_resourcesRemoved[0]);

        EXPECT_EQ(3u, resourceChanges.m_sceneResourceActions.size());
        EXPECT_EQ(renderTarget, resourceChanges.m_sceneResourceActions[0].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderTarget, resourceChanges.m_sceneResourceActions[0].action);
        EXPECT_EQ(renderBuffer, resourceChanges.m_sceneResourceActions[1].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyRenderBuffer, resourceChanges.m_sceneResourceActions[1].action);
        EXPECT_EQ(blitpassHandle, resourceChanges.m_sceneResourceActions[2].handle);
        EXPECT_EQ(ESceneResourceAction_DestroyBlitPass, resourceChanges.m_sceneResourceActions[2].action);
    });
    EXPECT_CALL(this->m_resourceComponent, resolveResources(_)).Times(AnyNumber()).WillRepeatedly(Return(ManagedResourceVector{}));
    EXPECT_CALL(this->m_resourceComponent, getResourceInfo(_)).Times(AnyNumber()).WillRepeatedly(ReturnRef(this->m_resInfo[0]));
    this->m_sceneLogic.flushSceneActions({}, {});
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererAfterFlushIfThereArePendingActions)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle node = this->m_scene.allocateNode(0u, NodeHandle(1u));

    // expect flush to original renderer first
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->m_scene.allocateRenderable(node, RenderableHandle(2));

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, this->m_sceneId, _));
    // expect newly flushed actions to new renderer
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(1u, update.actions.numberOfActions());
        EXPECT_EQ(ESceneActionId::AllocateNode, update.actions[0].type());
    });
    this->m_sceneLogic.addSubscriber(newRendererID);

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    // expect newly flushed actions to original and new renderer
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(UnorderedElementsAre(this->m_rendererID, newRendererID), _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(1u, update.actions.numberOfActions());
        EXPECT_EQ(ESceneActionId::AllocateRenderable, update.actions[0].type());
    });
    this->m_sceneLogic.flushSceneActions({}, {});

    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererWithNewResources)
{
    // add some active subscriber so actions are queued
    this->publish();
    auto  rendHandle = this->m_scene.allocateRenderable(this->m_scene.allocateNode(0, {}), {});
    const ResourceContentHash hash = this->m_textureResource->getHash();
    auto layoutHandle = this->m_scene.allocateDataLayout({}, hash, {});
    auto instanceHandle = this->m_scene.allocateDataInstance(layoutHandle, {});
    this->m_scene.setRenderableDataInstance(rendHandle, ERenderableDataSlotType_Geometry, instanceHandle);
    this->expectResourceQueries({ this->m_textureResource });
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.allocateNode(0, {}); // action not flushed

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(4u, update.actions.numberOfActions());

        auto& resourceChanges = update.flushInfos.resourceChanges;
        ASSERT_EQ(1u, resourceChanges.m_resourcesAdded.size());
        EXPECT_EQ(hash, resourceChanges.m_resourcesAdded[0]);
        EXPECT_TRUE(resourceChanges.m_resourcesRemoved.empty());
        EXPECT_TRUE(resourceChanges.m_sceneResourceActions.empty());
    });
    this->expectResourceQueries({}, { this->m_textureResource });
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererWithValidTimeInfoAndVersion)
{
    // add some active subscriber so actions are queued
    this->publish();

    const FlushTimeInformation ftiIn{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)), FlushTime::Clock::getClockType(), false };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiIn, versionTagIn);

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(0u, update.actions.numberOfActions());

        EXPECT_EQ(ftiIn, update.flushInfos.flushTimeInfo);
        EXPECT_EQ(versionTagIn, update.flushInfos.versionTag);
    });
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sendsSceneToNewlySubscribedRendererWithLastAppliedTimeInfoAndVersion)
{
    // add some active subscriber so actions are queued
    this->publish();

    const FlushTimeInformation ftiIn{ FlushTime::Clock::time_point(std::chrono::milliseconds(2)), FlushTime::Clock::time_point(std::chrono::milliseconds(3)), FlushTime::Clock::getClockType(), false };
    const SceneVersionTag versionTagIn{ 333 };
    this->m_sceneLogic.flushSceneActions(ftiIn, versionTagIn);

    const FlushTimeInformation ftiIn2{ FlushTime::Clock::time_point(std::chrono::milliseconds(22)), FlushTime::Clock::time_point(std::chrono::milliseconds(33)), FlushTime::Clock::getClockType(), false };
    const SceneVersionTag versionTagIn2{ 123 };
    this->m_sceneLogic.flushSceneActions(ftiIn2, versionTagIn2);

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(0u, update.actions.numberOfActions());

        EXPECT_EQ(ftiIn2, update.flushInfos.flushTimeInfo);
        EXPECT_EQ(versionTagIn2, update.flushInfos.versionTag);
    });
    this->m_sceneLogic.addSubscriber(newRendererID);
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, sendsSceneToNewlySubscribedRendererWithNewResources)
{
    // add some active subscriber so actions are queued
    this->publish();
    auto rendHandle = this->m_scene.allocateRenderable(this->m_scene.allocateNode(0, {}), {});
    const ResourceContentHash hash = this->m_textureResource->getHash();
    auto layoutHandle = this->m_scene.allocateDataLayout({}, hash, {});
    auto instanceHandle = this->m_scene.allocateDataInstance(layoutHandle, {});
    expectResourceQueries({ this->m_textureResource });
    this->m_scene.setRenderableDataInstance(rendHandle, ERenderableDataSlotType_Geometry, instanceHandle);
    this->m_sceneLogic.flushSceneActions({}, {});

    this->m_scene.allocateNode(0, {}); // action not flushed

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    this->m_sceneLogic.addSubscriber(newRendererID);

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        ASSERT_EQ(5u, update.actions.numberOfActions());

        auto& resourceChanges = update.flushInfos.resourceChanges;
        ASSERT_EQ(1u, resourceChanges.m_resourcesAdded.size());
        EXPECT_EQ(hash, resourceChanges.m_resourcesAdded[0]);
        EXPECT_TRUE(resourceChanges.m_resourcesRemoved.empty());
        EXPECT_TRUE(resourceChanges.m_sceneResourceActions.empty());
    });
    expectResourceQueries({}, { this->m_textureResource }, false, true);
    this->flush();
    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, doesNotSendSceneUpdatesToNewSubscriberThatUnsubscribedBeforeFlush)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, this->m_sceneId, _));
    this->expectFlushSceneActionList();
    this->m_sceneLogic.addSubscriber(newRendererID);

    this->m_scene.allocateNode(0, {});

    this->m_sceneLogic.removeSubscriber(newRendererID);

    // expect flush to original renderer first
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, doesNotSendAnythingToNewSubscriberThatUnsubscribedBeforeFlush)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    const SceneInfo sceneInfo(this->m_sceneId, this->m_scene.getName());

    this->m_sceneLogic.addSubscriber(newRendererID);
    this->m_scene.allocateNode(0, {});
    this->m_sceneLogic.removeSubscriber(newRendererID);

    // expect flush to original renderer first
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, sceneReferenceActionsAreNotSentToNewlySubscribedRenderer)
{
    this->m_scene.linkData(SceneReferenceHandle{ 1 }, DataSlotId{ 2 }, SceneReferenceHandle{ 3 }, DataSlotId{ 4 });

    this->publish();
    const Guid newRendererID("12345678-1234-5678-0000-123456789012");
    this->m_sceneLogic.addSubscriber(newRendererID);

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendCreateScene(newRendererID, _, _));
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ newRendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            auto& sceneReferenceActions = update.flushInfos.sceneReferences;
            EXPECT_TRUE(sceneReferenceActions.empty());
    });
    this->flush();
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, flushSendsOnlySceneReferenceActionsSinceLastFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.linkData(SceneReferenceHandle{ 1 }, DataSlotId{ 2 }, SceneReferenceHandle{ 3 }, DataSlotId{ 4 });
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            auto& sceneReferenceActions = update.flushInfos.sceneReferences;
            ASSERT_EQ(1u, sceneReferenceActions.size());
            EXPECT_EQ(SceneReferenceActionType::LinkData, sceneReferenceActions[0].type);
            EXPECT_EQ(SceneReferenceHandle{ 1 }, sceneReferenceActions[0].providerScene);
            EXPECT_EQ(DataSlotId{ 2 }, sceneReferenceActions[0].providerId);
            EXPECT_EQ(SceneReferenceHandle{ 3 }, sceneReferenceActions[0].consumerScene);
            EXPECT_EQ(DataSlotId{ 4 }, sceneReferenceActions[0].consumerId);
    });
    this->flush();

    this->m_scene.unlinkData(SceneReferenceHandle{ 1 }, DataSlotId{ 2 });
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            auto& sceneReferenceActions = update.flushInfos.sceneReferences;
            ASSERT_EQ(1u, sceneReferenceActions.size());
            EXPECT_EQ(SceneReferenceActionType::UnlinkData, sceneReferenceActions[0].type);
            EXPECT_EQ(SceneReferenceHandle{ 1 }, sceneReferenceActions[0].consumerScene);
            EXPECT_EQ(DataSlotId{ 2 }, sceneReferenceActions[0].consumerId);
    });
    this->flush();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        EXPECT_TRUE(update.flushInfos.resourceChanges.empty());
    });
    this->flush({}, SceneVersionTag{1});  // force flush by version
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, unpublishesSceneAndDoesNotFlushBeforeRemovingScene)
{
    // add some active subscriber so actions are queued
    this->publishAndAddSubscriberWithoutPendingActions();

    this->m_scene.allocateNode(0, {});

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
    SceneUpdate update;
    update.actions = (this->m_scene.getSceneActionCollection().copy());
    SceneActionCollectionCreator creator(update.actions);
    update.flushInfos.hasSizeInfo = true;
    update.flushInfos.flushCounter = 1;
    update.flushInfos.sizeInfo = this->m_scene.getSceneSizeInformation();

    this->expectSceneSend();
    this->expectSendOnActionList(update);
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
    SceneUpdate update;
    update.actions = this->m_scene.getSceneActionCollection().copy();
    SceneActionCollectionCreator creator(update.actions);
    update.flushInfos.hasSizeInfo = true;
    update.flushInfos.flushCounter = 1;
    update.flushInfos.sizeInfo = this->m_scene.getSceneSizeInformation();

    this->flush();

    // so far no expectations except scene publishing
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->expectSceneSend();
    this->expectSendOnActionList(update);

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
    SceneUpdate expectedUpdate;
    expectedUpdate.actions = (this->m_scene.getSceneActionCollection().copy());
    SceneActionCollectionCreator creator(expectedUpdate.actions);
    expectedUpdate.flushInfos.hasSizeInfo = true;
    expectedUpdate.flushInfos.flushCounter = 1;
    expectedUpdate.flushInfos.sizeInfo = this->m_scene.getSceneSizeInformation();

    this->flush();

    // so far no expectations except scene publishing
    Mock::VerifyAndClearExpectations(&this->m_sceneGraphProviderComponent);

    this->addSubscriber();
    this->expectSceneSend();
    this->expectSendOnActionList(expectedUpdate);

    this->flush();

    this->unpublish();
}

TEST_F(AClientSceneLogic_ShadowCopy, sceneActionsAreNotModifiedWhenLastRendererUnsubscribed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle node = this->m_scene.allocateNode(0, {});
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
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, _, _, _));
    this->addSubscriber();
    EXPECT_EQ(0u, this->m_scene.getSceneActionCollection().numberOfActions());

    this->expectSceneUnpublish();
}

TEST_F(AClientSceneLogic_Direct, sceneActionsAreNotModifiedWhenLastRendererUnsubscribed)
{
    this->publishAndAddSubscriberWithoutPendingActions();

    const NodeHandle node = this->m_scene.allocateNode(0, {});
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

    this->m_scene.allocateNode(0, {});

    uint32_t initialValue = this->m_scene.getStatisticCollection().statSceneActionsSent.getCounterValue();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        EXPECT_EQ(initialValue + update.actions.numberOfActions(), this->m_scene.getStatisticCollection().statSceneActionsSent.getCounterValue());
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

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
    {
        EXPECT_EQ(update.actions.numberOfActions(), this->m_scene.getStatisticCollection().statSceneActionsSent.getCounterValue());
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

TYPED_TEST(AClientSceneLogic_All, resolvesClientResourcesWhenFlushing)
{
    this->publish();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);

    this->flush();

    Mock::VerifyAndClearExpectations(&this->m_resourceComponent); // strict behavior for this test

    this->m_scene.allocateNode(0, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_arrayResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_arrayResource });
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            auto& resourceChanges = update.flushInfos.resourceChanges;
            EXPECT_EQ(1u, resourceChanges.m_resourcesAdded.size());
        });
    this->flush();

    this->m_scene.allocateNode(0, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(1u), {}, {}, this->m_textureResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_textureResource }, { this->m_arrayResource, this->m_textureResource });
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            auto& resourceChanges = update.flushInfos.resourceChanges;
            EXPECT_EQ(1u, resourceChanges.m_resourcesAdded.size());
        });
    this->flush();

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, resolvesClientResourcesWhenFlushingUnpublishedScene)
{
    Mock::VerifyAndClearExpectations(&this->m_resourceComponent); // strict behavior for this test

    this->m_scene.allocateNode(0, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_textureResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_textureResource });
    this->flush();
    this->m_scene.allocateNode(0, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(1u), {}, {}, this->m_arrayResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_arrayResource }, { this->m_arrayResource, this->m_textureResource });
    this->flush();
    this->publish();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            auto& resourceChanges = update.flushInfos.resourceChanges;
            EXPECT_EQ(2u, resourceChanges.m_resourcesAdded.size());
        });

    this->expectSceneSend();
    this->expectResourceQueries({}, { this->m_arrayResource, this->m_textureResource }, false, false, true); // shadow copy resolved on addSubscriber
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->expectResourceQueries({}, { this->m_arrayResource, this->m_textureResource }, false, true, false); // direct resolve on next flush
    this->flush();
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotTryToResolveResourcesWhenNoSceneChanges)
{
    Mock::VerifyAndClearExpectations(&this->m_resourceComponent); // strict behavior for this test
    this->publish();
    this->expectResourceQueries({},{}, false, false, false);
    this->flush({}, SceneVersionTag{ 823746 });

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));

    this->expectSceneSend();
    this->expectResourceQueries({},{}, false, false, false);
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotTryToResolveResourcesWhenNoResourceChanges)
{
    Mock::VerifyAndClearExpectations(&this->m_resourceComponent); // strict behavior for this test
    this->publish();
    this->m_scene.allocateNode(0, {});
    this->expectResourceQueries({},{}, false, false, false);
    this->flush({}, SceneVersionTag{ 823746 });

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));

    this->expectSceneSend();
    this->expectResourceQueries({},{}, false, false, false);
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, resolvesOldResourcesForNewSubscriber)
{
    this->publish();
    this->m_scene.allocateNode(0, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_textureResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_textureResource }, {}, false, false, true);
    this->flush();
    this->m_scene.allocateNode(0, {});
    this->expectResourceQueries({}, { this->m_textureResource }, false, false, false);
    this->flush();

    Mock::VerifyAndClearExpectations(&this->m_resourceComponent); // strict behavior for this test
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));

    this->expectSceneSend();
    this->expectResourceQueries({}, { this->m_textureResource }, false, false, true);
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->expectResourceQueries({}, { this->m_textureResource }, false, true, false);
    this->flush();
    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, succeedsASecondFlushContainingAllSceneActionsAfterAddingResourcesMissingFromFailedFirstFlush)
{
    this->publish();

    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _));
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_arrayResource->getHash(), {} }, {});
    auto rendHandle = this->m_scene.allocateRenderable(this->m_scene.allocateNode(0, {}), {});
    auto layoutHandle = this->m_scene.allocateDataLayout({}, this->m_textureResource->getHash(), {});
    auto instanceHandle = this->m_scene.allocateDataInstance(layoutHandle, {});
    this->m_scene.setRenderableDataInstance(rendHandle, ERenderableDataSlotType_Geometry, instanceHandle);
    const RenderTargetHandle renderTarget = this->m_scene.allocateRenderTarget({});
    EXPECT_FALSE(this->m_sceneLogic.flushSceneActions({}, {}));

    Mock::VerifyAndClearExpectations(&this->m_resourceComponent); // strict behavior for this test
    this->m_scene.allocateNode(0, {});
    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(1u), {}, {}, this->m_effectResource->getHash(), {} }, {});
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, _)).WillOnce([&](const auto& /*unused*/, const auto& update, auto /*unused*/, auto /*unused*/, auto& /*unused*/)
        {
            auto& resourceChanges = update.flushInfos.resourceChanges;
            EXPECT_EQ(3u, resourceChanges.m_resourcesAdded.size());

            ASSERT_EQ(9u, update.actions.numberOfActions());

            ASSERT_EQ(1u, resourceChanges.m_sceneResourceActions.size());
            EXPECT_EQ(renderTarget, resourceChanges.m_sceneResourceActions[0].handle);
            EXPECT_EQ(ESceneResourceAction_CreateRenderTarget, resourceChanges.m_sceneResourceActions[0].action);
        });
    this->expectResourceQueries({ this->m_textureResource, this->m_arrayResource, this->m_effectResource });
    EXPECT_TRUE(this->m_sceneLogic.flushSceneActions({}, {}));

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, fillsResourceStatisticsInitiallyWithZeros)
{
    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });
}

TYPED_TEST(AClientSceneLogic_All, fillsResourceStatisticsWithZerosIfNoResources)
{
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, _, _, _)).Times(AnyNumber());
    this->publish();
    this->m_scene.allocateNode(0, {});
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    this->expectSceneUnpublish();
}


TYPED_TEST(AClientSceneLogic_All, fillsResourceStatisticsWithZerosIfNoResourcesDespiteResourceChanges)
{
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, _, _, _)).Times(AnyNumber());
    this->publish();
    this->m_scene.allocateNode(0, {});
    auto dataSlotHandle = this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, { 0, 1 }, {} }, {});
    this->m_scene.releaseDataSlot(dataSlotHandle);
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, updatesResourceStatisticsIfArrayResourceAddedAndRemovedFromScene)
{
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, _, _, _)).Times(AnyNumber());
    this->publish();
    this->m_scene.allocateNode(0, {});
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    const DataSlotHandle dataSlot = this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_arrayResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_arrayResource });
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 1, 1, 1 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    auto newArray = new ArrayResource(EResourceType::IndexArray, 1, EDataType::UInt16, nullptr, {});
    newArray->setResourceData(ResourceBlob{ 2 }, { 4u, 4u });
    ManagedResource newManRes(newArray);

    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(1u), {}, {}, newArray->getHash(), {} }, {});
    this->expectResourceQueries({ newManRes }, { this->m_arrayResource, newManRes });
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 2, 1, 2 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    this->m_scene.releaseDataSlot(dataSlot);
    this->expectResourceQueries({}, { newManRes }, true);
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 1, 2, 2 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, updatesResourceStatisticsIfEffectAddedAndRemovedFromScene)
{
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, _, _, _)).Times(AnyNumber());
    this->publish();
    this->m_scene.allocateNode(0, {});
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    const DataSlotHandle dataSlot = this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_effectResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_effectResource });
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 1, 6, 6 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    ManagedResource newManRes(new EffectResource("f00", "bar", {}, {}, {}, {}, {}));

    this->expectResourceQueries({ newManRes }, { this->m_effectResource, newManRes });
    this->m_scene.allocateDataSlot({EDataSlotType::TextureProvider, DataSlotId(1u), {}, {}, newManRes->getHash(), {}}, {});
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 2, 7, 9 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    this->m_scene.releaseDataSlot(dataSlot);
    this->expectResourceQueries({}, { newManRes }, true);
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 1, 9, 9 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 0, 0, 0 });

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, updatesResourceStatisticsIfTextureAddedAndRemovedFromScene)
{
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, _, _, _)).Times(AnyNumber());
    this->publish();
    this->m_scene.allocateNode(0, {});
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    const DataSlotHandle dataSlot = this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_textureResource->getHash(), {} }, {});
    this->expectResourceQueries({ this->m_textureResource });
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 1, 1, 1 });

    auto newTex = new TextureResource(EResourceType::Texture2D, TextureMetaInfo(1u, 1u, 1u, EPixelStorageFormat::R8, false, {}, { 1u }), {});
    newTex->setResourceData(ResourceBlob{ 2 }, { 4u, 4u });
    ManagedResource newManRes(newTex);

    this->m_scene.allocateDataSlot({ EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, newManRes->getHash(), {} }, {});
    this->expectResourceQueries({ newManRes }, { this->m_textureResource, newManRes });
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 2, 1, 2 });

    this->expectResourceQueries({}, { newManRes }, true);
    this->m_scene.releaseDataSlot(dataSlot);
    this->flush();

    this->expectStatistics(EResourceStatisticIndex_ArrayResource, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Effect, { 0, 0, 0 });
    this->expectStatistics(EResourceStatisticIndex_Texture, { 1, 2, 2 });

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, clientSceneStatisticsIsPassedToSceneGraphComponentOnFlush)
{
    this->publishAndAddSubscriberWithoutPendingActions();
    this->m_scene.allocateNode(0, {});

    EXPECT_CALL(this->m_sceneGraphProviderComponent,
                sendSceneUpdate_rvr(std::vector<Guid>{ this->m_rendererID }, _, this->m_sceneId, _, Ref(this->m_scene.getStatisticCollection())));
    this->m_sceneLogic.flushSceneActions({}, {});

    this->expectSceneUnpublish();
}

TYPED_TEST(AClientSceneLogic_All, doesNotCallGetContentInfoWhenComponentDoesNotKnowResource)
{
    EXPECT_CALL(this->m_sceneGraphProviderComponent, sendSceneUpdate_rvr(_, _, _, _, _)).Times(AnyNumber());
    this->publish();
    this->m_scene.allocateNode(0, {});
    this->expectSceneSend();
    this->m_sceneLogic.addSubscriber(this->m_rendererID);
    this->flush();

    this->m_scene.allocateDataSlot({EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_textureResource->getHash(), {}}, {});
    const DataSlotHandle dataSlot = this->m_scene.allocateDataSlot({EDataSlotType::TextureProvider, DataSlotId(0u), {}, {}, this->m_effectResource->getHash(), {}}, {});
    this->expectResourceQueries({ this->m_textureResource,  this->m_effectResource });
    this->flush();

    this->m_scene.releaseDataSlot(dataSlot);
    Mock::VerifyAndClearExpectations(&this->m_resourceComponent);
    EXPECT_CALL(this->m_resourceComponent, knowsResource(this->m_textureResource->getHash())).WillOnce(Return(false));
    EXPECT_CALL(this->m_resourceComponent, resolveResources(_)).Times(AtLeast(0)).WillRepeatedly(Return(ManagedResourceVector{ this->m_textureResource }));

    this->flush();
    this->expectSceneUnpublish();
}
}
