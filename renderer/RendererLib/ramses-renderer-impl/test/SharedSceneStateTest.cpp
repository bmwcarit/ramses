//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "SharedSceneState.h"

namespace ramses
{
    using namespace testing;

    class ASharedSceneState : public Test
    {
    protected:
        const ContentIdentifier m_contentID1{ ContentID{ 321 }, true };
        const ContentIdentifier m_contentID2{ ContentID{ 322 }, false };
        const ContentIdentifier m_contentID3{ ContentID{ 323 }, false };
        const ContentIdentifier m_contentID4{ ContentID{ 323 }, true };

        SharedSceneState m_sharedSceneState;
    };

    TEST_F(ASharedSceneState, reportsUnavailableStateInitially)
    {
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getReportedState());
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getConsolidatedDesiredState());
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
    }

    TEST_F(ASharedSceneState, reportsPreviouslySetActualStateRegardlessOfDesiredStates)
    {
        m_sharedSceneState.setReportedState(RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getReportedState());
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Unavailable);
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Available);
        m_sharedSceneState.setDesiredState(m_contentID3, RendererSceneState::Ready);
        m_sharedSceneState.setDesiredState(m_contentID4, RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getReportedState());

        m_sharedSceneState.setReportedState(RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getReportedState());
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Unavailable);
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Available);
        m_sharedSceneState.setDesiredState(m_contentID3, RendererSceneState::Ready);
        m_sharedSceneState.setDesiredState(m_contentID4, RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getReportedState());
    }

    TEST_F(ASharedSceneState, reportsConsolidatedDesiredStateAlwaysAsHighestOfDesiredStates)
    {
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Unavailable);
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getConsolidatedDesiredState());

        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Available);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getConsolidatedDesiredState());

        m_sharedSceneState.setDesiredState(m_contentID3, RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getConsolidatedDesiredState());

        m_sharedSceneState.setDesiredState(m_contentID4, RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getConsolidatedDesiredState());

        // not affected by actual state
        m_sharedSceneState.setReportedState(RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getConsolidatedDesiredState());
        m_sharedSceneState.setReportedState(RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getConsolidatedDesiredState());
    }

    TEST_F(ASharedSceneState, reportsCorrectSceneStateForGivenContent_noSharing)
    {
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        // from nothing to rendered

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Available);
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setReportedState(RendererSceneState::Available);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setReportedState(RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setReportedState(RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        // going back to nothing

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setReportedState(RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Available);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setReportedState(RendererSceneState::Available);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Unavailable);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        m_sharedSceneState.setReportedState(RendererSceneState::Unavailable);
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
    }

    TEST_F(ASharedSceneState, lastContentToHoldStateReportsActualStateWhenChangingDesiredStateDownwards)
    {
        // both contents get to rendered
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Rendered);
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Rendered);
        m_sharedSceneState.setReportedState(RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // content1 desires lower state
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Available);
        // its state is right away the desired lower one because other content2 owns the higher state now
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // now content2 also desires lower state
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Ready);
        // its state is NOT the desired lower one till the actual state reaches that desired state
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // even now when content1 desires higher state (same as content2) it will still not become owner of the last actual state (rendered)
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Ready);
        // but its state is right away its newly desired one
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // only now when actual state reaches ready, content2's state is reported as ready
        m_sharedSceneState.setReportedState(RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID2));
    }

    TEST_F(ASharedSceneState, lastContentToHoldStateReportsActualStateWhenChangingDesiredStateDownwards_swappedOrder)
    {
        // both contents get to rendered
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Rendered);
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Rendered);
        m_sharedSceneState.setReportedState(RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // content2 desires lower state
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Available);
        // its state is right away the desired lower one because other content1 owns the higher state now
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID2));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        // now content1 also desires lower state
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Ready);
        // its state is NOT the desired lower one till the actual state reaches that desired state
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID2));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        // even now when content2 desires higher state (same as content1) it will still not become owner of the last actual state (rendered)
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Ready);
        // but its state is right away its newly desired one
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID2));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));

        // only now when actual state reaches ready, content1's state is reported as ready
        m_sharedSceneState.setReportedState(RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID2));
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
    }

    TEST_F(ASharedSceneState, reportsCorrectSceneStateForGivenContent_sharedByTwoContents)
    {
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setReportedState(RendererSceneState::Available);

        // content1 requests rendered
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // content2 requests ready
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setReportedState(RendererSceneState::Ready);

        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setReportedState(RendererSceneState::Rendered);

        // content1 is rendered
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // content2 gets to rendered
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Rendered);
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // content1 gets to nothing
        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Available);
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setDesiredState(m_contentID1, RendererSceneState::Unavailable);
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        // content2 gets to nothing
        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Ready);
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Rendered, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setReportedState(RendererSceneState::Ready);

        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Available);
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Ready, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setReportedState(RendererSceneState::Available);

        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setDesiredState(m_contentID2, RendererSceneState::Unavailable);
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Available, m_sharedSceneState.getCurrentStateForContent(m_contentID2));

        m_sharedSceneState.setReportedState(RendererSceneState::Unavailable);

        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID1));
        EXPECT_EQ(RendererSceneState::Unavailable, m_sharedSceneState.getCurrentStateForContent(m_contentID2));
    }

    TEST_F(ASharedSceneState, reportsUnavailableRequestedStateInitially)
    {
        EXPECT_EQ(m_sharedSceneState.getRequestedState(), RendererSceneState::Unavailable);
    }

    TEST_F(ASharedSceneState, getsLastSetRequestedState)
    {
        m_sharedSceneState.setRequestedState(RendererSceneState::Available);
        EXPECT_EQ(m_sharedSceneState.getRequestedState(), RendererSceneState::Available);

        m_sharedSceneState.setRequestedState(RendererSceneState::Rendered);
        m_sharedSceneState.setRequestedState(RendererSceneState::Ready);
        EXPECT_EQ(m_sharedSceneState.getRequestedState(), RendererSceneState::Ready);

        m_sharedSceneState.setRequestedState(RendererSceneState::Available);
        m_sharedSceneState.setRequestedState(RendererSceneState::Rendered);
        EXPECT_EQ(m_sharedSceneState.getRequestedState(), RendererSceneState::Rendered);
    }
}
