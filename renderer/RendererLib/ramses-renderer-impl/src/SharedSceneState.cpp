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
    void SharedSceneState::setActualState(RendererSceneState state)
    {
        m_actualState = state;

        updateLastHighestStateAndOwner();
    }

    RendererSceneState SharedSceneState::getActualState() const
    {
        return m_actualState;
    }

    void SharedSceneState::setDesiredState(ContentID contentID, RendererSceneState state)
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

    RendererSceneState SharedSceneState::getCurrentStateForContent(ContentID contentID) const
    {
        const auto desiredStateIt = m_desiredStates.find(contentID);
        if (desiredStateIt == m_desiredStates.cend())
            return RendererSceneState::Unavailable;

        // content is owner of last highest state, report actual scene state
        if (contentID == m_lastHighestStateOwner)
            return m_actualState;

        // content is not owner of last highest state
        // report actual state or desired state, whichever is lower
        return std::min(m_actualState, desiredStateIt->second);
    }

    void SharedSceneState::updateLastHighestStateAndOwner()
    {
        // Desired behavior is that the last state owner is defined as the one who triggered the actual state to be pushed highest.
        // The last state owner keeps being owner until someone else desires at least the level of actual state
        // (or the actual state drops to a desired state of another content).

        // find last highest state and its owner
        RendererSceneState maxCurrentlyDesiredState = RendererSceneState::Unavailable;
        ContentID maxCurrentlyDesiredStateOwner = ContentID::Invalid();
        for (const auto& it : m_desiredStates)
        {
            if (it.second >= maxCurrentlyDesiredState)
            {
                maxCurrentlyDesiredStateOwner = it.first;
                maxCurrentlyDesiredState = it.second;
            }
        }

        // update last highest state and owner only if desiring equal or higher than actual state
        if (maxCurrentlyDesiredState >= m_actualState)
        {
            m_lastHighestStateOwner = maxCurrentlyDesiredStateOwner;
            m_lastHighestDesiredState = maxCurrentlyDesiredState;
        }
    }
}
