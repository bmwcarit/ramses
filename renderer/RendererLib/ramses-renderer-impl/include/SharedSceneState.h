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
#include <unordered_map>
#include <algorithm>

namespace ramses
{
    class SharedSceneState
    {
    public:
        void setActualState(RendererSceneState state);
        RendererSceneState getActualState() const;

        void setDesiredState(ContentID contentID, RendererSceneState state);
        RendererSceneState getConsolidatedDesiredState() const;

        RendererSceneState getCurrentStateForContent(ContentID contentID) const;

    private:
        void updateLastHighestStateAndOwner();

        std::unordered_map<ContentID, RendererSceneState> m_desiredStates;
        RendererSceneState m_actualState = RendererSceneState::Unavailable;

        ContentID m_lastHighestStateOwner = ContentID::Invalid();
        RendererSceneState m_lastHighestDesiredState = RendererSceneState::Unavailable;
    };
}

#endif
