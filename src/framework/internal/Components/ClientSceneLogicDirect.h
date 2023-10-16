//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/ClientSceneLogicBase.h"
#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"

namespace ramses::internal
{
    class ClientSceneLogicDirect final : public ClientSceneLogicBase
    {
    public:
        ClientSceneLogicDirect(ISceneGraphSender& sceneGraphSender, ClientScene& scene, IResourceProviderComponent& res, const Guid& clientAddress);

        bool flushSceneActions(const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) override;

    private:
        SceneSizeInformation m_previousSceneSizes;
        FlushTime::Clock::time_point m_effectTimeSync{FlushTime::InvalidTimestamp};
    };
}
