//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/ClientSceneLogicBase.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Components/ManagedResource.h"

namespace ramses::internal
{
    class ClientSceneLogicShadowCopy final : public ClientSceneLogicBase
    {
    public:
        ClientSceneLogicShadowCopy(ISceneGraphSender& sceneGraphSender, ClientScene& scene, IResourceProviderComponent& res, const Guid& clientAddress, EFeatureLevel featureLevel);

        bool flushSceneActions(const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) override;

    private:
        void postAddSubscriber() override;
        void sendShadowCopySceneToWaitingSubscribers();

        SceneWithExplicitMemory m_sceneShadowCopy;
        FlushTimeInformation m_flushTimeInfoOfLastFlush;
        FlushTime::Clock::time_point m_effectTimeSync{FlushTime::InvalidTimestamp};
        SceneVersionTag m_lastVersionTag;
        ManagedResourceVector m_lastFlushUsedResources;
    };
}
