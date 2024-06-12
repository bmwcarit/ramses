//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "CreationHelper.h"
#include "ramses/client/SceneReference.h"
#include "impl/SceneReferenceImpl.h"

using namespace testing;

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace ramses::internal
{
    class ASceneReference : public LocalTestClientWithScene, public ::testing::Test
    {
    };

    TEST_F(ASceneReference, createSceneReference)
    {
        ramses::SceneReference* sceneReference = this->m_scene.createSceneReference(sceneId_t(444),"testSceneReference");
        ASSERT_NE(nullptr, sceneReference);

        const auto sceneRefHandle = sceneReference->impl().getSceneReferenceHandle();
        EXPECT_TRUE(sceneRefHandle.isValid());
        ASSERT_TRUE(this->getInternalScene().isSceneReferenceAllocated(sceneRefHandle));
        const auto& sceneRef = this->getInternalScene().getSceneReference(sceneRefHandle);
        EXPECT_EQ(444u, sceneRef.sceneId.getValue());
        EXPECT_EQ(RendererSceneState::Available, sceneRef.requestedState);
        EXPECT_EQ(0, sceneRef.renderOrder);
        EXPECT_FALSE(sceneRef.flushNotifications);

        EXPECT_TRUE(this->m_scene.destroy(*sceneReference));
        EXPECT_FALSE(this->getInternalScene().isSceneReferenceAllocated(sceneRefHandle));
    }

    TEST_F(ASceneReference, failsToCreateSceneReferenceReferencingSelf)
    {
        const auto sceneReference = this->m_scene.createSceneReference(this->m_scene.getSceneId());
        EXPECT_EQ(nullptr, sceneReference);
    }

    TEST_F(ASceneReference, failsToCreateSceneReferenceUsingSceneIdAlreadyReferenced)
    {
        EXPECT_NE(nullptr, this->m_scene.createSceneReference(sceneId_t{ 444 }));

        EXPECT_EQ(nullptr, this->m_scene.createSceneReference(sceneId_t{ 444 }));
        EXPECT_EQ(nullptr, this->m_scene.createSceneReference(sceneId_t{ 444 }));

        auto otherScene = getClient().createScene(sceneId_t(555));
        EXPECT_EQ(nullptr, otherScene->createSceneReference(sceneId_t(444)));
    }

    TEST_F(ASceneReference, failsToCreateSceneReferenceUsingInvalidSceneId)
    {
        const auto sceneReference = this->m_scene.createSceneReference(sceneId_t::Invalid());
        EXPECT_EQ(nullptr, sceneReference);
    }

    TEST_F(ASceneReference, canGetReferencedSceneSceneId)
    {
        ramses::SceneReference* sceneReference = this->m_scene.createSceneReference(sceneId_t(444), "testSceneReference");
        ASSERT_EQ(sceneId_t(444), sceneReference->getReferencedSceneId());
    }

    TEST_F(ASceneReference, getsInitialRequestedSceneState)
    {
        ramses::SceneReference* sceneReference = this->m_scene.createSceneReference(sceneId_t(444), "testSceneReference");
        ASSERT_EQ(RendererSceneState::Available, sceneReference->getRequestedState());
    }

    TEST_F(ASceneReference, canGetRequestedSceneState)
    {
        ramses::SceneReference* sceneReference = this->m_scene.createSceneReference(sceneId_t(444), "testSceneReference");
        sceneReference->requestState(RendererSceneState::Ready);
        ASSERT_EQ(RendererSceneState::Ready, sceneReference->getRequestedState());

        const auto sceneRefHandle = sceneReference->impl().getSceneReferenceHandle();
        ASSERT_TRUE(this->getInternalScene().isSceneReferenceAllocated(sceneRefHandle));
        EXPECT_EQ(RendererSceneState::Ready, this->getInternalScene().getSceneReference(sceneRefHandle).requestedState);
    }

    TEST_F(ASceneReference, rejectsLinkDataIfSceneReferencesAreInWrongState)
    {
        auto reference1 = m_scene.createSceneReference(sceneId_t(111));
        auto reference2 = m_scene.createSceneReference(sceneId_t(222));

        EXPECT_FALSE(m_scene.linkData(reference1, dataProviderId_t(1), reference2, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference2, dataProviderId_t(1), reference1, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(nullptr, dataProviderId_t(1), reference1, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference1, dataProviderId_t(1), nullptr, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(nullptr, dataProviderId_t(1), reference2, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference2, dataProviderId_t(1), nullptr, dataConsumerId_t(2)));

        reference1->impl().setReportedState(RendererSceneState::Available);
        EXPECT_FALSE(m_scene.linkData(reference1, dataProviderId_t(1), reference2, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference2, dataProviderId_t(1), reference1, dataConsumerId_t(2)));

        reference1->impl().setReportedState(RendererSceneState::Ready);
        EXPECT_FALSE(m_scene.linkData(reference1, dataProviderId_t(1), reference2, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference2, dataProviderId_t(1), reference1, dataConsumerId_t(2)));

        reference1->impl().setReportedState(RendererSceneState::Rendered);
        EXPECT_FALSE(m_scene.linkData(reference1, dataProviderId_t(1), reference2, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference2, dataProviderId_t(1), reference1, dataConsumerId_t(2)));
    }

    TEST_F(ASceneReference, acceptsValidLinkDataParameters)
    {
        auto reference1 = m_scene.createSceneReference(sceneId_t(111));
        auto reference2 = m_scene.createSceneReference(sceneId_t(222));
        reference1->impl().setReportedState(RendererSceneState::Ready);
        reference2->impl().setReportedState(RendererSceneState::Rendered);

        EXPECT_TRUE(m_scene.linkData(reference1, dataProviderId_t(1), nullptr, dataConsumerId_t(2)));
        EXPECT_TRUE(m_scene.linkData(nullptr, dataProviderId_t(1), reference1, dataConsumerId_t(2)));
        EXPECT_TRUE(m_scene.linkData(reference1, dataProviderId_t(1), reference2, dataConsumerId_t(2)));
    }

    TEST_F(ASceneReference, rejectsInvalidLinkDataParameters)
    {
        auto otherScene = getClient().createScene(sceneId_t(555));
        auto referenceFromOtherScene1 = otherScene->createSceneReference(sceneId_t(111));
        auto referenceFromOtherScene2 = otherScene->createSceneReference(sceneId_t(222));

        auto reference1 = m_scene.createSceneReference(sceneId_t(333));

        reference1->impl().setReportedState(RendererSceneState::Ready);
        referenceFromOtherScene1->impl().setReportedState(RendererSceneState::Rendered);
        referenceFromOtherScene2->impl().setReportedState(RendererSceneState::Rendered);

        EXPECT_FALSE(m_scene.linkData(nullptr, dataProviderId_t(1), nullptr, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference1, dataProviderId_t(1), reference1, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(nullptr, dataProviderId_t(1), referenceFromOtherScene1, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(reference1, dataProviderId_t(1), referenceFromOtherScene1, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(referenceFromOtherScene1, dataProviderId_t(1), nullptr, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(referenceFromOtherScene1, dataProviderId_t(1), reference1, dataConsumerId_t(2)));
        EXPECT_FALSE(m_scene.linkData(referenceFromOtherScene1, dataProviderId_t(1), referenceFromOtherScene2, dataConsumerId_t(2)));
    }

    TEST_F(ASceneReference, rejectsInvalidUnlinkDataParameters)
    {
        auto otherScene = getClient().createScene(sceneId_t(555));
        auto referenceFromOtherScene = otherScene->createSceneReference(sceneId_t(111));
        EXPECT_FALSE(m_scene.unlinkData(referenceFromOtherScene, dataConsumerId_t(1)));
    }

    TEST_F(ASceneReference, canRequestAndUnrequestVersionNotifications)
    {
        auto reference = m_scene.createSceneReference(sceneId_t(111));
        EXPECT_TRUE(reference->requestNotificationsForSceneVersionTags(true));
        EXPECT_TRUE(reference->requestNotificationsForSceneVersionTags(false));
    }

    TEST_F(ASceneReference, requestsVersionNotification)
    {
        constexpr sceneId_t sceneId{ 444 };
        auto reference = m_scene.createSceneReference(sceneId);
        ASSERT_NE(nullptr, reference);
        const auto sceneRefHandle = reference->impl().getSceneReferenceHandle();
        ASSERT_TRUE(this->getInternalScene().isSceneReferenceAllocated(sceneRefHandle));

        EXPECT_TRUE(reference->requestNotificationsForSceneVersionTags(true));
        EXPECT_TRUE(this->getInternalScene().getSceneReference(sceneRefHandle).flushNotifications);

        EXPECT_TRUE(reference->requestNotificationsForSceneVersionTags(false));
        EXPECT_FALSE(this->getInternalScene().getSceneReference(sceneRefHandle).flushNotifications);
    }

    TEST_F(ASceneReference, setsRenderOrder)
    {
        constexpr sceneId_t sceneId{ 444 };
        auto reference = m_scene.createSceneReference(sceneId);
        ASSERT_NE(nullptr, reference);

        EXPECT_TRUE(reference->setRenderOrder(-13));

        const auto sceneRefHandle = reference->impl().getSceneReferenceHandle();
        ASSERT_TRUE(this->getInternalScene().isSceneReferenceAllocated(sceneRefHandle));
        EXPECT_EQ(-13, this->getInternalScene().getSceneReference(sceneRefHandle).renderOrder);
    }

    TEST_F(ASceneReference, createsActionForLinkData)
    {
        constexpr sceneId_t sceneId1{ 444 };
        constexpr sceneId_t sceneId2{ 124 };
        constexpr dataProviderId_t providerId{ 12 };
        constexpr dataConsumerId_t consumerId{ 13 };
        auto reference1 = m_scene.createSceneReference(sceneId1);
        auto reference2 = m_scene.createSceneReference(sceneId2);
        ASSERT_NE(nullptr, reference1);
        ASSERT_NE(nullptr, reference2);
        const auto sceneRefHandle1 = reference1->impl().getSceneReferenceHandle();
        const auto sceneRefHandle2 = reference2->impl().getSceneReferenceHandle();
        reference1->impl().setReportedState(RendererSceneState::Ready);
        reference2->impl().setReportedState(RendererSceneState::Ready);

        EXPECT_TRUE(m_scene.linkData(reference1, providerId, reference2, consumerId));
        EXPECT_TRUE(m_scene.linkData(reference1, providerId, nullptr, consumerId));
        EXPECT_TRUE(m_scene.linkData(nullptr, providerId, reference2, consumerId));

        const auto& actions = this->getInternalScene().getSceneReferenceActions();
        ASSERT_EQ(3u, actions.size());

        EXPECT_EQ(ramses::internal::SceneReferenceActionType::LinkData, actions[0].type);
        EXPECT_EQ(sceneRefHandle1, actions[0].providerScene);
        EXPECT_EQ(providerId.getValue(), actions[0].providerId.getValue());
        EXPECT_EQ(sceneRefHandle2, actions[0].consumerScene);
        EXPECT_EQ(consumerId.getValue(), actions[0].consumerId.getValue());

        EXPECT_EQ(ramses::internal::SceneReferenceActionType::LinkData, actions[1].type);
        EXPECT_EQ(sceneRefHandle1, actions[1].providerScene);
        EXPECT_EQ(providerId.getValue(), actions[1].providerId.getValue());
        EXPECT_FALSE(actions[1].consumerScene.isValid()); // master scene stored as invalid scene ref handle
        EXPECT_EQ(consumerId.getValue(), actions[1].consumerId.getValue());

        EXPECT_EQ(ramses::internal::SceneReferenceActionType::LinkData, actions[2].type);
        EXPECT_FALSE(actions[2].providerScene.isValid()); // master scene stored as invalid scene ref handle
        EXPECT_EQ(providerId.getValue(), actions[2].providerId.getValue());
        EXPECT_EQ(sceneRefHandle2, actions[2].consumerScene);
        EXPECT_EQ(consumerId.getValue(), actions[2].consumerId.getValue());
    }

    TEST_F(ASceneReference, createsActionForUnlinkData)
    {
        constexpr sceneId_t sceneId{ 444 };
        constexpr dataConsumerId_t consumerId{ 13 };
        auto reference = m_scene.createSceneReference(sceneId);
        ASSERT_NE(nullptr, reference);
        const auto sceneRefHandle = reference->impl().getSceneReferenceHandle();

        EXPECT_TRUE(m_scene.unlinkData(reference, consumerId));
        EXPECT_TRUE(m_scene.unlinkData(nullptr, consumerId));

        const auto& actions = this->getInternalScene().getSceneReferenceActions();
        ASSERT_EQ(2u, actions.size());

        EXPECT_EQ(ramses::internal::SceneReferenceActionType::UnlinkData, actions[0].type);
        EXPECT_EQ(sceneRefHandle, actions[0].consumerScene);
        EXPECT_EQ(consumerId.getValue(), actions[0].consumerId.getValue());

        EXPECT_EQ(ramses::internal::SceneReferenceActionType::UnlinkData, actions[1].type);
        EXPECT_FALSE(actions[1].consumerScene.isValid());
        EXPECT_EQ(consumerId.getValue(), actions[1].consumerId.getValue());
    }
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
