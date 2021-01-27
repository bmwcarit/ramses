//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHAREDSCENESTATE_H
#define RAMSES_SHAREDSCENESTATE_H

#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/RendererSceneState.h"
#include <map>
#include <algorithm>

namespace ramses
{
    // Custom content identifier which allows the SharedSceneState to
    // separately track desired states of
    //     a) the content itself { contentId, false }
    //     b) this contents hide animation placeholder { contentId, true }
    // This allows the hide animation placeholder to still keep the scene
    // rendering the hide animation while user is ramping the scene state
    // of that content up or down independently from that animation.
    struct ContentIdentifier
    {
        ContentID contentId;
        bool      isHideAnimation;
    };

    bool operator<(const ContentIdentifier& lhs, const ContentIdentifier& rhs);

    class SharedSceneState
    {
    public:
        void setReportedState(RendererSceneState state);
        RendererSceneState getReportedState() const;

        void setRequestedState(RendererSceneState state);
        RendererSceneState getRequestedState() const;

        void setDesiredState(ContentIdentifier contentID, RendererSceneState state);
        RendererSceneState getConsolidatedDesiredState() const;

        RendererSceneState getCurrentStateForContent(ContentIdentifier contentID) const;

    private:
        void updateLastHighestStateAndOwner();

        std::map<ContentIdentifier, RendererSceneState> m_desiredStates;
        RendererSceneState m_reportedState = RendererSceneState::Unavailable;
        RendererSceneState m_requestedState = RendererSceneState::Unavailable;

        ContentIdentifier m_lastHighestStateOwner = ContentIdentifier{ ContentID::Invalid(), false };
        RendererSceneState m_lastHighestDesiredState = RendererSceneState::Unavailable;
    };
}

#endif
