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
#include "DisplayManager/IDisplayManager.h"
#include <unordered_map>
#include <algorithm>

namespace ramses
{
    class SharedSceneState
    {
    public:
        void setActualState(ramses_internal::SceneState state);
        ramses_internal::SceneState getActualState() const;

        void setDesiredState(ContentID contentID, ramses_internal::SceneState state);
        ramses_internal::SceneState getConsolidatedDesiredState() const;

        ramses_internal::SceneState getCurrentStateForContent(ContentID contentID) const;

    private:
        void updateLastHighestStateAndOwner();

        ContentID InvalidContentID{ std::numeric_limits<ContentID::BaseType>::max() };

        std::unordered_map<ContentID, ramses_internal::SceneState> m_desiredStates;
        ramses_internal::SceneState m_actualState = ramses_internal::SceneState::Unavailable;

        ContentID m_lastHighestStateOwner{ InvalidContentID };
        ramses_internal::SceneState m_lastHighestDesiredState = ramses_internal::SceneState::Unavailable;
    };
}

#endif
