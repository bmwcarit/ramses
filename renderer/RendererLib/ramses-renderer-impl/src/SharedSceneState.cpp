//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SharedSceneState.h"

namespace ramses
{
    bool operator<(const ContentIdentifier& lhs, const ContentIdentifier& rhs)
    {
        return lhs.contentId == rhs.contentId ? !lhs.isHideAnimation && rhs.isHideAnimation : lhs.contentId.getValue() < rhs.contentId.getValue();
    }

    void SharedSceneState::setReportedState(RendererSceneState state)
    {
        m_reportedState = state;

        updateLastHighestStateAndOwner();
    }

    RendererSceneState SharedSceneState::getReportedState() const
    {
        return m_reportedState;
    }

    void SharedSceneState::setRequestedState(RendererSceneState state)
    {
        m_requestedState = state;
    }

    RendererSceneState SharedSceneState::getRequestedState() const
    {
        return m_requestedState;
    }

    void SharedSceneState::setDesiredState(ContentIdentifier contentID, RendererSceneState state)
    {
        m_desiredStates[contentID] = state;

        updateLastHighestStateAndOwner();
    }

    RendererSceneState SharedSceneState::getConsolidatedDesiredState() const
    {
        RendererSceneState state = RendererSceneState::Unavailable;
        for (const auto& desiredStateIt : m_desiredStates)
            state = std::max(state, desiredStateIt.second);

        return state;
    }

    RendererSceneState SharedSceneState::getCurrentStateForContent(ContentIdentifier contentID) const
    {
        const auto desiredStateIt = m_desiredStates.find(contentID);
        if (desiredStateIt == m_desiredStates.cend())
            return RendererSceneState::Unavailable;

        // content is owner of last highest state, report actual scene state
        if (contentID.contentId == m_lastHighestStateOwner.contentId && contentID.isHideAnimation == m_lastHighestStateOwner.isHideAnimation)
            return m_reportedState;

        // content is not owner of last highest state
        // report actual state or desired state, whichever is lower
        return std::min(m_reportedState, desiredStateIt->second);
    }

    void SharedSceneState::updateLastHighestStateAndOwner()
    {
        // Desired behavior is that the last state owner is defined as the one who triggered the actual state to be pushed highest.
        // The last state owner keeps being owner until someone else desires at least the level of actual state
        // (or the actual state drops to a desired state of another content).

        // find last highest state and its owner
        RendererSceneState maxCurrentlyDesiredState = RendererSceneState::Unavailable;
        ContentIdentifier maxCurrentlyDesiredStateOwner = ContentIdentifier{ ContentID::Invalid(), false };
        for (const auto& it : m_desiredStates)
        {
            if (it.second >= maxCurrentlyDesiredState)
            {
                maxCurrentlyDesiredStateOwner = it.first;
                maxCurrentlyDesiredState = it.second;
            }
        }

        // update last highest state and owner only if desiring equal or higher than actual state
        if (maxCurrentlyDesiredState >= m_reportedState)
        {
            m_lastHighestStateOwner = maxCurrentlyDesiredStateOwner;
            m_lastHighestDesiredState = maxCurrentlyDesiredState;
        }
    }
}
