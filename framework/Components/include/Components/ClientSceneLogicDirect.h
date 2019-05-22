//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTSCENELOGICDIRECT_H
#define RAMSES_CLIENTSCENELOGICDIRECT_H

#include "Components/ClientSceneLogicBase.h"

namespace ramses_internal
{
    class ClientSceneLogicDirect final : public ClientSceneLogicBase
    {
    public:
        ClientSceneLogicDirect(ISceneGraphSender& sceneGraphSender, ClientScene& scene, const Guid& clientAddress);

        virtual void flushSceneActions(ESceneFlushMode flushMode, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) override;

    private:
        SceneSizeInformation   m_previousSceneSizes;
    };
}

#endif
